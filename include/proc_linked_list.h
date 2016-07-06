#ifndef PROC_LINKED_LIST
#define PROC_LINKED_LIST

typedef struct linked_list_ {
    void *data;
    struct linked_list_ *next;
    struct linked_list_ *prev;
} linked_list_t;

unsigned int proc_enqueue(void *data);
void timer_callback(unsigned long dummy);

#endif // PROC_LINKED_LIST
