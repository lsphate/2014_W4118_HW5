#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>

#define PTRS_PER_PTE 512
#define PTRS_PER_PGD 2048
#define PAGE_SIZE 4096

int main(int argc, char **argv)
{
	int pid, fd, pgd_num, ret;
	unsigned long *base, *fake_pgd;

	base = NULL;
	fake_pgd = NULL;
	pid = atoi(argv[2]);
	if(pid < -1)
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
/*	int i;
	printf("Base: 0x%p\n", base);
	for (i = 0; i < pgd_num; i++) {
		fake_pgd[i][0] = (unsigned long)base + (unsigned long)(i * 2 * PTRS_PER_PTE * sizeof(unsigned long) + PTRS_PER_PTE * sizeof(unsigned long));
		fake_pgd[i][1] = fake_pgd[i][0] + (unsigned long)((PTRS_PER_PTE / 2) * sizeof(unsigned long));

		printf("fake_pgd[%d][0] = 0x%lx", i, fake_pgd[i][0]);
		printf("\tfake_pgd[%d][1] = 0x%lx\n", i, fake_pgd[i][1]);
	//	unsigned long *temp;
	//	temp = (unsigned long)fake_pgd[i][0];
	//	printf("temp : %lu\n", *temp);
	}
*/
	close(fd);
	free((void *)fake_pgd);
	munmap((void *)base, pgd_num * (2^10) * sizeof(unsigned long));
	return ret;
}
