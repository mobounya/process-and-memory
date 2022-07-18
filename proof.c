#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <get_pid_info.h>

int main(void)
{
	struct pid_info data;
	pid_t wpid;
	int status;

	data.pwd = malloc(sizeof(char) * 255);
	data.root = malloc(sizeof(char) * 255);
	data.child_processes = malloc(sizeof(pid_t) * 20);

 	for (int i = 0; i < 10; i++) {
		if (fork() == 0) {
			sleep(10);
			return 0;
		}
	}

	syscall(548, &data, getpid());

	printf("---------------------------\n");
	printf("#n of children == %ld\n", data.n_children);
	printf("parent PID == %d\n", getpid());
	printf("---------------------------\n");

	unsigned int n_children = data.n_children;
	for (int i = 0; i < n_children; i++) {
		data.parent_pid = -1;
		data.pid = -1;
		printf("============================\n");
		printf("child pid == %d\n", data.child_processes[i]);
		get_pid_info(&data, data.child_processes[i]);
		printf("pid == %d\n", data.pid);
		if (data.parent_pid == getpid())
			printf("parent id == %d\n", data.parent_pid);
		else
			printf("ID DOES NOT MATCH\n");
		printf("============================\n");
	}

	while ((wpid = wait(&status)) > 0);
	return 0;
}
