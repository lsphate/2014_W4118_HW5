/*
 *	OS Homework 5
 *	System call implementation
 */
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE3(expose_page_table, pid_t, pid,
		unsigned long, fake_pgd,
		unsigned long, addr)
{
	//if (addr == NULL || fake_pgd == NULL)
	//	return -EINVAL;
	printk("Call expose_page_table successfully!\n");
	struct task_struct *temp = current;
	unsigned long index1 = 1, index2 = 2;

	printk("vm_start : %p\n", temp->mm->mmap->vm_start);
	printk("vm_end : %p\n", temp->mm->mmap->vm_end);
	index1 = pgd_index(temp->mm->mmap->vm_start);
//	printk("index1 : %lu, index2 : %lu\n", index1, index2);
	struct vm_area_struct *temp1 = temp->mm->mmap->vm_next;
	printk("vm_start : %lx\n", temp1->vm_start);
        printk("vm_end : %lx\n", temp1->vm_end);
	index2 = pgd_index(temp1->vm_start);
//	printk("index1 : %lu, index2 : %lu\n", index1, index2);


//////////////////////////////////////////////////////


//	mmap(*addr, length, PROT_READ, MAP_SHARED);
/* remap_pfn_range is used for remapping  page table entry */
	//remap_pfn_range ()



}
