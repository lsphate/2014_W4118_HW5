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

	int iter;
	int pgd_num = (PTRS_PER_PGD / 4) * 3;
	pgd_t *pgd_crnt;
	pte_t *pte_1, *pte_2;

	struct task_struct *p = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct mm_struct *mm = p->mm;

	pgd_crnt = mm->pgd;
	for (iter = 0; iter < pgd_num; iter++) {
		printk("Index: %d(0x%lx)  --->\tpte[0]: 0x%lx(%d), pte[1]: 0x%lx(%d)\n",
				iter,
				pgd_crnt,
				(*pgd_crnt)[0],
				pte_pfn((*pgd_crnt)[0]),
				(*pgd_crnt)[1],
				pte_pfn((*pgd_crnt)[1]));
		pgd_crnt++;

/*
 *  remap_pfn_range - remap kernel memory to userspace
 *  @vma: user vma to map to
 *  @addr: target user address to start at
 *  @pfn: physical address of kernel memory
 *  @size: size of map area
 *  @prot: page protection flags for this mapping
 *
 *  Note: this is only safe if the mm semaphore is held when called.
 */
		down_write(&(current->mm->mmap_sem));
		remap_pfn_range(find_vma(current->mm, addr),
				addr + iter * (2^10) * sizeof(unsigned long),
				(*pgd_crnt)[0] - PTRS_PER_PTE * sizeof(unsigned long),
				2 * PTRS_PER_PTE * sizeof(unsigned long),
				current->mm->mmap->vm_page_prot);
		up_write(&(current->mm->mmap_sem));
	}
	printk("Number of pgd: %d\n", pgd_num);

/*

	struct task_struct *p = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct mm_struct *mm = p->mm;
	struct vm_area_struct *vma = mm->mmap;
	unsigned long start, end, iter;
	pgd_t *pgd_curr = NULL;
	pte_t *pte_curr = NULL;
	int pgd_index;

	do {
		start = vma->vm_start;
		end = vma->vm_end;
		printk("start = 0x%lx\n", start);
		for (iter = start; iter < end; iter += 0x1000) {
			pgd_index = pgd_index(iter);
			pgd_curr = pgd_offset(mm, iter);
			pte_curr = pte_offset_map(pgd_curr, iter);
			printk("Index: %d(Ptr: 0x%p)(De-ref: 0x%lx) ---> pte: 0x%lx[0x%lx](%d)\n",
				pgd_index,
				pgd_curr,
				(*pgd_curr),
				pte_curr,
				(((unsigned long)(*pgd_curr) >> 10) << 10) + pte_index(iter),
				0);

		}
		printk("end = 0x%lx\n", end);
		vma = vma->vm_next;

	} while (vma);
*/
	return 0;	
}
