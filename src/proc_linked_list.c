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
#include "proc_linked_list.h"
#include "proc_ioctl_buf.h"

linked_list_t *head=NULL;
linked_list_t *tail=NULL;

#define SLICE_SIZE 10


unsigned int proc_enqueue(void *data)
{
#ifndef USER_MODULE
    linked_list_t *node = kmalloc(sizeof(linked_list_t), GFP_KERNEL);
#else
    linked_list_t *node = malloc(sizeof(linked_list_t), GFP_KERNEL);
#endif
	
    if (!node) {
        return EFAIL;
    }

    node->data = data;
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


linked_list_t *proc_dequeue(void)
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

void timer_callback(unsigned long dummy)
{
	linked_list_t *node;	
	unsigned int slice = 0;

	while (1) {
		if (slice++ >= SLICE_SIZE) 
			break;
		if (!(node = proc_dequeue())) 
			break;

        proc_ioctl_buf_handler(node->data); 
#ifndef USER_MODULE
		kfree(node);
#else
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
