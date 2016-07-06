#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */

#define MODULE_VERS "1.0"
#define MODULE_NAME "cae_data"

#define PROCFS_MAX_SIZE		1024

/*
 * various files under procfs cae_data
 */
#define PROCFS_ASIC_NAME    "asic"
#define PROCFS_IF_NAME      "interface"
#define PROCFS_Q_NAME       "queue"
#define PROCFS_ROUTE_NAME   "route"
#define PROCFS_FILTER_NAME  "filter"
#define PROCFS_HIF_NAME     "host_interface"

static RADIX_TREE(test_tree, GFP_KERNEL);

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *cae_dir;
static struct proc_dir_entry *asic_file;
static struct proc_dir_entry *if_file;
static struct proc_dir_entry *queue_file;
static struct proc_dir_entry *route_file;
static struct proc_dir_entry *filter_file;
static struct proc_dir_entry *hif_file;

/**
 * The buffer used to store character for this module
 *
 */
static char procfs_asic_buffer[PROCFS_MAX_SIZE];
static char procfs_if_buffer[PROCFS_MAX_SIZE];
static char procfs_q_buffer[PROCFS_MAX_SIZE];
static char procfs_route_buffer[PROCFS_MAX_SIZE];
static char procfs_filter_buffer[PROCFS_MAX_SIZE];
static char procfs_hif_buffer[PROCFS_MAX_SIZE];

/*
 * The size of the buffer
 */
static unsigned long procfs_asic_buffer_size = 0;
static unsigned long procfs_if_buffer_size = 0;
static unsigned long procfs_q_buffer_size = 0;
static unsigned long procfs_route_buffer_size = 0;
static unsigned long procfs_filter_buffer_size = 0;
static unsigned long procfs_hif_buffer_size = 0;

/*
 * This function is called then the /proc file is read
 */
int 
procfile_asic_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_asic_read (/proc/%s) called\n",
            PROCFS_ASIC_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_asic_buffer, procfs_asic_buffer_size);
		ret = procfs_asic_buffer_size;
	}

	return ret;
}

int 
procfile_if_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_if_read (/proc/%s) called\n",
            PROCFS_IF_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_if_buffer, procfs_if_buffer_size);
		ret = procfs_if_buffer_size;
	}

	return ret;
}

int 
procfile_q_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_q_read (/proc/%s) called\n",
            PROCFS_Q_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_q_buffer, procfs_q_buffer_size);
		ret = procfs_q_buffer_size;
	}

	return ret;
}

int 
procfile_route_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_route_read (/proc/%s) called\n",
            PROCFS_ROUTE_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_route_buffer, procfs_route_buffer_size);
		ret = procfs_route_buffer_size;
	}

	return ret;
}

int 
procfile_filter_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_filter_read (/proc/%s) called\n",
            PROCFS_FILTER_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_filter_buffer, procfs_filter_buffer_size);
		ret = procfs_filter_buffer_size;
	}

	return ret;
}

int 
procfile_hif_read(char *buffer, char **buffer_location,
    off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_hif_read (/proc/%s) called\n",
            PROCFS_HIF_NAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_hif_buffer, procfs_hif_buffer_size);
		ret = procfs_hif_buffer_size;
	}

	return ret;
}

/*
 * This function is called with the /proc file is written
 */
int procfile_asic_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_asic_buffer_size = count;
	if (procfs_asic_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_asic_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if ( copy_from_user(procfs_asic_buffer, buffer, procfs_asic_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_asic_buffer_size;
}

int procfile_if_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_if_buffer_size = count;
	if (procfs_if_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_if_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if ( copy_from_user(procfs_if_buffer, buffer, procfs_if_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_if_buffer_size;
}

int procfile_q_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_q_buffer_size = count;
	if (procfs_q_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_q_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if ( copy_from_user(procfs_q_buffer, buffer, procfs_q_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_q_buffer_size;
}

int procfile_route_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_route_buffer_size = count;
	if (procfs_route_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_route_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if (copy_from_user(procfs_route_buffer, buffer, procfs_route_buffer_size)) {
		return -EFAULT;
	}
	
	return procfs_route_buffer_size;
}

int procfile_filter_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_filter_buffer_size = count;
	if (procfs_filter_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_filter_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if (copy_from_user(procfs_filter_buffer, buffer, procfs_filter_buffer_size)) {
		return -EFAULT;
	}
	
	return procfs_filter_buffer_size;
}

int procfile_hif_write(struct file *file, const char *buffer,
    unsigned long count, void *data)
{
	/*
     * get buffer size
     */
	procfs_hif_buffer_size = count;
	if (procfs_hif_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_hif_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/*
     * write data to the buffer
     */
	if (copy_from_user(procfs_hif_buffer, buffer, procfs_hif_buffer_size)) {
		return -EFAULT;
	}
	
	return procfs_hif_buffer_size;
}

/*
 * This function is called when the module is loaded
 */
int init_module()
{
    /* 
     * create cae directory
     */
    cae_dir = proc_mkdir(MODULE_NAME, NULL);
    if (cae_dir == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
               MODULE_NAME);
		return -ENOMEM;
	}
	printk(KERN_INFO "/proc/%s created\n", MODULE_NAME);	

	/* 
     * create the /proc/cae_data/asic file
     */
	asic_file = create_proc_entry(PROCFS_ASIC_NAME, 0644, cae_dir);
	if (asic_file == NULL) {
		remove_proc_entry(PROCFS_ASIC_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_ASIC_NAME);
		return -ENOMEM;
	}
	asic_file->read_proc    = procfile_asic_read;
	asic_file->write_proc   = procfile_asic_write;
	asic_file->mode         = S_IFREG | S_IRUGO;
	asic_file->uid          = 0;
	asic_file->gid          = 0;
	asic_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_ASIC_NAME);	

	/* 
     * create the /proc/cae_data/interface file
     */
	if_file = create_proc_entry(PROCFS_IF_NAME, 0644, cae_dir);
	if (if_file == NULL) {
		remove_proc_entry(PROCFS_IF_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_IF_NAME);
		return -ENOMEM;
	}
	if_file->read_proc    = procfile_if_read;
	if_file->write_proc   = procfile_if_write;
	if_file->mode         = S_IFREG | S_IRUGO;
	if_file->uid          = 0;
	if_file->gid          = 0;
	if_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_IF_NAME);	

	/* 
     * create the /proc/cae_data/queue file
     */
	queue_file = create_proc_entry(PROCFS_Q_NAME, 0644, cae_dir);
	if (queue_file == NULL) {
		remove_proc_entry(PROCFS_Q_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_Q_NAME);
		return -ENOMEM;
	}
	queue_file->read_proc    = procfile_q_read;
	queue_file->write_proc   = procfile_q_write;
	queue_file->mode         = S_IFREG | S_IRUGO;
	queue_file->uid          = 0;
	queue_file->gid          = 0;
	queue_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_Q_NAME);	

	/* 
     * create the /proc/cae_data/route file
     */
	route_file = create_proc_entry(PROCFS_ROUTE_NAME, 0644, cae_dir);
	if (route_file == NULL) {
		remove_proc_entry(PROCFS_ROUTE_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_ROUTE_NAME);
		return -ENOMEM;
	}
	route_file->read_proc    = procfile_route_read;
	route_file->write_proc   = procfile_route_write;
	route_file->mode         = S_IFREG | S_IRUGO;
	route_file->uid          = 0;
	route_file->gid          = 0;
	route_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_ROUTE_NAME);	

	/* 
     * create the /proc/cae_data/filter file
     */
	filter_file = create_proc_entry(PROCFS_FILTER_NAME, 0644, cae_dir);
	if (filter_file == NULL) {
		remove_proc_entry(PROCFS_FILTER_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_FILTER_NAME);
		return -ENOMEM;
	}
	filter_file->read_proc    = procfile_filter_read;
	filter_file->write_proc   = procfile_filter_write;
	filter_file->mode         = S_IFREG | S_IRUGO;
	filter_file->uid          = 0;
	filter_file->gid          = 0;
	filter_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_FILTER_NAME);

	/* 
     * create the /proc/cae_data/filter file
     */
	hif_file = create_proc_entry(PROCFS_HIF_NAME, 0644, cae_dir);
	if (hif_file == NULL) {
		remove_proc_entry(PROCFS_HIF_NAME, cae_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_HIF_NAME);
		return -ENOMEM;
	}
	hif_file->read_proc    = procfile_hif_read;
	hif_file->write_proc   = procfile_hif_write;
	hif_file->mode         = S_IFREG | S_IRUGO;
	hif_file->uid          = 0;
	hif_file->gid          = 0;
	hif_file->size         = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_HIF_NAME);

	return 0;	/* everything is ok */
}

/**
 *This function is called when the module is unloaded
 *
 */
void cleanup_module()
{
	remove_proc_entry(PROCFS_ASIC_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_ASIC_NAME);

	remove_proc_entry(PROCFS_IF_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_IF_NAME);

	remove_proc_entry(PROCFS_Q_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_Q_NAME);

	remove_proc_entry(PROCFS_ROUTE_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_ROUTE_NAME);

	remove_proc_entry(PROCFS_FILTER_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_FILTER_NAME);

	remove_proc_entry(PROCFS_HIF_NAME, cae_dir);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_HIF_NAME);

	remove_proc_entry(MODULE_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", MODULE_NAME);
}

MODULE_AUTHOR("Jai Kumar");
MODULE_DESCRIPTION("CAE Data");
MODULE_LICENSE("GPL");
