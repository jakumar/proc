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
    char keystore[RADIXTREE_KEYSIZE];
    unsigned int level;
    unsigned int pid;
    struct radix_node_ *parent;
    struct radix_node_ *left;
    struct radix_node_ *right;
} radix_node_t;

void *radix_retrieve(char *key, radix_node_t *root);
unsigned int radix_insert(char *key, void *data, radix_node_t *rnode);
void radix_inorder_traversal(radix_node_t *root);
radix_node_t * radix_tree_init();

#endif // PROC_RADIX_TREE

