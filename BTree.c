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
#include <stdio.h>

extern ssize_t n_read;
extern ssize_t n_write;
extern ssize_t erropn;


off_t ALLOCATE_NODE(b_tree *Tree, BTreeNode *node)
{
	off_t offset;
	if (!ListEmpty(&(Tree->free_blocks))) {
		offset = LIST_ENTRY(Tree->free_blocks.next, free_block, link)->offset;
		ListDelete(Tree->free_blocks.next);
	}
	else {
		offset = lseek(Tree->fd, 0, SEEK_END);
	}
	node->n = 0;
        node->leaf = FALSE;
	node->self = offset;
	B_TREE_NODE_WRITE_DISK(Tree, node);

	return offset;
}


void B_TREE_CREATE(b_tree *Tree, char *Filename)
{
	Tree->fd = open(Filename, O_RDWR | O_CREAT, 0644);
	strcpy(Tree->filename, Filename);
	LIST_MAKE_HEAD(&Tree->free_blocks);
	n_write = B_TREE_WRITE_DISK(Tree);
	if (n_write < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}

	BTreeNode root;
	root.self = ALLOCATE_NODE(Tree, &root);
	root.n = 0;
	root.leaf = TRUE;

	n_write = B_TREE_NODE_WRITE_DISK(Tree, &root);
	if (n_write < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}
	
	/* Tree->file_size = lseek(Tree->fd, 0, SEEK_END); */
	Tree->level = 0;
	Tree->size = 0;
	Tree->root = root.self;
	n_write = B_TREE_WRITE_DISK(Tree);
	if (n_write < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}
	
	close(Tree->fd);
}


void B_TREE_LOAD(b_tree *Tree, char *Filename)
{
	erropn = open(Filename, O_RDWR);
	if (erropn < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}
	else {
		Tree->fd = erropn;
		B_TREE_READ_DISK(Tree, Tree->fd);
		LIST_MAKE_HEAD(&Tree->free_blocks);
		char file_free_block[128];
		STRFREEBLOCKS(file_free_block, Tree->filename);
		erropn = open(file_free_block, O_RDWR);
		if (erropn < 0)
			return;
		off_t *offset;
		while ((n_read = read(erropn, offset, sizeof(off_t)) > 0)) {
			free_block *temp_block = (free_block *)malloc(sizeof(free_block));
			temp_block->offset = *offset;
			ListTailInsert(&Tree->free_blocks, &temp_block->link);
		}
	}
}


void B_TREE_LOAD_CLOSE(b_tree *Tree)
{
	char file_free_block[128];
	STRFREEBLOCKS(file_free_block, Tree->filename);
	erropn = open(file_free_block, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (erropn < 0)
		exit(-1);

	free_block *item, *next;
	LIST_FOR_EACH_ENTRY_SAFE(item, next, &(Tree)->free_blocks, free_block, link) {
		write(erropn, &item->offset, sizeof(off_t));
		free(item);
	}


	B_TREE_WRITE_DISK(Tree);
	close(Tree->fd);
}


BOOL B_TREE_SEARCH(b_tree *Tree, off_t node_offset, KEY_TYPE key)
{
	BTreeNode node;
	n_read = B_TREE_NODE_READ_DISK(Tree, &node, node_offset);
	if (n_read < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}

	int i = 0;
	while (i < node.n && key > node.key[i])
		i++;
	if (i < node.n && key == node.key[i])
		return TRUE;
	else if (node.leaf == TRUE)
		return FALSE;
	else
		return B_TREE_SEARCH(Tree, node.children[i], key);
}


void B_TREE_SPLIT_CHILD(b_tree *Tree, BTreeNode *x, int pivot)
{
	BTreeNode z;
	z.self = ALLOCATE_NODE(Tree, &z);
	BTreeNode y;
	B_TREE_NODE_READ_DISK(Tree, &y, x->children[pivot]);
	z.leaf = y.leaf;
	z.n = MINIMUM_DEGREE - 1;

	int i;
	for (i = 0; i < MINIMUM_DEGREE - 1; ++i)
		z.key[i] = y.key[i + MINIMUM_DEGREE];

	if (y.leaf != TRUE)
		for (i = 0; i < MINIMUM_DEGREE; ++i)
			z.children[i] = y.children[i + MINIMUM_DEGREE];
	
	y.n = MINIMUM_DEGREE - 1;
	for (i = x->n; i > pivot; i--)
		x->children[i + 1] = x->children[i];
	x->children[i + 1] = z.self;

	for (i = x->n - 1; i > pivot; i--)
		x->key[i + 1] = x->key[i];
	x->key[i + 1] = y.key[MINIMUM_DEGREE - 1];
	x->n = x->n + 1;
	

	B_TREE_NODE_WRITE_DISK(Tree, &y);
	B_TREE_NODE_WRITE_DISK(Tree, &z);
	B_TREE_NODE_WRITE_DISK(Tree, x);
}


void B_TREE_INSERT(b_tree *Tree, KEY_TYPE key)
{
	BTreeNode r;
	n_read = B_TREE_NODE_READ_DISK(Tree, &r, Tree->root);
	if (n_read < 0) {
		perror(strerror(ENOENT));
		exit(-1);
	}
	if (r.n == 2 * MINIMUM_DEGREE - 1) {
		BTreeNode newRoot;
		newRoot.self = ALLOCATE_NODE(Tree, &newRoot);
		newRoot.leaf = FALSE;
		newRoot.n = 0;
		newRoot.children[0] = Tree->root;
		Tree->root = newRoot.self;
		Tree->level++;
		B_TREE_SPLIT_CHILD(Tree, &newRoot, 0);
		B_TREE_INSERT_NONFULL(Tree, &newRoot, key);
	}
	else
		B_TREE_INSERT_NONFULL(Tree, &r, key);
	Tree->size++;
}


void B_TREE_INSERT_NONFULL(b_tree *Tree, BTreeNode *x, KEY_TYPE key)
{
	int i = x->n - 1;
	if (x->leaf == TRUE) {
		while (i >= 0 && key < x->key[i]){
			x->key[i + 1] = x->key[i];
			i--;
		}
		x->key[i + 1] = key;
		x->n = x->n + 1;
		n_write = B_TREE_NODE_WRITE_DISK(Tree, x);
		if (n_write < 0) {
			perror(strerror(ENOENT));
			exit(-1);
		}
	}
	else {
		while (i >= 0 && key < x->key[i])
			i--;
		i++;
		BTreeNode node;
		B_TREE_NODE_READ_DISK(Tree, &node, x->children[i]);
		if (n_read < 0) {
			perror(strerror(ENOENT));
			exit(-1);
		}
		if (node.n == 2 * MINIMUM_DEGREE - 1) {
			B_TREE_SPLIT_CHILD(Tree, x, i);
			if (key > x->key[i]){
				B_TREE_NODE_READ_DISK(Tree, &node, x->children[i + 1]);
				if (n_read < 0) {
					perror(strerror(ENOENT));
					exit(-1);
				}
			}
		}
		B_TREE_INSERT_NONFULL(Tree, &node, key);
	}
}
