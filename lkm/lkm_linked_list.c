/**
 *
 * lkm_linked_list.c
 *
 * linked list to keep IOCTL information for procfs
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "lkm_proc_ioctl_kernel.h"
#include "lkm_linked_list.h"

#if 1
/* Linked list's head and tail */
static list_node_t *list_head = NULL, *list_tail = NULL;

int
list_insert_tail (void *data)
{
    void *tmp_data = NULL;
    list_node_t *tmp_node;

    /* Perform some sanity checks before proceeding */
    if (!data) {
        printk("Linked list add with NULL data\n");
        return -1;
    }

    /* Allocate memory for the new node */
    tmp_node = kmalloc(sizeof(list_node_t), GFP_KERNEL);
    if (!tmp_node) {
        printk("Memory allocation failure for list_insert (node)\n");
        return -1;
    }

    /* Allocate memory for the data in the new node */
    tmp_data = kmalloc(sizeof(proc_data_t), GFP_KERNEL);
    if (!tmp_data) {
        printk("Memory allocation failure for list_insert (data)\n");
        kfree(tmp_node);
        return -1;
    }

    /* Copy the contents of the data to the kernel memory */
    memcpy(tmp_data, data, sizeof(proc_data_t));

    tmp_node->data = tmp_data;

    tmp_node->node_next = NULL;

    if (!list_head) {
        list_head = list_tail = tmp_node;
        tmp_node->node_prev = NULL;
    } else {
        list_tail->node_next = tmp_node;
        tmp_node->node_prev = list_tail;
        list_tail = tmp_node;
    }

    printk("LL: head %p tail: %p new_node %p\n", list_head, list_tail, tmp_node);
    return 0;
}

void*
list_remove_head (void)
{
    void *data;
    list_node_t *tmp_node;

    if (!list_head) {
        printk("Linked list is empty\n");
        return NULL;
    }

    tmp_node = list_head;

    list_head = list_head->node_next;
    if (list_head == NULL) {
        list_tail = list_head;
    } else {
        list_head->node_prev = NULL;
    }

    data = tmp_node->data;

    printk("LL: head %p tail: %p free_node %p\n", list_head, list_tail, tmp_node);
    kfree(tmp_node);

    return data;
}
#endif
