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

	struct task_struct *p = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct mm_struct *mm = p->mm;

	pgd_crnt = mm->pgd;
	for (iter = 0; iter < pgd_num; iter++) {
		printk("Index: %d(0x%lx)   --->\tpte[0]: 0x%lx(%d), pte[1]: 0x%lx(%d)\n",
				iter,
				pgd_crnt,
				(*pgd_crnt)[0],
				pte_pfn((*pgd_crnt)[0]),
				(*pgd_crnt)[1],
				pte_pfn((*pgd_crnt)[1]));
		pgd_crnt++;
		remap_pfn_range(find_vma(mm, addr),
                                        addr + iter*(2^10)*sizeof(unsigned long),
                                        (*pgd_crnt)[0]-PTRS_PER_PTE*sizeof(unsigned long),
                                        2*PTRS_PER_PTE*sizeof(unsigned long),
                                        PROT_READ);
	}
	printk("Number of pgd: %d\n", pgd_num);


/*
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

			//(2^10)*sizeof(unsigned long) = each PTE size
			//(2^9) = number of PTES in a page table	
			remap_pfn_range(find_vma(mm, addr),
					addr + index_pgd*((2^9)*(2^10)*sizeof(unsigned long)) + index_pte*(2^10)*sizeof(unsigned long),
					*pgd + index_pte*(2^10)*sizeof(unsigned long),
					(2^10)*sizeof(unsigned long),
					PROT_READ);

		}
		printk("end = 0x%lx\n", end);
		vma = vma->vm_next;
	} while (vma);
*/
	return 0;	
}
