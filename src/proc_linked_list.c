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
#include "proc_linked_list.h"
#include "proc_ioctl_buf.h"
#include "proc_ioctl.h"

linked_list_t *head = NULL;
linked_list_t *tail = NULL;

unsigned int
proc_enqueue (void *data)
{
#ifndef USER_MODULE
    linked_list_t *node = kmalloc(sizeof(linked_list_t), GFP_KERNEL);
    void *node_data = kmalloc(sizeof(proc_data_t), GFP_KERNEL);
#else
    linked_list_t *node = malloc(sizeof(linked_list_t), GFP_KERNEL);
    void *node_data = malloc(sizeof(proc_data_t), GFP_KERNEL);
#endif
	
    if ((!data) || (!node) || (!node_data)) {
        return EFAIL;
    }

#ifndef USER_MODULE
    copy_from_user(node_data, data, sizeof(proc_data_t));
#else
    memcpy(node_data, data, sizeof(proc_data_t));
#endif
    node->data = node_data;
    node->next = node->prev = NULL;

    if (!tail) {
        head = tail = node;
    } else {
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
    return EOK;
}

linked_list_t *
proc_dequeue (void)
{
    linked_list_t *node = NULL;

    if (head) {
        node = head;
        head = head->next;
        if (head) {
            head->prev = NULL;
        } else {
            tail = NULL;
        }
    }

    return node;
}

void
timer_callback (unsigned long dummy)
{
    linked_list_t *node;	
    unsigned int slice = 0;

    while (1) {
        if ((slice++ >= SLICE_SIZE) || (!(node = proc_dequeue()))) {
            break;
        }

        if (proc_process_and_add_node_to_radix_tree(node->data) != EOK) {
            printk("Process node failed %p\n", node);
        }
#ifndef USER_MODULE
        kfree(node->data);
        kfree(node);
#else
        free(node->data);
        free(node);
#endif
    }
}

//******* Kernel module for timer *************
#if 0
#ifdef KERNEL_MODE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#endif


static struct timer_list proc_timer;

int init_module(void)
{
	// setup the timer
	setup_timer(&proc_timer, proc_timer_callback, 0);

	// setup timer interval to 2 sec
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
	return 0;
}

void cleanup_module(void)
{
	// remove kernel timer when unloading the module
	del_timer(&proc_timer);
	return;
}
#endif
