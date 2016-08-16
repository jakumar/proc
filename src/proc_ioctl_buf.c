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

#include "proc_common.h"
#include "proc_radix_tree.h"
#include "proc_linked_list.h"
#include "libproc_api.h"
#include "proc_ioctl.h"
#include "proc_ioctl_buf.h"


/** For class registration to work, you need GPL license **/
MODULE_LICENSE("GPL");

radix_node_t *data_radix_root;

static int
procfs_seq_file_show (struct seq_file *proc_seq_fp, void *arg)
{
    int kv_index;
    proc_schema_t *kv_data;
    radix_leaf_data_t *radix_data = (radix_leaf_data_t *)(proc_seq_fp->private);

    /* Check for input data */
    if (!radix_data) {
        printk("Received NULL radix data\n");
        goto err;
    }

    /* Reset the private pointer */
    proc_seq_fp->private = NULL;

    /* Get the inner key:value pairs */
    kv_data = (proc_schema_t *)(radix_data->kv);
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
    char proc_file_name[MAX_RADIX_KEY_SIZE], *err;
    radix_leaf_data_t *radix_data;

    /* Get the file name */
    err =
        dentry_path_raw(proc_file->f_path.dentry,
                        proc_file_name, MAX_RADIX_KEY_SIZE);

    /* Copy the file name as per the radix key */
    snprintf(proc_file_name, MAX_RADIX_KEY_SIZE, "/proc%s", err);

    printk("Open File name: %s\n", proc_file_name);

    /* Get the radix node */
    radix_data = proc_radix_retrieve(proc_file_name, NULL, data_radix_root);
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
proc_add_entry (char *key, proc_info_t *info)
{
    proc_schema_t *kv_data;
    int num_entry = info->num_entry;
    radix_leaf_data_t *node;
    int i;
    
    /* Check for input args */
    if (!info) {
        printk("%s: info is NULL\n", __func__);
        return EFAIL;
    }

    if (!(info->schema)) {
        printk("%s: schema is NULL\n", __func__);
        return EFAIL;
    }

    /* Initialize the global data radix root if the already */
    if (!data_radix_root) {
        data_radix_root = proc_radix_tree_init();

        if (!data_radix_root) {
            printk("%s: Failed to init data_radix_root\n", __func__);
            return EFAIL;
        }
    }

    kv_data = kmalloc(sizeof(proc_schema_t)*num_entry, GFP_KERNEL);
    if (copy_from_user(kv_data, info->schema,num_entry*sizeof(proc_schema_t))) {

		return EFAIL;
	}
    printk("num_entry:%d \n", info->num_entry);
    for (i=0; i < info->num_entry; i++) {
        printk("MEM: Key:%s\tValue:%s \n", kv_data[i].key, kv_data[i].value);
    }

    node = kmalloc(sizeof(radix_leaf_data_t), GFP_KERNEL);
    if (!node) {
        printk("%s(): Memory allocation failed key %s\n", __func__, key);
        kfree(kv_data);
        return EFAIL;
    }

    strncpy(node->rkey, key, MAX_RADIX_KEY_SIZE);
    node->rkey[MAX_RADIX_KEY_SIZE-1] = '\0';
    node->num_kv = num_entry;
    node->kv = (void *)kv_data;

    proc_radix_insert(key, node, data_radix_root);

    return EOK;
}

int
proc_update_entry (void *data)
{
    radix_leaf_data_t *node = (radix_leaf_data_t *)data;
    radix_leaf_data_t *pnode = NULL;
    proc_schema_t *kv_data;
    int i;
    
    /* Check for input args */
    if (!data) {
        printk("%s(): list data is NULL\n", __func__);
        return EFAIL;
    }

    if (!(node->rkey) || !(node->kv)) {
        printk("%s(): List data node key/value is NULL\n", __func__);
        return EFAIL;
    }

    if (!data_radix_root) {
        printk("%s(): Un-initialized data radix root \n", __func__);
        return EFAIL;
    }

    kv_data = (proc_schema_t *)node->kv;
    printk("num_entry:%d \n", node->num_kv);
    for (i=0; i < node->num_kv; i++) {
        printk("MEM: Key:%s\tValue:%s \n", kv_data[i].key, kv_data[i].value);
    }

    pnode = proc_radix_retrieve(node->rkey, node, data_radix_root);

    if (pnode) {
        //free up the memory
        kfree(pnode->kv);
        kfree(pnode);
    } else {
        printk("%s(): Inconsistent radix tree with empty leaf", __func__);
        return EFAIL;
    }

    return EOK;
}

