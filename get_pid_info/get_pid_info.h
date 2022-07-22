#ifndef GET_PID_INFO_H
#define GET_PID_INFO_H

struct pid_info {
	pid_t			pid;
	unsigned int	state;
	void			*stack;
	unsigned int	age;
	size_t			n_children;
	pid_t			*child_processes;
	pid_t			parent_pid;
	char			*root;
	char			*pwd;
	unsigned long stack_size;
};

#endif /* GET_PID_INFO_H */
