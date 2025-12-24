#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
	pid_t pid1, pid2;
	
	pid1 = fork();
	if (pid1 < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	

	if (pid1 == 0) {
		// процесс-потомок 1
		char *argv[] = {"./lab1", NULL};
		execvp(argv[0], argv);
		perror("execvp lab1");
		exit(EXIT_FAILURE);
	}
	else {
		// родительский процесс ждет завершения первого потомка
		waitpid(pid1, NULL, 0);
	}
	
	if (pid2 == 0) {
		// процесс-потомок 2
		char *argv[] = {"./lab2", NULL};
		execvp(argv[0], argv);
		perror("execvp lab2");
		exit(EXIT_FAILURE);
	}
	else {
		// родительский процесс ждет завершения второго потомка
		waitpid(pid1, NULL, 0);
	}
	
	return 0;
}
