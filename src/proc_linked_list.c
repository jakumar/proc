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
#include "proc_linked_list.h"
#include "libproc_api.h"
#include "proc_ioctl.h"
#include "proc_ioctl_buf.h"

linked_list_t *head = NULL;
linked_list_t *tail = NULL;

unsigned int
proc_enqueue (void *data)
{
    linked_list_t *node = kmalloc(sizeof(linked_list_t), GFP_KERNEL);
	
    if ((!node) || (!data)) {
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
