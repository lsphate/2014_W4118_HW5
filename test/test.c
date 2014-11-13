#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int result = 0;
/*	
	struct sched_param param;
	param.sched_priority = 0;
	result = syscall(156, 1, 6, &param);
*/
	result = syscall(223, -1, 0x00000001, 0x00000001);
	if(result < 0)
		perror("error");
	return result;
}
