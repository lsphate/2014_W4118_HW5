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

SYSCALL_DEFINE3(expose_page_table, pid_t, pid,
		unsigned long, fake_pgd,
		unsigned long, addr)
{
	if (addr == NULL || fake_pgd == NULL)
		return -EINVAL;
	if ((int)pid == -1)
		pid = current->pid;

	struct task_struct *p = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct mm_struct *mm = p->mm;
	struct vm_area_struct *vma = mm->mmap;
	unsigned long start, end, it;
	pgd_t *pgd;
	pte_t *pte;
	int vma_ct = 0, pgd_ct = 0, pte_ct = 0, index_temp = 0;

	do {
		start = vma->vm_start;
		end = vma->vm_end;
		printk("start = 0x%lx\n", start);
		for (it = start; it < end; it += 0x1000) {
			
			int index_pgd = pgd_index(it);
			int index_pte = pte_index(it);
			printk("pgd_index(0x%lx) = %d\n", it, index_pgd);
			printk("pte_index = %d\n", index_pte);
			pgd = pgd_offset(mm, it);
			
			remap_pfn_range(find_vma(mm, addr),
					addr + index_pgd*((2^9)*(2^10)*sizeof(unsigned long)) + index_pte*(2^10)*sizeof(unsigned long),
					*pgd + index_pte*(2^10)*sizeof(unsigned long),
					(2^10)*sizeof(unsigned long),
					PROT_READ);
		}
		printk("end = 0x%lx\n", end);
		vma = vma->vm_next;
	} while (vma);



	

/*
	pgd_t *pgdp = current->mm->pgd;
	unsigned long address = TASK_SIZE;
	unsigned long tmp;
	unsigned long *pp;

	for (tmp = address; tmp < (PAGE_OFFSET - 1); tmp += 0x400000) {
		printk("pgd_index(0x%lx)=%d\n", tmp, pgd_index(tmp));
		pp = pgd_offset_k(tmp);
		printk("conten of pgd is 0x%lx\n", *pp);
	}
	printk("Call expose_page_table successfully!\n");
*/
	return 0;
}
