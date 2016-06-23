#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "proc_common.h"
#include "proc_radix_tree.h"

radix_node_t *root = NULL;

int main(int argc, char **argv)
{
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

    root = radix_tree_init();

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

