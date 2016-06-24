/**
 *
 * Linux kernel module for maintaining proc fs entries
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/utsname.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/seq_file.h>
#include "lkm_proc_ioctl_kernel.h"
#include "lkm_linked_list.h"

/** For class registration to work, you need GPL license **/
MODULE_LICENSE("GPL");

static int device_major_number = 0;
static char proc_dir[LKM_CMD_MSG_LEN];
static char proc_file[LKM_CMD_MSG_LEN];
static struct class *char_driver_class;
static void* ll_data[10];
static int ll_data_size[10];

static void proc_add_node_to_radix_tree(void);

static int
procfs_test_show (struct seq_file *proc_fp, void *arg)
{
    int index, data_index;
    lkm_list_data_t *inner_kv;

    /* Add to radix tree on dequeue */
    proc_add_node_to_radix_tree();

    for (index = 0; index < 10; index++) {
        if (ll_data[index]) {
            for (data_index = 0;
                 data_index < ll_data_size[index];
                 data_index++) {
                inner_kv =
                    ll_data[index] + (data_index * sizeof(lkm_list_data_t));
                seq_printf(proc_fp, "%-10s:%s\n",
                           inner_kv->inner_key, inner_kv->inner_value);
                printk("%s:%s\n", inner_kv->inner_key, inner_kv->inner_value);
            }
        }
    }

    return 0;
}

static int
procfs_open (struct inode *inode, struct file *file)
{
    return single_open(file, procfs_test_show, NULL);
}

static const struct
file_operations procfs_file_ops = {
    .open           = procfs_open,
    .read           = seq_read,
    .llseek         = seq_lseek,
    .release        = single_release,
};

static void
proc_add_node_to_radix_tree (void)
{
    void *data;
    proc_data_t *proc_data;
    int start_data, index, rec_index, total_rec, total_data_len, num_data;
    char *tmp_char, tmp_buf[100] = { 0 };
    char *o_key_value, *i_key_value;
    lkm_list_data_t *inner_kv = NULL;

    while ((data = list_remove_head()) != NULL) {
    proc_data = (proc_data_t *)data;

    total_rec = proc_data->total;

    printk("proc_info: %s\n", proc_data->proc_info);

    /* TODO: this has to be updated to accept multiple list nodes */
    for (rec_index = 0; rec_index < total_rec; rec_index++) {
        start_data = proc_data->kv_md[rec_index].start_data;
        total_data_len = proc_data->kv_md[rec_index].total_data_len;
        num_data = proc_data->kv_md[rec_index].num_data;

        ll_data[rec_index] =
            kmalloc(num_data * sizeof(lkm_list_data_t), GFP_KERNEL);
        if (!ll_data[rec_index]) {
            printk("Memory allocation failed for ll_data\n");
            goto out;
        }
        ll_data_size[rec_index] = num_data;
        memcpy(tmp_buf, proc_data->proc_info + start_data, total_data_len);
        tmp_char = tmp_buf;
        index = 0;
        while ((o_key_value = strsep(&tmp_char, "$")) != NULL) {
            inner_kv = ll_data[rec_index] + (index * sizeof(lkm_list_data_t));
            if ((i_key_value = strsep(&o_key_value, ":")) != NULL) {
                strlcpy(inner_kv->inner_key, i_key_value, INNER_KEY_LEN);
                strlcpy(inner_kv->inner_value, o_key_value, INNER_VALUE_LEN);
            }
            index++;
        }
        memset(tmp_buf, 0, sizeof(tmp_buf));
        //TBD: ADD ll_data as radix tree node's data
    }
    }

out:
    return;
}

static int
procfs_add_data (char *msg)
{
    if (!msg) {
        printk("%s: Input data is NULL\n", __func__);
        goto out;
    }

    /* Create linked list node's data */
    if (list_insert_tail((void *)msg)) {
        return -1;
    }

out:
    return 0;
}

static void
procfs_destroy (void)
{
    char *dir_name;
    char tmp_dir[LKM_CMD_MSG_LEN], *tmp_dir_p;

    /* Remove the proc filesystem entry created earlier */
    if (strlen(proc_dir)) {
        strlcpy(tmp_dir, proc_dir, LKM_CMD_MSG_LEN);
        tmp_dir_p = tmp_dir;
    } else {
        printk("%s: PROC DIR is NULL\n", __func__);
        return;
    }

    if ((dir_name = strsep(&tmp_dir_p, "/")) != NULL) {
        printk("Remove proc dir: %s\n", dir_name);
        remove_proc_subtree(dir_name, NULL);
    }

    return;
}

static int
procfs_init (void)
{
    char *dir_name;
    char tmp_dir[LKM_CMD_MSG_LEN], *tmp_dir_p;
    struct proc_dir_entry *last_dir_name = NULL;

    if (strlen(proc_dir)) {
        strlcpy(tmp_dir, proc_dir, LKM_CMD_MSG_LEN);
        tmp_dir_p = tmp_dir;
    } else {
        printk("%s: PROC DIR is NULL\n", __func__);
        return -1;
    }

    while ((dir_name = strsep(&tmp_dir_p, "/")) != NULL) {
        printk("Next dir: %s\n", dir_name);
        last_dir_name = proc_mkdir(dir_name, last_dir_name);
    }

    proc_create(proc_file, 0, last_dir_name, &procfs_file_ops);

    return 0;
}

int
cdev_open (struct inode *inode, struct file *filp)
{
    int result = 0;
    printk("%s\n", __func__);
    return result;
}

int
cdev_close (struct inode *inode, struct file *filp)
{
    printk("%s\n", __func__);
    return 0;
}

static long
cdev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    int result = 0;
    lkm_cmd_msg_t *cmd_msg;

    printk("Ioctl is called\r\n");

    if (!arg) {
        printk("Input args NULL\n");
        result = -1;
        goto out;
    }
    cmd_msg = (lkm_cmd_msg_t *)arg;

    switch (cmd) {
    case LKM_PROC_DIR_CMD:
        /* Set the global proc directory structure */
        strlcpy(proc_dir, cmd_msg->msg, LKM_CMD_MSG_LEN - 1);
        strlcpy(proc_file, cmd_msg->msg_2, LKM_CMD_MSG_LEN - 1);

        printk("Received proc dir %s, file %s\n", proc_dir, proc_file);

        /* Initialize the proc filesystem */
        result = procfs_init();
        if (result < 0) {
            printk("Error in initializing proc filesystem");
            result = -1;
            goto out;
        }
        break;

    case LKM_PROC_INFO_ADD:
        /* Copy the interface data */
        result = procfs_add_data(cmd_msg->msg);
        if (result < 0) {
            printk("Unable to process proc info successfully\n");
            result = -1;
            goto out;
        }
        break;

    default:
        break;
    }

out:
    printk("return from Ioctl\r\n");
    return result;
}

static struct
file_operations cdev_file_ops = {
    .open           = cdev_open,
    .release        = cdev_close,
    .unlocked_ioctl = cdev_ioctl,
};

static int
char_driver_create (void)
{
    printk("Initialize character device for LKM\n");

    /* Register and get a new device number from the kernel */
    device_major_number = register_chrdev(0, LKMDRVNAME, &cdev_file_ops);
    if (device_major_number < 0) {
        printk("Error in allocating device number");
        return -1;
    }

    printk("Kernel assigned major number is %d ..\r\n", device_major_number);

    /* Create a class for this device */
    if ((char_driver_class = class_create(THIS_MODULE, "chardrv")) == NULL) {
        printk("Error in class_create for device");
        /* Return the device number to the kernel */
	unregister_chrdev(device_major_number, LKMDRVNAME);
        return -1;
    }

   if (device_create(char_driver_class, NULL, MKDEV(device_major_number, 0),
                     NULL, LKMDRVNAME) == NULL) {
        printk("Error in creating device");
        /* Destroy the class created for this device */
        class_destroy(char_driver_class);
        /* Return the device number to the kernel */
	unregister_chrdev(device_major_number, LKMDRVNAME);
        return -1;
    }

    return 0;
}

static void
char_driver_destroy (void)
{
    /* Destroy the character device */
    device_destroy(char_driver_class, MKDEV(device_major_number, 0));

    /* unregister the driver class */
    class_unregister(char_driver_class);

    /* Destroy the class for the character device */
    class_destroy(char_driver_class);

    /* Return the device number to the kernel */
    unregister_chrdev(device_major_number, LKMDRVNAME);

    printk("Remove character device %d\n", device_major_number);
}

static int
lkm_init (void)
{
    int result;

    /* Create the character driver */
    result = char_driver_create();
    if (result < 0) {
        return result;
    }

    strlcpy(proc_dir, "\0", LKM_CMD_MSG_LEN);
    strlcpy(proc_file, "\0", LKM_CMD_MSG_LEN);

    return result;
}

static void
lkm_exit (void)
{
    /* Free up the character device */
    char_driver_destroy();

    /* Remove the proc entry */
    procfs_destroy();
}

/**
 * module_init
 *
 * @brief
 *     This is the first function called when the LKM is pushed to the kernel.
 *
 * @param [in] lkm_init
 *     The function which is to be called to initialize the LKM environment.
 *
 * @return
 */
module_init(lkm_init);

/**
 * module_exit
 *
 * @brief
 *     This is the function called when the LKM is removed from the kernel.
 *
 * @param [in] lkm_exit
 *     The function that the module exit is supposed to call to clean up the LKM
 *     resources created for its use.
 *
 * @return
 */
module_exit(lkm_exit);
