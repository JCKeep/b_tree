#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "BTree.h"
#include "btreedef.h"

int main()
{
	b_tree a;
	char file[20];
	
	scanf("%s", file);
	
	B_TREE_CREATE(&a, file);


	b_tree b;
	
	B_TREE_LOAD(&b, a.filename);
	

	printf("filename:%s\tfd:%d\troot:%ld\n", a.filename, a.fd, a.root);
	printf("filename:%s\tfd:%d\troot:%ld\n", b.filename, b.fd, b.root);
	
	BTreeNode root;
	B_TREE_NODE_READ_DISK(&b, &root, b.root);
	
	printf("root: %ld\tkey: %d\n", b.root, root.key[0]);
	
	int i;
	for (i = 0; i < 10; ++i) {
		int tmp;
		scanf("%d", &tmp);
		B_TREE_INSERT(&a, tmp);
	}
	
	for (i = 0; i < 10; ++i) {
		int tmp;
		scanf("%d", &tmp);
		BOOL flag = B_TREE_SEARCH(&a, a.root, tmp);
		if (flag == TRUE)
			printf("\033[1;42mtrue\033[0m\n");
		else
			printf("\033[1;41mfalse\033[0m\n");
	}
	
	
	
	for (i = 0; i < 10; ++i) {
		int tmp;
		scanf("%d", &tmp);
		BOOL flag = B_TREE_SEARCH(&b, b.root, tmp);
		if (flag == TRUE)
			printf("\033[1;42mtrue\033[0m\n");
		else
			printf("\033[1;41mfalse\033[0m\n");
	}
	
	B_TREE_LOAD_CLOSE(&b);
	
	close(b.fd);
	

	return 0;
}
