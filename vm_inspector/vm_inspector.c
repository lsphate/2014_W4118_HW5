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
#define L_PTE_PRESENT	(0x1 << 0)
#define L_PTE_YOUNG	(0x1 << 1)
#define L_PTE_FILE	(0x1 << 2)
#define L_PTE_DIRTY	(0x1 << 6)
#define L_PTE_RDONLY	(0x1 << 7)
#define L_PTE_XN	(0x1 << 9)

void PrintFakePgd(int index, unsigned long fake_pgd_entry)
{
	unsigned long young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit;
	unsigned long virt, phys;

	virt = 0x00000000 + index * PAGE_SIZE;
	phys = fake_pgd_entry;
	young_bit =	fake_pgd_entry & L_PTE_YOUNG;
	file_bit =	fake_pgd_entry & L_PTE_FILE;
	dirty_bit =	fake_pgd_entry & L_PTE_DIRTY;
	rdonly_bit =	fake_pgd_entry & L_PTE_RDONLY;
	xn_bit =	fake_pgd_entry & L_PTE_XN;

	printf("0x%d\t0x%08lx\t0x%08lx\t", index, virt, phys);
	printf("%lx\t%lx\t%lx\t%lx\t%lx\n", young_bit, file_bit, dirty_bit, rdonly_bit, xn_bit);
}

int main(int argc, char **argv)
{
	int pid, fd, pgd_num, ret, v;
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
	base = mmap(NULL, pgd_num * (2^10) * sizeof(unsigned long), PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (base < 0) {
		perror("mmap error.\n");
		return -1;
	}

	ret = posix_memalign((void **)&fake_pgd, PAGE_SIZE, PTRS_PER_PGD * 2 * sizeof(unsigned long));
	if (ret) {
		perror("posix_menalign error.\n");
		return -1;
	}

	ret = syscall(223, -1, (unsigned long)(fake_pgd), (unsigned long)base);	
	if (ret < 0)
		return ret;
/*====================Print starts below here====================*/
	int iter;
	unsigned long fake_pgd_entry[2];
	
/* print everything*/
	printf("[index]\t[virt]\t\t[phys]\t\t[y]\t[f]\t[d]\t[rdo]\t[xn]\n");
	if (v == 1) {
		for (iter = 0; iter < pgd_num; iter++) {
			fake_pgd_entry[0] = (*fake_pgd);
			fake_pgd++;
			fake_pgd_entry[1] = (*fake_pgd);
			if (iter != pgd_num -1)
				fake_pgd++;

			PrintFakePgd((iter * 2), fake_pgd_entry[0]);
			PrintFakePgd((iter * 2 + 1), fake_pgd_entry[1]);
			//printf("fake_pgd[%d][0] = 0x%08lx\t", iter, fake_pgd_entry[0]);
			//printf("fake_pgd[%d][1] = 0x%08lx\n", iter, fake_pgd_entry[1]);
		}
	} else {
/* print that is not NULL */
		for (iter = 0; iter < pgd_num; iter++) {
			fake_pgd_entry[0] = (*fake_pgd);
			fake_pgd++;
			fake_pgd_entry[1] = (*fake_pgd);
			if (iter != pgd_num -1)
				fake_pgd++;

			if (fake_pgd_entry[0] != 0x00000000)
				PrintFakePgd((iter * 2), fake_pgd_entry[0]);
				//printf("fake_pgd[%d][0] = 0x%08lx\t", iter, fake_pgd_entry[0]);
			if (fake_pgd_entry[1] != 0x00000000)
				PrintFakePgd((iter * 2 + 1), fake_pgd_entry[1]);
				//printf("fake_pgd[%d][1] = 0x%08lx\n", iter, fake_pgd_entry[1]);
		}
	}

	close(fd);
//	free((void*)fake_pgd);
	munmap((void *)base, pgd_num * (2^10) * sizeof(unsigned long));
	return ret;
}
