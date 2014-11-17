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
	printf("safe until here.\n");
	
	ret = syscall(223, -1, (unsigned long)(fake_pgd), (unsigned long)base);	
	if (ret < 0)
		return ret;
	
/*====================Print starts below here====================*/
	int iter;
	unsigned long fake_pgd_entry[2];
	for (iter = 0; iter < pgd_num; iter++) {
		fake_pgd_entry[0] = (*fake_pgd);
		fake_pgd++;
		fake_pgd_entry[1] = (*fake_pgd);
		if (iter != pgd_num -1)
			fake_pgd++;

		printf("fake_pgd[%d][0] = 0x%08lx\t", iter, fake_pgd_entry[0]);
		printf("fake_pgd[%d][1] = 0x%08lx\n", iter, fake_pgd_entry[1]);
	}

	close(fd);
	free((void *)fake_pgd);
	munmap((void *)base, pgd_num * (2^10) * sizeof(unsigned long));
	return ret;
}
