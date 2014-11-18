#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>

#define PTRS_PER_PTE 	512
#define PTRS_PER_PGD 	2048
#define PAGE_SIZE 	4096
#define L_PTE_PRESENT	(1UL << 0)
#define L_PTE_YOUNG	(1UL << 1)
#define L_PTE_FILE	(1UL << 2)
#define L_PTE_DIRTY	(1UL << 6)
#define L_PTE_RDONLY	(1UL << 7)
#define L_PTE_XN	(1UL << 9)

void PrintFakePgd(unsigned long remap_pte)
{
	unsigned long young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit;
	unsigned long phys;

	phys = remap_pte >> 12;
	young_bit =	remap_pte & L_PTE_YOUNG;
	file_bit =	remap_pte & L_PTE_FILE;
	dirty_bit =	remap_pte & L_PTE_DIRTY;
	rdonly_bit =	remap_pte & L_PTE_RDONLY;
	xn_bit =	remap_pte & L_PTE_XN;

	printf("0x%08lx\t\t%lx\t%lx\t%lx\t%lx\t%lx\n", phys, young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit);
}

int main(int argc, char **argv)
{
	int pid, fd, pgd_num, ret, v = 0;
	unsigned long *base, *fake_pgd;

	base = NULL;
	fake_pgd = NULL;
	if (argv[2] == '\0') {
		pid = atoi(argv[1]);
		v = 0;
	}
	else {
		pid = atoi(argv[2]);
		v = 1;
	}

/* this printf will cause crashing = = */
//	printf ("pid : %d\n", pid);

	if (pid < -1)
		return -EINVAL;

	fd = open("/dev/zero", O_CREAT);
	if (fd < 0)
		perror("file open error.\n");

	pgd_num = (PTRS_PER_PGD / 4) * 3;
	base = mmap(NULL, PTRS_PER_PGD * (2^10) * sizeof(unsigned long), PROT_READ, MAP_SHARED, fd, 0);
	if (base < 0) {
		perror("mmap error.\n");
		return -1;
	}

	ret = posix_memalign((void **)&fake_pgd, PAGE_SIZE, PTRS_PER_PGD * sizeof(unsigned long));
	if (ret) {
		perror("posix_menalign error.\n");
		return -1;
	}

	ret = syscall(223, pid, (unsigned long)(fake_pgd), (unsigned long)base);	
	if (ret < 0)
		return ret;
/*====================Print starts below here====================*/
	unsigned long pgd_iter, fake_pgd_index, pte_index;
	unsigned long remap_pte_base, remap_pte;
	
/* print everything*/
	printf("[index]\t[virt]\t\t[phys]\t\t[y]\t[f]\t[d]\t[rdo]\t[xn]\n");

	for (pgd_iter = 0x00000000; pgd_iter < 0xc0000000; pgd_iter += 0x00001000) {
		fake_pgd_index = pgd_iter >> 21;

		if (!(*(fake_pgd += fake_pgd_index)))
			remap_pte_base = 0x00000000;
		else
			remap_pte_base = *(fake_pgd += fake_pgd_index);

		for (pte_index = 0x0; pte_index < 0x200; pte_index += 0x1) {
			if (v == 1 && remap_pte_base == 0x00000000)
				printf("0x%lx\t0x%08lx\t\t0\t\t0\t0\t0\t0\t0\n", fake_pgd_index, remap_pte);
			else if (v == 0 || remap_pte_base != 0x00000000) {
				remap_pte = remap_pte_base + pte_index;
				if (!(*(unsigned long *)remap_pte))
					continue;
				printf("0x%lx\t0x%08lx\t\t", fake_pgd_index, remap_pte);
				PrintFakePgd(*(unsigned long *)remap_pte);
			}
		}
	}
/*
	if (v == 1) {
		for (iter = 0; iter < pgd_num; iter++) {
			remap_pte = (*fake_pgd);
			fake_pgd++;
			remap_pte = (*fake_pgd);
			if (iter != pgd_num -1)
				fake_pgd++;
			if (remap_pte != 0x00000000)
				PrintFakePgd(iter, remap_pte);
			else
				printf("Skip fake_pgd: %d\n", iter);
			//PrintFakePgd((iter * 2 + 1), remap_pte);
		}
	} else {
		for (iter = 0; iter < pgd_num; iter++) {
			remap_pte[0] = (*fake_pgd);
			fake_pgd++;
			remap_pte[1] = (*fake_pgd);
			if (iter != pgd_num -1)
				fake_pgd++;

			if ((remap_pte[0] & 0xff) != 0x00000000)
				PrintFakePgd((iter * 2), remap_pte[0]);
				//printf("fake_pgd[%d][0] = 0x%08lx\t", iter, fake_pgd_entry[0]);
			if ((remap_pte[1] & 0xff) != 0x00000000)
				PrintFakePgd((iter * 2 + 1), remap_pte[1]);
				//printf("fake_pgd[%d][1] = 0x%08lx\n", iter, fake_pgd_entry[1]);
		}
	}
*/
	close(fd);
//	free((void*)fake_pgd);
	munmap((void *)base, pgd_num * (2^10) * sizeof(unsigned long));
	return ret;
}
