#ifdef KERNEL_MODULE

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

#include "proc_common.h"
#include "proc_linked_list.h"
#include "proc_radix_tree.h"

/** For class registration to work, you need GPL license **/
MODULE_LICENSE("GPL");

static int dev_major_number = 0;
static char proc_dir[LKM_CMD_MSG_LEN];
static char proc_file[LKM_CMD_MSG_LEN];
static struct class *char_driver_class;



extern const struct file_operations procfs_file_ops;



static void
proc_ioctl_fs_destroy(void)
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
proc_ioctl_init (void)
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
proc_ioctl_cdev_open(struct inode *inode, struct file *filp)
{
    int result = 0;
    printk("%s\n", __func__);
    return result;
}

int
proc_ioctl_cdev_close(struct inode *inode, struct file *filp)
{
    printk("%s\n", __func__);
    return 0;
}

static long
proc_ioctl_cdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
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
        result = proc_ioctl_init();
        if (result < 0) {
            printk("Error in initializing proc filesystem");
            result = -1;
            goto out;
        }
        break;

    case LKM_PROC_INFO_ADD:
        result = proc_enqueue(msg);
        if (result == EFAIL) {
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
    .open           = proc_ioctl_cdev_open,
    .release        = proc_ioctl_cdev_close,
    .unlocked_ioctl = proc_ioctl_cdev_ioctl,
};

static int
proc_ioctl_ch_driver_create(void)
{
    /* Register and get a new device number from the kernel */
    dev_major_number = register_chrdev(0, LKMDRVNAME, &cdev_file_ops);
    if (dev_major_number < 0) {
        printk("Error in allocating device number");
        return -1;
    }

    /* Create a class for this device */
    if ((char_driver_class = class_create(THIS_MODULE, "chardrv")) == NULL) {
        // Return the device number to the kernel
        unregister_chrdev(dev_major_number, LKMDRVNAME);
        return -1;
    }

   if (device_create(char_driver_class, NULL, MKDEV(dev_major_number, 0),
                     NULL, LKMDRVNAME) == NULL) {
        printk("Error in creating device");
        /* Destroy the class created for this device */
        class_destroy(char_driver_class);
        /* Return the device number to the kernel */
	unregister_chrdev(dev_major_number, LKMDRVNAME);
        return -1;
    }

    return 0;
}

static void
proc_ioctl_ch_driver_destroy(void)
{
    /* Destroy the character device */
    device_destroy(char_driver_class, MKDEV(dev_major_number, 0));

    /* unregister the driver class */
    class_unregister(char_driver_class);

    /* Destroy the class for the character device */
    class_destroy(char_driver_class);

    /* Return the device number to the kernel */
    unregister_chrdev(dev_major_number, LKMDRVNAME);
}

static int
proc_ioctl_lkm_init(void)
{
    int result;

    /* Create the character driver */
    result = proc_ioctl_ch_driver_create();
    if (result < 0) {
        return result;
    }

    strlcpy(proc_dir, "\0", LKM_CMD_MSG_LEN);
    strlcpy(proc_file, "\0", LKM_CMD_MSG_LEN);

    return result;
}

static void
proc_ioctl_lkm_exit (void)
{
    /* Free up the character device */
	proc_ioctl_ch_driver_destroy(void);

    /* Remove the proc entry */
	proc_ioctl_fs_destroy();
}

/**
 * module_init
 *
 * @brief
 *     This is the first function called when the LKM is pushed to the kernel.
 *
 * @param [in] proc_ioctl_lkm_init
 *     The function which is to be called to initialize the LKM environment.
 *
 * @return
 */
module_init(proc_ioctl_lkm_init);

/**
 * module_exit
 *
 * @brief
 *     This is the function called when the LKM is removed from the kernel.
 *
 * @param [in] proc_ioctl_lkm_exit
 *     The function that the module exit is supposed to call to clean up the LKM
 *     resources created for its use.
 *
 * @return
 */
module_exit(proc_ioctl_lkm_exit);

#endif // KERNEL_MODULE
