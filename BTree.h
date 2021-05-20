#ifdef _cplusplus
#if _cplusplus
extern "C" {
#endif /* _BTREENODE_H */
#endif /* _BTREENODE_H */




#include "btreedef.h"
#include "LIST.h"

#ifndef _B_TREE_H
#define _B_TREE_H



typedef struct BTreeNode {
	off_t self;
	int n;
	KEY_TYPE key[2 * MINIMUM_DEGREE - 1];
	off_t children[2 * MINIMUM_DEGREE];
	BOOL leaf;
} BTreeNode;


typedef struct  free_block {
	LIST link;
	off_t offset;
} free_block;



typedef struct b_tree {
	int fd;
	char filename[128];
	int level;
	unsigned long size;
	off_t root;
	LIST free_blocks;
} b_tree;




void B_TREE_CREATE(b_tree *Tree, char *Filename);
void B_TREE_LOAD(b_tree *Tree, char *Filename);
void B_TREE_LOAD_CLOSE(b_tree *Tree);
off_t ALLOCATE_NODE(b_tree *Tree, BTreeNode *node);
BOOL B_TREE_SEARCH(b_tree *Tree, off_t node_offset, KEY_TYPE key);
void B_TREE_SPLIT_CHILD(b_tree *Tree, BTreeNode *x, int pivot);
void B_TREE_INSERT(b_tree *Tree, KEY_TYPE key);
void B_TREE_INSERT_NONFULL(b_tree *Tree, BTreeNode *x, KEY_TYPE key);



#endif /* _B_TREE_H */





#ifdef _cplusplus
#if _cplusplus
}
#endif /* _cplusplus */
#endif /* _cplusplus */
