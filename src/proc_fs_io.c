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

#endif // KERNEL MODULE

#include "proc_common.h"
#include "proc_radix_tree.h"
#include "proc_ioctl.h"

extern radix_node_t *global_root;

#ifndef USER_MODULE
static int
proc_fs_io_read(struct seq_file *proc_fp, void *arg)
#else
int
proc_fs_io_read(void *key, void *arg)
#endif // USER_MODULE
{
    radix_leaf_data_t *data;
    int cnt;

    if ((data = proc_radix_retrieve(key, global_root)) == NULL) {
        return EFAIL;
    }
    cnt = data->num_kv;
    while (cnt) {
        kv_data_t *kv = (kv_data_t *)&(data->kv[cnt]);
#ifndef USER_MODULE
        seq_printf(proc_fp, "%-64s:%s\n", kv->key, kv->value);
#else
        printf("%-64s:%s\n", kv->key, kv->value);
#endif    
        cnt--;
    }


    return 0;
}

#ifndef USER_MODULE
static int
proc_fs_io_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_fs_io_read, NULL);
}

const struct
file_operations procfs_file_ops = {
    .open           = proc_fs_io_open,
    .read           = seq_read,
    .llseek         = seq_lseek,
    .release        = single_release,
};
#endif // USER_MODULE
