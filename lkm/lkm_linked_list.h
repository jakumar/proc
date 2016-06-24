/**
 *
 * lkm_linked_list.h
 *
 * linked list to keep IOCTL information for procfs
 *
 */

typedef struct list_node_s {
    void *data;
    struct list_node_s *node_next;
    struct list_node_s *node_prev;
} list_node_t;

int list_insert_tail(void *data);

void* list_remove_head(void);
