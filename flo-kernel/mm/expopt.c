/*
 *	OS Homework 5
 *	System call implementation
 */
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


SYSCALL_DEFINE3(expose_page_table, pid_t, pid,
		unsigned long, fake_pgd,
		unsigned long, addr)
{
	if ((unsigned long *)addr == NULL || (unsigned long *)fake_pgd == NULL)
		return -EINVAL;
	if ((int)pid == -1)
		pid = current->pid;

	int iter, pgd_num;
	pgd_t *pgd_crnt;
	unsigned long L2T_base, *fake_pgd_k, *fake_pgd_k_iter;
	unsigned long pte_0_base, pte_1_base;
	struct task_struct *p = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct mm_struct *mm = p->mm;

	pgd_num = (PTRS_PER_PGD / 4) * 3;
	fake_pgd_k = kmalloc(sizeof(unsigned long) * pgd_num * 2, GFP_KERNEL);
	if (!fake_pgd_k)
		return -EFAULT;

	fake_pgd_k_iter = fake_pgd_k;
	pgd_crnt = mm->pgd;
	for (iter = 0; iter < USER_PTRS_PER_PGD; iter++) {
		L2T_base = ((unsigned long)((*pgd_crnt)[0])) & 0xfffff000;
		pte_0_base = (*pgd_crnt)[0];
		pte_1_base = (*pgd_crnt)[1];

		pr_debug("(%d)L1 Tbl Ptr: 0x%08lx ---> L2 Table base: 0x%08lx\n",
				iter, (unsigned long)pgd_crnt, L2T_base);
		pr_debug("\tL2 H/W Tbl Base Ptr[0] = 0x%08lx\n", pte_0_base);
		pr_debug("\tL2 H/W Tbl Base Ptr[1] = 0x%08lx\n", pte_1_base);

		pgd_crnt++;
		down_write(&(current->mm->mmap_sem));
		if (L2T_base != 0x00000000) {
			if (access_ok(VERIFY_WRITE, addr, PAGE_SIZE)) {
				remap_pfn_range(find_vma(current->mm, addr),
					addr,
					L2T_base >> PAGE_SHIFT,
					PAGE_SIZE,
					current->mm->mmap->vm_page_prot);
				*fake_pgd_k_iter = addr;
			} else
				return -EFAULT;
			pr_debug("Remap L2 Table: %d\n", iter);
		} else
			pr_debug("Skip  L1 Table: %d\n", iter);
		up_write(&(current->mm->mmap_sem));

		if (iter != pgd_num - 1)
			fake_pgd_k_iter++;
		addr += PAGE_SIZE;
	}

	if (copy_to_user((unsigned long *)fake_pgd,
				fake_pgd_k,
				sizeof(unsigned long) * 4096))
		return -EFAULT;
	kfree(fake_pgd_k);
	return 0;
}
