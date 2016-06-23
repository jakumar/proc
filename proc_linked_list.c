#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "proc_common.h"
#include "proc_linked_list.h"
#include "proc_radix_tree.h"

linked_list_t *head=NULL;
linked_list_t *tail=NULL;

#define SLICE_SIZE 10

unsigned int proc_enqueue(char *key, void *data)
{
	linked_list_t *node = malloc(sizeof(linked_list_t));
	
	if (!node) {
		return EFAIL;
	}

	strncpy(node->key, key, RADIXTREE_KEYSIZE);
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
	linked_list_t *node;

	if (head) {
		node = head;
		head->next->prev = head->next;
		head = head->next;
	} else {
		node = NULL;
	}

	return node;
}

extern radix_node_t *root;
void timer_callback(unsigned long dummy)
{
	linked_list_t *node;	
	void *data_object;
	unsigned int slice = 0;

	while (head) {
		if (slice++ >= SLICE_SIZE) 
			break;
		node = proc_dequeue();

		// put another while loop for all the chunks in 
		// data buffer
		// create chunks of data buffer into individual
		// resource data objects.
		data_object = node->data;

		radix_insert(node->key, data_object, root);
		free(node);
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
