#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "BTree.h"
#include "btreedef.h"
#include "Random.h"

int main(void)
{
	b_tree b, a;
	B_TREE_CREATE(&a, "file");
	B_TREE_LOAD(&b, "file");

	int k;
	scanf("%d", &k);
	int i;
	for (i = 1; i <= k; ++i) {
		B_TREE_INSERT(&b, RandomInt());
	}
	
	printf("------------------------------------------\n");
	printf("Filename:%s\tFiledes:%d\tRoot:%ld\n", b.filename, b.fd, b.root);
	printf("Level:%d\tSize:%ld\n", b.level, b.size);
	int tmp;
	scanf("%d", &tmp);
	RandomInitialize(1);
	for (i = 0; i < tmp; ++i) {
		int t = RandomInt();
		BOOL flag = B_TREE_SEARCH(&b, b.root, t);
		if (flag == TRUE) {
			printf("\033[1;42mtrue\033[0m\n");
			B_TREE_DELETE(&b, t);
		}
	}


	printf("------------------------------------------\n");
	printf("Filename:%s\tFiledes:%d\tRoot:%ld\n", b.filename, b.fd, b.root);
	printf("Level:%d\tSize:%ld\n", b.level, b.size);

	
	B_TREE_LOAD_CLOSE(&b);
	

	return 0;
}
