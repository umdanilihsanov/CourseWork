#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	pid_t child, pid, ppid;
	int a = 0;
	
	child = fork();
	if (child < 0) {
		printf("error\n");
		exit(1);
	}
	else if (child == 0) {
		a++;
	}
	else {
		a++;
	}
	
	pid = getpid();
	ppid = getppid();
	printf("My pid = %d, my ppid = %d, result = %d\n", (int)pid, (int)ppid, a);
	
	return 0;
}
