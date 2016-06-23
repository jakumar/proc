#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "proc_common.h"
#include "proc_radix_tree.h"

static inline int keylen(char *key)
{
    int len = strlen(key);

    if (len < 0 || len >= RADIXTREE_KEYSIZE)
        printf("Warning: rxt key (%d) exceeds limit (%d)\n",
                len, RADIXTREE_KEYSIZE);
    return ((BITS_IN_BYTE * (len + 1)) - 1);
}

#define RADIX_NEW_LEAF(pnode, leaf, key, data)          \
{                                                       \
    leaf = malloc(sizeof(radix_node_t));                \
    if (!leaf) return EFAIL;                            \
    strncpy(leaf->keystore, key, RADIXTREE_KEYSIZE);    \
    leaf->keystore[RADIXTREE_KEYSIZE - 1] = '\0';       \
    leaf->key = leaf->keystore;                         \
    leaf->pos = keylen(key);                            \
    leaf->val = data;                                   \
    leaf->parent = pnode;                               \
    leaf->type = NODE_TYPE_LEAF;                        \
    leaf->left = leaf->right = NULL;                    \
}

static unsigned int common_bits(char *k1, char *k2, unsigned int bits)
{
    unsigned int mask = 1;
    
    while ((~(*k1 ^ *k2) & mask) && --bits) {
        mask <<= 1;
        if (mask == 0x100) {
            // reinit values for next byte
            mask = 1;
            k1++;
            k2++;
	}
    }
    return bits;
}

static unsigned int common_bits_in_key(char *k1, char *k2, unsigned int max)
{
    unsigned int cnt = max;

    // walk down the key in chunks for 4 bytes (or int size)
    while ((*k1 == *k2) && (cnt >= BITS_IN_INT)) {
        int *i1 = (int *)k1;
        int *i2 = (int *)k2;
    
        if (*i1 == *i2) {
            k1 += SIZE_OF_INT;
            k2 += SIZE_OF_INT;
            cnt -= BITS_IN_INT;
        } else
            break;
    }

    // now handle the last chunk of 4 bytes (or int size)
    while ((*k1 == *k2) && (cnt >= BITS_IN_BYTE)) {
        k1++;
        k2++;
        cnt -= BITS_IN_BYTE;
    }
    return (max - common_bits(k1, k2, cnt));
}

static inline int radix_min(int i1, int i2)
{
    return i1 > i2 ? i2 : i1;
}

static inline unsigned int key_bit_value_at(char *key, int i)
{
    return (*(key + (i >> 3)) & (1 << (i & 7)));
}

static unsigned int radix_insert_leaf(radix_node_t *leaf, radix_node_t *sibling, 
                              radix_node_t *parent)
{
    int bit, bit_idx;
    radix_node_t *node;

    bit_idx = common_bits_in_key(leaf->key, sibling->key, 
                radix_min(leaf->pos, sibling->pos));
    bit = key_bit_value_at(leaf->key, bit_idx);

    if (!parent) {
        parent = sibling;
        node = malloc(sizeof(radix_node_t));
        if (!node)
            return EFAIL;
        
        node->type = NODE_TYPE_NODE;
        node->pos = parent->pos;
        node->key = parent->key;
        node->val = NULL;
        node->parent = parent;
        node->left = parent->left;
        node->right = parent->right;
        parent->pos = bit_idx;
        parent->left->parent = node;
        parent->right->parent = node;
        leaf->parent = parent;
        if (bit) {
            parent->right = leaf;
            parent->left = node;
			printf("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Inserting Sibling@Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printf("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
        } else {
            parent->right = node;
            parent->left = leaf;
			printf("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Inserting Sibling@Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printf("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
        }
        return EOK;
    }

    if (bit_idx < parent->pos) {
        // leaf need to above the parent
		printf("\nradix_insert_leaf(): Walking UP Pos:%d Parent Pos:%d",
			bit_idx, parent->pos);
		printf("\nradix_insert_leaf(): Walking UP Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
			leaf->type);
		printf("\nradix_insert_leaf(): Walking UP Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
			parent->type);
		if (parent->parent) {
			printf("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->parent->key, *(int *)(parent->parent->key + 4), 
				parent->parent->key, parent->pos, parent->parent->type);
		}
        return radix_insert_leaf(leaf, parent, parent->parent);
    } else {
        // add leaf is a as a child of the parent
        // check for duplicates
        if ((leaf->pos == sibling->pos) && 
            !strncmp(leaf->key, sibling->key, leaf->pos)) {
            // free leaf in the calling function
			printf("\nradix_insert_leaf(): ERROR: Duplicate Key.....");
            return EFAIL;
        }

        node = malloc(sizeof(radix_node_t));
        if (!node) 
            return EFAIL;

        node->type = NODE_TYPE_NODE;
        node->val = NULL;
        node->parent = parent;
        node->pos = bit_idx;
        node->key = sibling->key;
        leaf->parent = node;
        sibling->parent = node;

        if (bit) {
            node->right = leaf;
            node->left = sibling;
			printf("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Adjusting Sibling @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
        } else {
            node->right = sibling;
            node->left = leaf;
			printf("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Adjusting Sibling @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
        }

        if (parent->left == sibling) {
            parent->left = node;
			printf("\nradix_insert_leaf(): Inserting Parent @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printf("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
        } else if (parent->right == sibling) {
            parent->right = node;
			printf("\nradix_insert_leaf(): Inserting Parent @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printf("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
        } else {
			printf("\nradix_insert_leaf(): ERROR.....");
            return EFAIL;
        }
    }

    return EOK;
}

static unsigned int radix_insert_common(radix_node_t *leaf, radix_node_t *rnode)
{
    int lbits = common_bits_in_key(leaf->key, rnode->left->key, 
                    radix_min(leaf->pos, rnode->left->pos));
    int rbits = common_bits_in_key(leaf->key, rnode->right->key, 
                    radix_min(leaf->pos, rnode->right->pos));

    // its a leaf for sure
    if (radix_min(lbits, rbits) < rnode->pos) {
        if (lbits >= rbits) {
            // insert left leaf
	    	printf("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
            return radix_insert_leaf(leaf, rnode->left, rnode);
        }

        //insert right leaf
	   	printf("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
        return radix_insert_leaf(leaf, rnode->right, rnode);
    }

    if (lbits >= rbits) {
        if (rnode->left->type == NODE_TYPE_LEAF) {
            // found the leaf 
	   		printf("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
            return radix_insert_leaf(leaf, rnode->left, rnode);
        }
	    printf("\nWalking Left - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->left->key, *(int *)(rnode->left->key + 4), rnode->left->key,
			rnode->left->pos, rnode->left->type);
        return radix_insert_common(leaf, rnode->left);
    } else {
        if (rnode->right->type == NODE_TYPE_LEAF) {
            // found the leaf
	   		printf("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
            return radix_insert_leaf(leaf, rnode->right, rnode);
        }
	    printf("\nWalking Right - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->right->key, *(int *)(rnode->right->key + 4), rnode->right->key,
			rnode->right->pos, rnode->right->type);
        return radix_insert_common(leaf, rnode->right);
    }

    return EFAIL;
}

unsigned int radix_insert(char *key, void *data, radix_node_t *rnode)
{
    radix_node_t *leaf;

    RADIX_NEW_LEAF(rnode, leaf, key, data);
    if (!(rnode->left || rnode->right)) {
        radix_node_t *sibling;
        unsigned int bits;

        if (!rnode->val) {
            // Empty root
            rnode->val = leaf;
            rnode->type = NODE_TYPE_ROOT;
			printf("\nradix_insert(): Initializing Root - Type:%d", rnode->type);
			printf("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos, leaf->type);
            return EOK;
        }

        // convert root to node and attach leaf
        sibling = rnode->val;
        bits = common_bits_in_key(key, sibling->key,
                radix_min(leaf->pos, sibling->pos));
		printf("\nradix_insert(): Common Bits:%d", bits);
        if (key_bit_value_at(key, bits)) {
            // add new leaf at right
            rnode->right = leaf;
            rnode->left = sibling;
			printf("\nradix_insert(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert(): Adjusting @Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
        } else {
            rnode->left = leaf;
            rnode->right = sibling;
			printf("\nradix_insert(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert(): Adjusting @Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
        }

        rnode->val = NULL;
        rnode->key = key;
        rnode->pos = bits;
        rnode->type = NODE_TYPE_NODE;
		printf("\nradix_insert(): Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key,
			 rnode->pos, rnode->type);
        return EOK;
    }

    // don't know the parent
    leaf->parent = NULL;
	printf("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
			leaf->type);
	printf("\nradix_insert(): Root Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos,
			rnode->type);
    return radix_insert_common(leaf, rnode);
}

static radix_node_t* radix_lookup(char *key, radix_node_t *root)
{
    if (!root) {
		return NULL;
	}

    if (root->type) {
        if (root->type == NODE_TYPE_ROOT) {
			root = root->val;
		}
        if (!strncmp(key, root->key, root->pos))
            return root;
        return NULL;
    }

    if (key_bit_value_at(key, root->pos)) {
        return radix_lookup(key, root->right);
	}
    return radix_lookup(key, root->left);
}

void *radix_retrieve(char *key, radix_node_t *root)
{
	radix_node_t *rnode = radix_lookup(key, root);

	if (!rnode) {
		return NULL;
	}
	return rnode->val;
}

void radix_inorder_traversal(radix_node_t *root)
{
    if (!root) {
		return;
	}

    if (root->type) {
        if (root->type == NODE_TYPE_ROOT) {
			root = root->val;
		}
        printf("\nradix_inorder_traversal():Key[%s] Value[%s] Type[%d]", root->key, (char*)root->val, root->type);
        return;
    }
    radix_inorder_traversal(root->left);
    radix_inorder_traversal(root->right);
}

radix_node_t * radix_tree_init()
{
    radix_node_t *root = malloc(sizeof(radix_node_t));

    if (!root) 
        return NULL;

    memset(root, 0, sizeof(radix_node_t));
    return root;
}

static void radix_free_node(radix_node_t *rnode)
{
    if (!rnode)
        return;

    radix_free_node(rnode->left);
    radix_free_node(rnode->right);

    if (rnode->val) {
		// free the data stored
        free(rnode->val);
	}
    rnode->left =
    rnode->right =
    rnode->val = NULL;

    free(rnode);
}


#if 0
int main(int argc, char **argv)
{
    radix_node_t *root = radix_tree_init();
	char *key1 = "Romane";
	char *data1 = "enamoR";
	char *key2 = "Romanus";
	char *data2 = "sunamoR";
	char *key3 = "Romanfu";
	char *data3 = "ufnamoR";
	char *key4 = "Roming";
	char *data4 = "gnimoR";
	char *key5 = "Romanea";
	char *data5 = "aenamoR";
	char *key6 = "Romings";
	char *data6 = "sgnimoR";

	if (radix_insert(key1, data1, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key1);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key1, *(int *)(key1 + 4), key1);
		printf("\n\n");
	}

	if (radix_insert(key2, data2, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key2);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key2, *(int *)(key2 + 4), key2);
		printf("\n\n");
	}

	if (radix_insert(key3, data3, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key3);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key3, *(int *)(key3 + 4), key3);
		printf("\n\n");
	}

	if (radix_insert(key4, data4, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key4);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key4, *(int *)(key4 + 4), key4);
		printf("\n\n");
	}

	if (radix_insert(key5, data5, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key5);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key5, *(int *)(key5 + 4), key5);
		printf("\n\n");
	}

	if (radix_insert(key6, data6, root) == EFAIL) {
		printf("\nInsert Failed:[%s]...", key6);
	} else {
		printf("\nInsert Success:0x%x 0x%x [%s]...", *(int *)key6, *(int *)(key6 + 4), key6);
		printf("\n\n");
	}

	printf("\n\nRetrieval for Key:%s is Value:%s", key3, radix_retrieve(key3, root));
	printf("\n\nRetrieval for Key:%s is Value:%s", key5, radix_retrieve(key5, root));
	printf("\n\nRetrieval for Key:%s is Value:%s", key1, radix_retrieve(key1, root));
	printf("\n\nRetrieval for Key:%s is Value:%s", key2, radix_retrieve(key2, root));
	printf("\n\nRetrieval for Key:%s is Value:%s", key4, radix_retrieve(key4, root));

	printf("\n\nThe Patricia Tree....");
	radix_inorder_traversal(root);

	return 0;
}
#endif
