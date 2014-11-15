#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PTRS_PER_PTE 512
#define PTRS_PER_PGD 2048

int main(int argc, char **argv)
{
	int pid = atoi(argv[2]);
	if(pid < -1) 
		return -EINVAL; 
	int fd = open("/dev/zero", O_CREAT);
	unsigned long *base;
	unsigned long fake_pgd[2048][2];

	/* 2^10*sizeof(unsigned long) = each PTE size*/
	base = mmap(NULL, (2^11)*(2^10)*sizeof(unsigned long), PROT_WRITE, MAP_PRIVATE, fd, 0);
	//	printf("base : %lx\n", *base);
	//	printf("WTF\n");
	syscall(223, -1, fake_pgd, base);	
		printf("WTF\n");
	
	unsigned long i;
	for (i = 0; i < 2048; i++) {
		fake_pgd[i][0] = (unsigned long)base + i*PTRS_PER_PTE*sizeof(unsigned long) + PTRS_PER_PTE*sizeof(unsigned long);
		fake_pgd[i][1] = (unsigned long)base + 3*256*sizeof(unsigned long);
		printf("fake_pgd[0] = 0x%lx\n", fake_pgd[i][0]);
		printf("fake_pgd[1] = 0x%lx\n", fake_pgd[i][1]);
	//	unsigned long *temp;
	//	temp = (unsigned long)fake_pgd[i][0];
	//	printf("temp : %lu\n", *temp);
	}
	int j;
	for(j = 0; j < 2048; j++) {

	}
		
	return 0;
}
