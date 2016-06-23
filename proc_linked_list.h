#ifndef PROC_LINKED_LIST
#define PROC_LINKED_LIST

typedef struct linked_list_ {
	char key[RADIXTREE_KEYSIZE];
	void *data;
	struct linked_list_ *next;
	struct linked_list_ *prev;
} linked_list_t;

#endif // PROC_LINKED_LIST
