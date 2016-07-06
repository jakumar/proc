#ifndef USER_MODULE

#include <linux/module.h>	
#include <linux/kernel.h>	
#include <linux/proc_fs.h>	
#include <asm/uaccess.h>

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

