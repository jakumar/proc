/**
 *
 * Linux kernel module for maintaining proc fs entries
 *
 */

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

#else

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#endif

#include "proc_common.h"
#include "proc_radix_tree.h"
#include "proc_linked_list.h"
#include "proc_ioctl.h"
#include "proc_ioctl_buf.h"

#ifdef USER_MODULE
extern radix_node_t *global_root;
#endif

/** For class registration to work, you need GPL license **/
MODULE_LICENSE("GPL");

#ifndef USER_MODULE
radix_node_t *data_radix_root;

static int
procfs_seq_file_show (struct seq_file *proc_seq_fp, void *arg)
{
    int kv_index;
    kv_data_t *kv_data;
    radix_leaf_data_t *radix_data = (radix_leaf_data_t *)(proc_seq_fp->private);

    /* Check for input data */
    if (!radix_data) {
        printk("Received NULL radix data\n");
        goto err;
    }

    /* Reset the private pointer */
    proc_seq_fp->private = NULL;

    /* Get the inner key:value pairs */
    kv_data = (kv_data_t *)(radix_data->kv);
    if (!kv_data) {
        printk("No information for file\n");
        goto err;
    }

    /* Go thru all the key:value pairs and print them to the seq file */
    for (kv_index = 0; kv_index < radix_data->num_kv; kv_index++) {
        seq_printf(proc_seq_fp, "%-10s:%s\n", kv_data->key, kv_data->value);
        printk("%s:%s\n", kv_data->key, kv_data->value);

        kv_data++;
    }

    return EOK;

err:
    return EFAIL;
}

static int
procfs_open (struct inode *proc_inode, struct file *proc_file)
{
    char proc_file_name[LKM_OUTER_KEY_LEN], *err;
    radix_leaf_data_t *radix_data;

    /* Get the file name */
    err =
        dentry_path_raw(proc_file->f_path.dentry,
                        proc_file_name, LKM_OUTER_KEY_LEN);

    /* Copy the file name as per the radix key */
    snprintf(proc_file_name, LKM_OUTER_KEY_LEN, "/proc%s", err);

    printk("Open File name: %s\n", proc_file_name);

    /* Get the radix node */
    radix_data = proc_radix_retrieve(proc_file_name, data_radix_root);
    if (!radix_data) {
        printk("Unable to retrieve information for file %s\n", proc_file_name);
        return EFAIL;
    }

    /* Let single_open do the work of providing the data */
    return single_open(proc_file, procfs_seq_file_show, (void *)radix_data);
}

static const struct
file_operations procfs_file_ops = {
    .open    = procfs_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

void
procfs_create (char *file_name, struct proc_dir_entry *parent_dir)
{
    printk("Create proc file:%s\n", file_name);

    /* Create the proc file entry under the parent directory */
    proc_create(file_name, 0, parent_dir, &procfs_file_ops);
}

int
proc_process_and_add_node_to_radix_tree (void *data_object)
{
    int key_index, kv_index = 0, kv_data_size = 0;
    void *kv_data = NULL;
    proc_data_t *proc_data;
    radix_leaf_data_t *radix_data;

    /* Check for input args */
    if (!data_object) {
        printk("%s: data_object is NULL\n", __func__);
        return EFAIL;
    }

    /* Initialize the global data radix root if the already */
    if (!data_radix_root) {
        data_radix_root = proc_radix_tree_init();
        /* Ensure the global data radix root is initialized correctly */
        if (!data_radix_root) {
            printk("Failed to init data_radix_root\n");
            return EFAIL;
        }
    }

    /* Get the proc data object */
    proc_data = (proc_data_t *)data_object;

    /*
     * Go thru all the keys and inner key:value information and create
     * individual radix nodes for each of the outer keys
     */
    for (key_index = 0; key_index < proc_data->num_key; key_index++) {
        printk("Key[%d]: %s (data_num:%d)\n", key_index,
                (char *)(proc_data->key_info + key_index),
                proc_data->num_data_per_key[key_index]);

        /* Get the total inner key:value data size for this key */
        kv_data_size =
            proc_data->num_data_per_key[key_index] * sizeof(kv_data_t);

        /* Allocate the memory required for this inner key:value information */
        kv_data = kmalloc(kv_data_size, GFP_KERNEL);
        if (!kv_data) {
            printk("Memory allocation failed for kv_data for key %s\n",
                    (char *)(proc_data->key_info + key_index));
            return EFAIL;
        }

        /* Copy the inner key:value data to the newly allocated memory */
        memcpy(kv_data, proc_data->kv_data + kv_index, kv_data_size);

        /* Allocate memory for the radix data node */
        radix_data = kmalloc(sizeof(radix_leaf_data_t), GFP_KERNEL);
        if (!radix_data) {
            printk("Memory allocation failed for radix leaf data for key %s\n",
                    (char *)(proc_data->key_info + key_index));
            kfree(kv_data);
            return EFAIL;
        }

        /* 
         * Set the details of the radix node before inserting it into
         * the data radix tree
         */
        radix_data->num_kv = proc_data->num_data_per_key[key_index];
        radix_data->kv = kv_data;

        /* Perform the radix tree insert */
        proc_radix_insert((char *)(proc_data->key_info + key_index),
                          radix_data, data_radix_root);

        /* Proceed the inner kv_index to the next inner key:value data */
        kv_index += kv_data_size;
    }

    return EOK;
}
#else

#endif

#if 0
void proc_ioctl_buf_print(void *data)
{
    radix_leaf_data_t *buf = (radix_leaf_data_t *)data;
    int i;

    if (!buf) {
#ifndef USER_MODULE
        printk("\nError: Empty Buffer !!!!");
#else
        printf("\nError: Empty Buffer !!!!");
#endif
        return;
    }

    for (i=0; i < buf->num_kv;i++) {
#ifndef USER_MODULE
        printk("\n\tKey:%s Value:%s", buf->kv[i].key, buf->kv[i].value);
#else
        printf("\n\tKey:%s Value:%s", buf->kv[i].key, buf->kv[i].value);
#endif
    }
}

void proc_ioctl_buf_handler(void *data)
{
    ioctl_buf_t *buf = (ioctl_buf_t *)data;

    if (!buf) {
#ifndef USER_MODULE
        printk("\nError: Empty Buffer !!!!");
#else
        printf("\nError: Empty Buffer !!!!");
#endif
        return;
    }

#ifdef USER_MODULE
    for (int i=0;i < buf->num_recs; i++) {
        proc_radix_insert(buf->buf[i].xpath, (void *)&(buf->buf[i].data), global_root);
    }
#endif
}
#endif
