#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <get_pid_info.h>

long	get_pid_info(struct pid_info *ret, int pid)
{
	return syscall(548, ret, pid);
}

void	print_proccess_info(struct pid_info *data)
{
	printf("pid == %d\n", data->pid);
	printf("state == %d\n", data->state);
	printf("stack == %p\n", data->stack);
	printf("age == %u\n", data->age);
	printf("n of children == %d\n", data->n_children);
	printf("parent_pid == %d\n", data->parent_pid);
	printf("root == %s\n", data->root);
	printf("pwd === %s\n", data->pwd);
}

int	fake_function(int *i)
{
	int temp = *i;
	*i = *i * 2;
	return temp;
}

int main(void)
{
	struct pid_info data;
	pid_t wpid;
	int status;
	unsigned int n_children;

	data.pwd = malloc(sizeof(char) * 255);
	data.root = malloc(sizeof(char) * 255);
	data.child_processes = malloc(sizeof(pid_t) * 30);

 	for (int i = 0; i < 10; i++) {
		if (fork() == 0) {
			for (int i = 0; i < 500000000; i++)
			{
				i = fake_function(&i);
			}
			sleep(60);
			return 0;
		}
	}

	sleep(1);

	get_pid_info(&data, getpid());
	
	printf("========= PARENT ===========\n");
	print_proccess_info(&data);
	printf("============================\n");

	n_children = data.n_children;

	for (int i = 0; i < n_children; i++) {
		get_pid_info(&data, data.child_processes[i]);
		printf("========= CHILD ===========\n");
		print_proccess_info(&data);
		printf("===========================\n");
	}


	while ((wpid = wait(&status)) > 0);
	
	return 0;
}
