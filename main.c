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
#include "Random.h"

int main()
{
	b_tree a;
	
	
	//B_TREE_CREATE(&a, "file");


	b_tree b;
	
	B_TREE_LOAD(&b, "file");
	
	

	//printf("filename:%s\tfd:%d\troot:%ld\n", a.filename, a.fd, a.root);
	printf("filename:%s\tfd:%d\troot:%ld\n", b.filename, b.fd, b.root);
	
	int i, k, InitVal;
	scanf("%d", &k);
	scanf("%d", &InitVal);
	RandomInitialize(InitVal);
	int * c = (int *)malloc(k * sizeof(int));
	for (i = 0; i < k; ++i)
		c[i] = RandomInt();
	
	for (i = 0; i < k; ++i) {
		B_TREE_INSERT(&b, c[i]);
	}
	//B_TREE_SEARCH(&b, b.root, c[100]);
	for (i = 0; i < k; ++i) {
		BOOL flag = B_TREE_SEARCH(&b, b.root, c[i]);
		if (flag == TRUE)
			printf("\033[1;42m%d\033[0m\n", c[i]);
		else
			printf("\033[1;41m%d\033[0m\n", c[i]);
	}
	
	/*BTreeNode root;
	B_TREE_NODE_READ_DISK(&b, &root, b.root);
	
	BTreeNode rootC1;
	B_TREE_NODE_READ_DISK(&b, &rootC1, root.children[0]);
	
	BTreeNode rootC2;
	B_TREE_NODE_READ_DISK(&b, &rootC2, root.children[1]);
	
	BTreeNode rootC3;
	B_TREE_NODE_READ_DISK(&b, &rootC3, root.children[2]);
	
	for (i = 0; i < root.n; ++i)
		printf("%d ", root.key[i]);
	printf("\n");
	for (i = 0; i < rootC1.n; ++i)
		printf("%d ", rootC1.key[i]);
	printf("\n");
	for (i = 0; i < rootC2.n; ++i)
		printf("%d ", rootC2.key[i]);
	printf("\n");
	for (i = 0; i < rootC3.n; ++i)
		printf("%d ", rootC3.key[i]);
	printf("\n");*/
	/*for (i = 0; i <= root.n + 1; ++i) {
		BTreeNode rootC2;
		B_TREE_NODE_READ_DISK(&b, &rootC2, root.children[i]);
		printf("\n");
		for (i = 0; i < rootC2.n; ++i)
			printf("%d ", rootC2.key[i]);
		printf("\n");
	}*/
		
	
	/*for (i = 0; i < 10; ++i) {
		int tmp;
		scanf("%d", &tmp);
		BOOL flag = B_TREE_SEARCH(&b, b.root, tmp);
		if (flag == TRUE)
			printf("\033[1;42mtrue\033[0m\n");
		else
			printf("\033[1;41mfalse\033[0m\n");
	}*/
	
	
	
	/*for (i = 0; i < 10; ++i) {
		int tmp;
		scanf("%d", &tmp);
		BOOL flag = B_TREE_SEARCH(&b, b.root, tmp);
		if (flag == TRUE)
			printf("\033[1;42mtrue\033[0m\n");
		else
			printf("\033[1;41mfalse\033[0m\n");
	}*/
	
	B_TREE_LOAD_CLOSE(&b);
	free(c);
	
	close(b.fd);
	

	return 0;
}
