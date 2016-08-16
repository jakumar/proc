#ifndef PROC_RADIX_TREE
#define PROC_RADIX_TREE

#define NODE_TYPE_NODE      0
#define NODE_TYPE_LEAF      1
#define NODE_TYPE_ROOT      2

typedef struct radix_node_ {
    char *key;
    void *val;
    unsigned int pos;
    unsigned int type;
    char keystore[MAX_KEYSIZE];
    unsigned int level;
    unsigned int pid;
    struct radix_node_ *parent;
    struct radix_node_ *left;
    struct radix_node_ *right;
} radix_node_t;

void *proc_radix_retrieve(char *key, void *leaf, radix_node_t *root);
unsigned int proc_radix_insert(char *key, void *data, radix_node_t *rnode);
void proc_radix_inorder_traversal(radix_node_t *root);
radix_node_t * proc_radix_tree_init(void);

#endif // PROC_RADIX_TREE

