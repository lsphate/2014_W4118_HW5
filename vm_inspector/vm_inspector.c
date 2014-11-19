#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>

#define PTRS_PER_PGD	2048
#define PAGE_SIZE	4096
#define L_PTE_PRESENT	(1UL << 0)
#define L_PTE_YOUNG	(1UL << 1)
#define L_PTE_FILE	(1UL << 2)
#define L_PTE_DIRTY	(1UL << 6)
#define L_PTE_RDONLY	(1UL << 7)
#define L_PTE_XN	(1UL << 9)

void PrintFakePgd(unsigned long remap_pte)
{
	unsigned long phys;
	unsigned long young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit;

	phys = remap_pte >> 12;
	young_bit =	(remap_pte & L_PTE_YOUNG)	>> 1;
	file_bit =	(remap_pte & L_PTE_FILE)	>> 2;
	dirty_bit =	(remap_pte & L_PTE_DIRTY)	>> 6;
	rdonly_bit =	(!((remap_pte & L_PTE_RDONLY)	>> 7));
	xn_bit =	(remap_pte & L_PTE_XN)		>> 9;

	printf("0x%lx\t\t%d\t%d\t%d\t%d\t%d\n",
			phys,
			(unsigned int)young_bit,
			(unsigned int)file_bit,
			(unsigned int)dirty_bit,
			(unsigned int)rdonly_bit,
			(unsigned int)xn_bit);
}

int main(int argc, char **argv)
{
	int pid, fd, ret, v;
	unsigned long *base, *fake_pgd;

	base = NULL;
	fake_pgd = NULL;
	if (argv[2] == '\0') {
		pid = atoi(argv[1]);
		v = 0;
	} else {
		pid = atoi(argv[2]);
		v = 1;
	}
	printf("pid : %d, v : %d\n", pid, v);
	if (pid < -1)
		return -EINVAL;

	fd = open("/dev/zero", O_CREAT);
	if (fd < 0)
		perror("file open error.\n");

	base = mmap(NULL, PTRS_PER_PGD * (2^10) * sizeof(unsigned long),
			PROT_READ, MAP_PRIVATE, fd, 0);
	if (base < 0) {
		perror("mmap error.\n");
		return -1;
	}

	ret = posix_memalign((void **)&fake_pgd, PAGE_SIZE,
			PTRS_PER_PGD * sizeof(unsigned long));
	if (ret) {
		perror("posix_menalign error.\n");
		return -1;
	}

	ret = syscall(223, pid, (unsigned long)(fake_pgd),
			(unsigned long)base);
	if (ret < 0)
		return ret;
/*====================Print starts below here====================*/
	unsigned long va_iter, fake_pgd_index, pte_index;
	unsigned long remap_pte_base, remap_pte;
	unsigned long *fake_pgd_iter;

	fake_pgd_iter = fake_pgd;
	printf("[index]\t[virt]\t\t\t[phys]\t\t[y]\t[f]\t[d]\t[rdo]\t[xn]\n");

	for (va_iter = 0x0; va_iter < 0xc0000000; va_iter += 0x1000) {
		pte_index = (va_iter & 0x001ff000) >> 12;
		fake_pgd_index = va_iter >> 21;
		fake_pgd_iter = fake_pgd + fake_pgd_index;
		if (v == 0) {
			if (!(*fake_pgd_iter)) {
				continue;
			} else {
				remap_pte_base = *fake_pgd_iter;
				remap_pte = remap_pte_base + pte_index;
				printf("0x%d\t0x%08lx\t\t",
						(unsigned int)fake_pgd_index,
						va_iter);
				PrintFakePgd(remap_pte);
			}
		} else {
			if (!(*fake_pgd_iter)) {
				printf("0x%d\t0x%08lx\t\t0\t0\t0\t0\t0\t0\t\n",
					(unsigned int)fake_pgd_index,
					va_iter);
			} else {
				remap_pte_base = *fake_pgd_iter;
				remap_pte = remap_pte_base + pte_index;
				printf("0x%d\t0x%08lx\t\t",
						(unsigned int)fake_pgd_index,
						va_iter);
						PrintFakePgd(remap_pte);
			}
		}
	}

	close(fd);
	/*free((void*)fake_pgd);*/
	munmap((void *)base, PTRS_PER_PGD * (2^10) * sizeof(unsigned long));
	return ret;
}
