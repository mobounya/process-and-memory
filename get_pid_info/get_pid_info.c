#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs_struct.h>
#include <linux/path.h>
#include <linux/get_pid_info.h>
#include <linux/vmalloc.h>

static char *get_full_path(struct dentry *entry)
{
	char *buffer;
	char *full_path;
	char *temp;

	buffer = kmalloc(sizeof(char) * 256, GFP_KERNEL);

	if (buffer == NULL)
		return NULL;

	full_path = dentry_path_raw(entry, buffer, 256);
	temp = kstrdup(full_path, GFP_HIGHUSER_MOVABLE);
	kfree(buffer);
	return temp;
}

static size_t	get_number_of_children(struct list_head *children_list)
{
	size_t size;
	struct task_struct *child;
	struct list_head *list;

	size = 0;
	list_for_each(list, children_list) {
			child = list_entry(list, struct task_struct, sibling);
			size++;
	}
	return size;
}

static void		initialize_list(pid_t *children, size_t size)
{
	for (size_t i = 0; i < size; i++)
		children[i] = 0;
}

static pid_t	*get_array_of_child_procceses_pid(struct list_head *children_list, size_t *n_children)
{
	struct task_struct *child;
	struct list_head *list;
	pid_t *new_children_list;
	int index;

	index = 0;
	*n_children = get_number_of_children(children_list);
	new_children_list = kmalloc(sizeof(pid_t) * (*n_children), GFP_KERNEL);
	initialize_list(new_children_list, *n_children);
	list_for_each(list, children_list)
	{
		child = list_entry(list, struct task_struct, sibling);
		new_children_list[index] = child->pid;
		index++;
	}
	return new_children_list;
}

SYSCALL_DEFINE2(get_pid_info, struct pid_info *, data, int, pid)
{
	pid_t		*children;
	size_t	n_children;
	struct	task_struct *process_list;
	struct	path root;
	struct	path pwd;
	char		*temp;
	loff_t	off;

	off = 0;
	for_each_process(process_list) {
		if (process_list->pid == pid)
		{
			data->parent_pid = process_list->parent->pid;

			data->pid = process_list->pid;

			if (process_list->__state == -1)
				data->state = -1;
			else if (process_list->__state == TASK_RUNNING)
				data->state = TASK_RUNNING;
			else if (process_list->__state > TASK_RUNNING)
				data->state = 1;

			if (process_list->mm)
				data->stack = (void *)(process_list->mm->start_stack);
			else
				data->stack = NULL;
			data->age = process_list->start_time;

			children = get_array_of_child_procceses_pid(&(process_list->children), &n_children);
			simple_read_from_buffer(data->child_processes, 20 * sizeof(pid_t), &off, children, n_children * sizeof(pid_t));
			kfree(children);

			data->n_children = n_children;

			if (process_list->fs) {
				get_fs_root(process_list->fs, &root);
				get_fs_pwd(process_list->fs, &pwd);

				off = 0;
				temp = get_full_path(root.dentry);
				simple_read_from_buffer(data->root, 255, &off, temp, strlen(temp));
				kfree(temp);

				off = 0;
				temp = get_full_path(pwd.dentry);
				simple_read_from_buffer(data->pwd, 255, &off, temp, strlen(temp));
				kfree(temp);
			}
		}
	}

	return pid;
}
