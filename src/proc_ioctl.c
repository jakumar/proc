#ifndef USER_MODULE

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

#include "proc_common.h"
#include "proc_ioctl.h"
#include "proc_linked_list.h"
#include "proc_radix_tree.h"

/** For class registration to work, you need GPL license **/
#define MODULE_VERS "1.0"

MODULE_AUTHOR("Juniper Networks Inc.");
MODULE_DESCRIPTION("Juniper Kernel Module for Telemetry Data");
MODULE_LICENSE("GPL");

static char *jnpr_dir_name = "jnpr";
static struct proc_dir_entry *jnpr_dir_entry;
static int dev_major_number = 0;
static struct class *char_driver_class;

radix_node_t *proc_dir_root = NULL;

extern const struct file_operations procfs_file_ops;

static void
proc_ioctl_proc_fs_destroy (void)
{
    printk("Remove proc dir: %s\n", jnpr_dir_name);
    remove_proc_subtree(jnpr_dir_name, NULL);
    return;
}

static int
proc_ioctl_proc_fs_init (void)
{
    /* Create JNPR subtree under /proc */
    jnpr_dir_entry = proc_mkdir(jnpr_dir_name, NULL);
    if (!jnpr_dir_entry) {
        printk("Unable to create JNPR parent directory in proc fs\n");
        return EFAIL;
    }

    /* Create global radix root to keep proc directory entries */
    proc_dir_root = proc_radix_tree_init();
    if (!proc_dir_root) {
        printk("Unable to init proc radix root\n");
        proc_ioctl_proc_fs_destroy();
        return EFAIL;
    }

    return EOK;
}

static int
proc_ioctl_dir_create (void *msg)
{
    void *radix_data = NULL;
    int file_index, dir_index, index;
    char *dir_name;
    char tmp_dir[LKM_DIR_NAME_LEN], *tmp_dir_p;
    char tmp_proc_dir[LKM_DIR_NAME_LEN];
    proc_dir_file_t *proc_dir_file;
    struct proc_dir_entry *last_dir_entry = jnpr_dir_entry;

    if (!last_dir_entry) {
        printk("Parent JNPR directory not created\n");
        return EFAIL;
    }

    if (!msg) {
        printk("Input msg is NULL\n");
        return EFAIL;
    }

    /* Get the directory and file structure */
    proc_dir_file = (proc_dir_file_t *)msg;

    printk("proc_dir: %s\nnum_dir: %d\n", tmp_proc_dir, proc_dir_file->num_dir);

    /* Start from first file */
    file_index = 0;

    /*
    * Go thru all the directories and create them one after the other.
    * Create the files in the directory before proceeding to create the next
    * directories and files.
    */
    for (dir_index = 0; dir_index < proc_dir_file->num_dir; dir_index++) {
        /* Set the well-known file prefix */
        snprintf(tmp_proc_dir, LKM_DIR_NAME_LEN, "/proc/%s", jnpr_dir_name);

        printk("Dir[%d]: %s\n", dir_index,
                (char *)(proc_dir_file->dir_names + dir_index));
        
        /* This is the top level JNPR directory created under /proc */
        last_dir_entry = jnpr_dir_entry;

        /* Get the directory name */
        memcpy(tmp_dir, proc_dir_file->dir_names + dir_index, LKM_DIR_NAME_LEN);
        tmp_dir_p = tmp_dir;

        /* 
         * Parse the complete directory name with '/' as the seperator between
         * individual directories
         */
        while ((dir_name = strsep(&tmp_dir_p, "/")) != NULL) {
            /* Append this directory name to the top level directory */
            snprintf(tmp_proc_dir + strlen(tmp_proc_dir),
                     LKM_DIR_NAME_LEN, "/%s", dir_name);

            printk("Next dir: %s\n", tmp_proc_dir);

            /* Check if this directory already exists */
            radix_data = proc_radix_retrieve(tmp_proc_dir, proc_dir_root);
            if (radix_data) {
                printk("Proc dir %s already present\n", tmp_proc_dir);
                /* 
                 * If present in radix tree assume this directory is already
                 * created and move on to the next one
                 */
                last_dir_entry = (struct proc_dir_entry *)(radix_data);
                continue;
            }

            printk("Adding dir: %s\n", dir_name);

            /* Add new directory under it's parent directory */
            last_dir_entry = proc_mkdir(dir_name, last_dir_entry);
            if (!last_dir_entry) {
                printk("Failed to add proc dir %s\n", tmp_proc_dir);
                return EFAIL;
            } else {
                /* Add the newly added directory pointer to the radix tree */
                if (proc_radix_insert(tmp_proc_dir, last_dir_entry,
                                      proc_dir_root) == EFAIL) {
                    printk("Unable to store proc dir for %s in radix tree\n",
                            tmp_proc_dir);
                    return EFAIL;
                }
            }
        }

        printk("Files[%d]: ", proc_dir_file->num_files_per_dir[dir_index]);

        /* Now create all the files under this directory */
        for (index = 0;
             index < proc_dir_file->num_files_per_dir[dir_index];
             index++, file_index++) {
            printk("%s ", (char *)(proc_dir_file->file_names + file_index));
            procfs_create((char *)(proc_dir_file->file_names + file_index),
                          last_dir_entry);
        }
        printk("\n");
    }

    return EOK;
}

int
proc_ioctl_cdev_open (struct inode *inode, struct file *filp)
{
    printk("%s\n", __func__);
    return EOK;
}

int
proc_ioctl_cdev_close (struct inode *inode, struct file *filp)
{
    printk("%s\n", __func__);
    return EOK;
}

static long
proc_ioctl_cdev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    int result = EOK;
    lkm_cmd_msg_t *cmd_msg;

    printk("Ioctl is called\r\n");

    if (!arg) {
        printk("Input args NULL\n");
        result = EFAIL;
        goto out;
    }
    cmd_msg = (lkm_cmd_msg_t *)arg;

    printk("cmd: %d\n", cmd);
    switch (cmd) {
    case LKM_PROC_DIR_CMD:
        /* Initialize the proc filesystem */
        printk("at LKM_PROC_DIR_CMD\n");
        result = proc_ioctl_dir_create(cmd_msg->msg);
        if (result < 0) {
            printk("Error in initializing proc filesystem\n");
            result = EFAIL;
            goto out;
        }
        break;

    case LKM_PROC_INFO_ADD:
        printk("at LKM_PROC_INFO_ADD\n");
        result = proc_enqueue(cmd_msg->msg);
        if (result == EFAIL) {
            printk("Unable to process proc info successfully\n");
            result = EFAIL;
            goto out;
        }
        timer_callback(1);
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
proc_ioctl_ch_driver_create (void)
{
    /* Register and get a new device number from the kernel */
    dev_major_number = register_chrdev(0, LKMDRVNAME, &cdev_file_ops);
    if (dev_major_number < 0) {
        printk("Error in allocating device number");
        return EFAIL;
    }

    /* Create a class for this device */
    if ((char_driver_class = class_create(THIS_MODULE, "chardrv")) == NULL) {
        /* Return the device number to the kernel */
        unregister_chrdev(dev_major_number, LKMDRVNAME);
        return EFAIL;
    }

    /* Create the character device */
   if (device_create(char_driver_class, NULL, MKDEV(dev_major_number, 0),
                     NULL, LKMDRVNAME) == NULL) {
        printk("Error in creating device");
        /* Destroy the class created for this device */
        class_destroy(char_driver_class);
        /* Return the device number to the kernel */
	unregister_chrdev(dev_major_number, LKMDRVNAME);
        return EFAIL;
    }

    return EOK;
}

static void
proc_ioctl_ch_driver_destroy (void)
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
proc_ioctl_lkm_init (void)
{
    int result;

    /* Create the character driver */
    result = proc_ioctl_ch_driver_create();
    if (result < 0) {
        return result;
    }

    /* Init proc related details */
    result = proc_ioctl_proc_fs_init();
    if (result < 0) {
        proc_ioctl_ch_driver_destroy(); 
        return result;
    }

    return result;
}

static void
proc_ioctl_lkm_exit (void)
{
    /* Free up the character device */
    proc_ioctl_ch_driver_destroy();

    /* Remove the proc entry */
    proc_ioctl_proc_fs_destroy();
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

#endif // USER_MODULE
