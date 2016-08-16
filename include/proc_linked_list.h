#ifndef PROC_LINKED_LIST
#define PROC_LINKED_LIST

typedef struct linked_list_ {
    void *data;
    struct linked_list_ *next;
    struct linked_list_ *prev;
} linked_list_t;

linked_list_t * proc_dequeue (void);
unsigned int proc_enqueue(void *data);
void proc_timer_callback(unsigned long dummy);

#endif // PROC_LINKED_LIST
