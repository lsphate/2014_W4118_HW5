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

	printf("0x%lx\t\t%lx\t%lx\t%lx\t%lx\t%lx\n", phys, young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit);
}

int main(int argc, char **argv)
{
	int pid, fd, pgd_num, ret;
	unsigned long *base, *fake_pgd;

	base = NULL;
	fake_pgd = NULL;
	if (argv[2] == '\0') {
		pid = atoi(argv[1]);
		//v = 0;
	}
	else {
		pid = atoi(argv[2]);
		//v = 1;
	}

/* this printf will cause crashing = = */
//	printf ("pid : %d\n", pid);

	if (pid < -1)
		return -EINVAL;

	fd = open("/dev/zero", O_CREAT);
	if (fd < 0)
		perror("file open error.\n");

	pgd_num = (PTRS_PER_PGD / 4) * 3;
	base = mmap(NULL, PTRS_PER_PGD * (2^10) * sizeof(unsigned long), PROT_READ, MAP_PRIVATE, fd, 0);
	if (base < 0) {
		perror("mmap error.\n");
		return -1;
	}

	ret = posix_memalign((void **)&fake_pgd, PAGE_SIZE, PTRS_PER_PGD * sizeof(unsigned long));
	if (ret) {
		perror("posix_menalign error.\n");
		return -1;
	}

	ret = syscall(223, -1, (unsigned long)(fake_pgd), (unsigned long)base);	
	if (ret < 0)
		return ret;
/*====================Print starts below here====================*/
	unsigned long va_iter, fake_pgd_index, pte_index;
	unsigned long remap_pte_base, remap_pte;
	unsigned long *fake_pgd_temp;
/* print everything*/
	fake_pgd_temp = fake_pgd;
	printf("[index]\t[virt]\t\t[phys]\t\t[y]\t[f]\t[d]\t[rdo]\t[xn]\n");


	for (va_iter = 0x00000000; va_iter < 0xc0000000; va_iter+=0x00001000) {
		pte_index = (va_iter & 0x001ff000) >> 12;
		fake_pgd_index = va_iter >> 21;
		printf("%lx\t%lx\n", fake_pgd_index, pte_index);
		fake_pgd_temp += fake_pgd_index;
		printf("%p\n", fake_pgd_temp);

		if (!(*fake_pgd_temp)) {
			printf("skip A\n");
			continue;
		} else {
			remap_pte_base = *(fake_pgd_temp += fake_pgd_index);
			printf("0x%08lx\n", remap_pte_base);
			if (remap_pte_base == 0x00000000) {
				printf("skip B\n");
				continue;
			}
			remap_pte = remap_pte_base + pte_index;
			printf("0x%08lx\n", remap_pte);
			//if (!(*(unsigned long *)remap_pte)) {
			//	continue;
			//}
			//printf("0x%lx\t0x%lx\t\t", fake_pgd_index, va_iter);
			//PrintFakePgd(remap_pte/**(unsigned long *)remap_pte*/);
		}
	}

	close(fd);
//	free((void*)fake_pgd);
	munmap((void *)base, pgd_num * (2^10) * sizeof(unsigned long));
	return ret;
}
