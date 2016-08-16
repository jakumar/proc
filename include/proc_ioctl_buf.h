#ifndef IOCTL_BUF_HANDLER
#define IOCTL_BUF_HANDLER

void
procfs_create(char *file_name, struct proc_dir_entry *parent_dir);

int
proc_add_entry (char *key, proc_info_t *info);

int
 proc_update_entry (void *data);
#endif // IOCTL_BUF_HANDLER
