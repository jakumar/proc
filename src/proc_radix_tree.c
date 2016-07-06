#ifndef USER_MODULE

#include <linux/module.h>	
#include <linux/kernel.h>	
#include <linux/proc_fs.h>	
#include <asm/uaccess.h>

#else

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#endif

#include "proc_common.h"
#include "proc_radix_tree.h"

static inline int keylen(char *key)
{
    int len = strlen(key);

    if (len < 0 || len >= MAX_KEYSIZE)
#ifndef USER_MODULE
        printk("Warning: rxt key (%d) exceeds limit (%d)\n",
                len, MAX_KEYSIZE);
#else
        printf("Warning: rxt key (%d) exceeds limit (%d)\n",
                len, MAX_KEYSIZE);
#endif
    return ((BITS_IN_BYTE * (len + 1)) - 1);
}

static radix_node_t *
proc_radix_alloc_leaf(radix_node_t *pnode, 
                      char *key, 
                      void *data)
{
    radix_node_t *leaf;

#ifndef USER_MODULE
    leaf = (radix_node_t *)kmalloc(sizeof(radix_node_t), GFP_KERNEL);   
#else
    leaf = malloc(sizeof(radix_node_t)); 
#endif

    if (!leaf) {
        return NULL;
    }

    strncpy(leaf->keystore, key, MAX_KEYSIZE);
    leaf->keystore[MAX_KEYSIZE - 1] = '\0';
    leaf->key = leaf->keystore;
    leaf->pos = keylen(key);
    leaf->val = data;
    leaf->parent = pnode;
    leaf->type = NODE_TYPE_LEAF;
    leaf->left = leaf->right = NULL;

    return leaf;
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

static unsigned int radix_insert_leaf(radix_node_t *leaf, radix_node_t *sibling, radix_node_t *parent)
{
    int bit, bit_idx;
    radix_node_t *node;

    bit_idx = common_bits_in_key(leaf->key, sibling->key, 
                radix_min(leaf->pos, sibling->pos));
    bit = key_bit_value_at(leaf->key, bit_idx);

    if (!parent) {
        parent = sibling;
#ifndef USER_MODULE
        node = kmalloc(sizeof(radix_node_t), GFP_KERNEL);
#else
        node = malloc(sizeof(radix_node_t));
#endif
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
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert_leaf(): Inserting Sibling@Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printk("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#else
			printf("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Inserting Sibling@Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printf("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#endif
#endif
        } else {
            parent->right = node;
            parent->left = leaf;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert_leaf(): Inserting Sibling@Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printk("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#else
			printf("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Inserting Sibling@Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos, node->type);
			printf("\nradix_insert_leaf(): Sibling as Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#endif
#endif
        }
        return EOK;
    }

    if (bit_idx < parent->pos) {
        // leaf need to above the parent
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
		printk("\nradix_insert_leaf(): Walking UP Pos:%d Parent Pos:%d",
			bit_idx, parent->pos);
		printk("\nradix_insert_leaf(): Walking UP Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
			leaf->type);
		printk("\nradix_insert_leaf(): Walking UP Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
			parent->type);
		if (parent->parent) {
			printk("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->parent->key, *(int *)(parent->parent->key + 4), 
				parent->parent->key, parent->pos, parent->parent->type);
		}
#else
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
#endif
#endif
        return radix_insert_leaf(leaf, parent, parent->parent);
    } else {
        // add leaf is a as a child of the parent
        // check for duplicates
        if ((leaf->pos == sibling->pos) && 
            !strncmp(leaf->key, sibling->key, leaf->pos)) {
            // free leaf in the calling function
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): ERROR: Duplicate Key.....");
#else
			printf("\nradix_insert_leaf(): ERROR: Duplicate Key.....");
#endif
            return EFAIL;
        }

#ifndef USER_MODULE
        node = kmalloc(sizeof(radix_node_t), GFP_KERNEL);
#else
        node = malloc(sizeof(radix_node_t));
#endif
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
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert_leaf(): Adjusting Sibling @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
#else
			printf("\nradix_insert_leaf(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Adjusting Sibling @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
#endif
#endif
        } else {
            node->right = sibling;
            node->left = leaf;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert_leaf(): Adjusting Sibling @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
#else
			printf("\nradix_insert_leaf(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert_leaf(): Adjusting Sibling @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key, sibling->pos,
				sibling->type);
#endif
#endif
        }

        if (parent->left == sibling) {
            parent->left = node;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Parent @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printk("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#else
			printf("\nradix_insert_leaf(): Inserting Parent @Left Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printf("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#endif
#endif
        } else if (parent->right == sibling) {
            parent->right = node;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): Inserting Parent @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printk("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#else
			printf("\nradix_insert_leaf(): Inserting Parent @Right Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)node->key, *(int *)(node->key + 4), node->key, node->pos,
				node->type);
			printf("\nradix_insert_leaf(): Grant Parent Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)parent->key, *(int *)(parent->key + 4), parent->key, parent->pos,
				parent->type);
#endif
#endif
        } else {
#ifndef USER_MODULE
			printk("\nradix_insert_leaf(): ERROR.....");
#else
			printf("\nradix_insert_leaf(): ERROR.....");
#endif
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
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	    	printk("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#else
	    	printf("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#endif
#endif
            return radix_insert_leaf(leaf, rnode->left, rnode);
        }

        //insert right leaf
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	   	printk("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#else
	   	printf("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#endif
#endif
        return radix_insert_leaf(leaf, rnode->right, rnode);
    }

    if (lbits >= rbits) {
        if (rnode->left->type == NODE_TYPE_LEAF) {
            // found the leaf 
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	   		printk("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#else
	   		printf("\nradix_insert_common(): Insert Left of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#endif
#endif
            return radix_insert_leaf(leaf, rnode->left, rnode);
        }
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	    printk("\nWalking Left - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->left->key, *(int *)(rnode->left->key + 4), rnode->left->key,
			rnode->left->pos, rnode->left->type);
#else
	    printf("\nWalking Left - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->left->key, *(int *)(rnode->left->key + 4), rnode->left->key,
			rnode->left->pos, rnode->left->type);
#endif
#endif
        return radix_insert_common(leaf, rnode->left);
    } else {
        if (rnode->right->type == NODE_TYPE_LEAF) {
            // found the leaf
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	   		printk("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#else
	   		printk("\nradix_insert_common(): Insert Right of Node Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos, rnode->type);
#endif
#endif
            return radix_insert_leaf(leaf, rnode->right, rnode);
        }
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	    printk("\nWalking Right - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->right->key, *(int *)(rnode->right->key + 4), rnode->right->key,
			rnode->right->pos, rnode->right->type);
#else
	    printf("\nWalking Right - Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)rnode->right->key, *(int *)(rnode->right->key + 4), rnode->right->key,
			rnode->right->pos, rnode->right->type);
#endif
#endif
        return radix_insert_common(leaf, rnode->right);
    }

    return EFAIL;
}

unsigned int proc_radix_insert(char *key, void *data, radix_node_t *rnode)
{
    radix_node_t *leaf;

    leaf = proc_radix_alloc_leaf(rnode, key, data);
    if (!(rnode->left || rnode->right)) {
        radix_node_t *sibling;
        unsigned int bits;

        if (!rnode->val) {
            // Empty root
            rnode->val = leaf;
            rnode->type = NODE_TYPE_ROOT;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert(): Initializing Root - Type:%d", rnode->type);
			printk("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos, leaf->type);
#else
			printf("\nradix_insert(): Initializing Root - Type:%d", rnode->type);
			printf("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos, leaf->type);
#endif
#endif
            return EOK;
        }

        // convert root to node and attach leaf
        sibling = rnode->val;
        bits = common_bits_in_key(key, sibling->key,
                radix_min(leaf->pos, sibling->pos));
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
		printk("\nradix_insert(): Common Bits:%d", bits);
#else
		printf("\nradix_insert(): Common Bits:%d", bits);
#endif
#endif
        if (key_bit_value_at(key, bits)) {
            // add new leaf at right
            rnode->right = leaf;
            rnode->left = sibling;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert(): Adjusting @Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
#else
			printf("\nradix_insert(): Inserting Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert(): Adjusting @Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
#endif
#endif
        } else {
            rnode->left = leaf;
            rnode->right = sibling;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
			printk("\nradix_insert(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printk("\nradix_insert(): Adjusting @Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
#else
			printf("\nradix_insert(): Inserting Left Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
				leaf->type);
			printf("\nradix_insert(): Adjusting @Right Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
				*(int *)sibling->key, *(int *)(sibling->key + 4), sibling->key,
				 sibling->pos, sibling->type);
#endif
#endif
        }

        rnode->val = NULL;
        rnode->key = key;
        rnode->pos = bits;
        rnode->type = NODE_TYPE_NODE;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
		printk("\nradix_insert(): Parent[0x%x] Key[0x%x 0x%x:%s] Pos:%d Type:%d", (int)rnode,
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key,
			 rnode->pos, rnode->type);
#else
		printf("\nradix_insert(): Parent[0x%x] Key[0x%x 0x%x:%s] Pos:%d Type:%d", (int)rnode,
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key,
			 rnode->pos, rnode->type);
#endif
#endif
        return EOK;
    }

    // don't know the parent
    leaf->parent = NULL;
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
	printk("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
			leaf->type);
	printk("\nradix_insert(): Root Node[0x%x] Key[0x%x 0x%x:%s] Pos:%d Type:%d", (int)rnode,
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos,
			rnode->type);
#else
	printf("\nradix_insert(): Inserting Leaf Key[0x%x 0x%x:%s] Pos:%d Type:%d",
			*(int *)leaf->key, *(int *)(leaf->key + 4), leaf->key, leaf->pos,
			leaf->type);
	printf("\nradix_insert(): Root Node[0x%x] Key[0x%x 0x%x:%s] Pos:%d Type:%d", (int)rnode,
			*(int *)rnode->key, *(int *)(rnode->key + 4), rnode->key, rnode->pos,
			rnode->type);
#endif
#endif
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

void *proc_radix_retrieve(char *key, radix_node_t *root)
{
	radix_node_t *rnode = radix_lookup(key, root);

	if (!rnode) {
#ifndef USER_MODULE
        printk("\nproc_radix_retrieve(): NULL NODE !!!!!!");
#else
        printf("\nproc_radix_retrieve(): NULL NODE !!!!!!");
#endif
		return NULL;
	}
	return rnode->val;
}

void proc_radix_inorder_traversal(radix_node_t *root)
{
    if (!root) {
		return;
	}

    if (root->type) {
        if (root->type == NODE_TYPE_ROOT) {
			root = root->val;
		}
#ifdef DEBUG_FLAG
#ifndef USER_MODULE
        printk("\nradix_inorder_traversal():Key[%s] Type[%d]", root->key, root->type);
#else
        printf("\nradix_inorder_traversal():Key[%s] Type[%d]", root->key, root->type);
#endif
#endif
        return;
    }
    proc_radix_inorder_traversal(root->left);
    proc_radix_inorder_traversal(root->right);
}

radix_node_t * proc_radix_tree_init()
{
#ifndef USER_MODULE
    radix_node_t *root = kmalloc(sizeof(radix_node_t), GFP_KERNEL);
#else
    radix_node_t *root = malloc(sizeof(radix_node_t));
#endif

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
#ifndef USER_MODULE
        kfree(rnode->val);
#else
        free(rnode->val);
#endif
	}
    rnode->left =
    rnode->right =
    rnode->val = NULL;

#ifndef USER_MODULE
    kfree(rnode);
#else
    free(rnode);
#endif
}
