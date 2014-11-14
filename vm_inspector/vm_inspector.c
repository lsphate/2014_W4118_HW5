#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int pid = atoi(argv[2]);
	if(pid < -1) 
		return -EINVAL; 
	int fd = open("/dev/zero", O_CREAT);
	unsigned long *base;
	unsigned long fake_pgd[2048];

	/* 2^10*sizeof(unsigned long) = each PTE size*/
	base = mmap(NULL, (2^11)*(2^9)*(2^10)*sizeof(unsigned long), PROT_WRITE, MAP_PRIVATE, fd, 0);
	printf("WTF\n");
	syscall(223, -1, fake_pgd, base);	
	
	printf("WTF\n");
	unsigned long i;
	for (i = 0; i < 2048; i++) {
		fake_pgd[i] = (unsigned long)base + i*(2^9)*(2^10)*sizeof(unsigned long);
		printf("fake_pgd[%lu] = 0x%lx\n", i, fake_pgd[i]);
	}
	
	return 0;
}
