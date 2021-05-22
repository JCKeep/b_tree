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
extern unsigned long Seed;

STATIC KEY_TYPE POINTER PREV_OF_CURRENT = NULL;
STATIC KEY_TYPE POINTER NEXT_OF_CURRENT = NULL;


off_t ALLOCATE_NODE(b_tree *Tree, BTreeNode *node)
{
	off_t offset;
	if (!ListEmpty(&(Tree->free_blocks))) {
		offset = LIST_ENTRY(Tree->free_blocks.next, free_block, link)->offset;
		LIST *tmp = Tree->free_blocks.next;
		ListDelete(Tree->free_blocks.next);
		free(LIST_ENTRY(tmp, free_block, link));
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
			//printf("\n\n%ld\n\n", *offset);
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

	for (i = x->n - 1; i >= pivot; i--)
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
			i = i - 1;
		i = i + 1;
		BTreeNode node;
		B_TREE_NODE_READ_DISK(Tree, &node, x->children[i]);
		if (n_read < 0) {
			perror(strerror(ENOENT));
			exit(-1);
		}
		if (node.n == 2 * MINIMUM_DEGREE - 1) {
			B_TREE_SPLIT_CHILD(Tree, x, i);
			if (key > x->key[i])
				i = i + 1;
			B_TREE_NODE_READ_DISK(Tree, &node, x->children[i]);
		}
		B_TREE_INSERT_NONFULL(Tree, &node, key);
	}
}


void B_TREE_DELETE(b_tree *Tree, KEY_TYPE key)
{
	BTreeNode r;
	n_read = B_TREE_NODE_READ_DISK(Tree, &r, Tree->root);
	if (n_read < 0) {
		exit(-1);
	}
	B_TREE_DELETE_OVER_MINDEGREE(Tree, &r, key);
}


VOID B_TREE_DELETE_OVER_MINDEGREE(b_tree *Tree, BTreeNode *x, KEY_TYPE key)
{
	int i = x->n - 1;
	while (i >= 0 && key < x->key[i])
		i = i - 1;
	if (key == x->key[i]) {
		if (x->leaf == TRUE) {
			int j;
			for (j = i; j < x->n - 1; j++)
				x->key[j] = x->key[j + 1];
			x->n = x->n - 1;
			Tree->size = Tree->size - 1;
			B_TREE_NODE_WRITE_DISK(Tree, x);
			return;
		}
		BTreeNode left;
		B_TREE_NODE_READ_DISK(Tree, &left, x->children[i]);
		if (left.n >= MINIMUM_DEGREE) {
			B_TREE_DELETE_MAXIMUM(Tree, &left);
			if (PREV_OF_CURRENT != NULL) {
				x->key[i] = *PREV_OF_CURRENT;
				free(PREV_OF_CURRENT);
				PREV_OF_CURRENT = NULL;
				B_TREE_NODE_WRITE_DISK(Tree, x);
			}
		}
		else {
			BTreeNode right;
			B_TREE_NODE_READ_DISK(Tree, &right, x->children[i + 1]);
			if (right.n >= MINIMUM_DEGREE) {
                B_TREE_DELETE_MINIMUM(Tree, &right);
                if (NEXT_OF_CURRENT != NULL) {
                    x->key[i] = *NEXT_OF_CURRENT;
                    free(NEXT_OF_CURRENT);
                    NEXT_OF_CURRENT = NULL;
					B_TREE_NODE_WRITE_DISK(Tree, x);
                }
            }
			else {
				B_TREE_MERGE_CHILD(Tree, x, i);
				if (x->n == 0) {
					B_TREE_NODE_READ_DISK(Tree, x, x->children[0]);
				}
				B_TREE_DELETE_OVER_MINDEGREE(Tree, x, key);
			}

		}
	}
	else {
		i = i + 1;
		BTreeNode toDelete;
		B_TREE_NODE_READ_DISK(Tree, &toDelete, x->children[i]);
		if (toDelete.n >= MINIMUM_DEGREE)
			B_TREE_DELETE_OVER_MINDEGREE(Tree, &toDelete, key);
		else if (toDelete.n == MINIMUM_DEGREE - 1) {
			int brotherIdx = i;
			if (i == x->n) brotherIdx--;
			else	brotherIdx++;
			BTreeNode brother;
			B_TREE_NODE_READ_DISK(Tree, &brother, x->children[brotherIdx]);
			if (x->n > i) {
				if (brother.n >= MINIMUM_DEGREE) {
					B_TREE_TRANSPLANT(Tree, x, &toDelete, &brother, i);
					B_TREE_DELETE_OVER_MINDEGREE(Tree, &toDelete, key);
				}
				else {
					B_TREE_MERGE_CHILD(Tree, x, i);
					B_TREE_NODE_READ_DISK(Tree, &toDelete, x->children[i]);
					B_TREE_DELETE_OVER_MINDEGREE(Tree, &toDelete, key);
				}
			}
			else {
				if (brother.n >= MINIMUM_DEGREE) {
					B_TREE_TRANSPLANT2(Tree, x, &brother, &toDelete, i);
					B_TREE_DELETE_OVER_MINDEGREE(Tree, &toDelete, key);
				}
				else {
					B_TREE_MERGE_CHILD(Tree, x, i - 1);
					B_TREE_NODE_READ_DISK(Tree, &toDelete, i - 1);
					B_TREE_DELETE_OVER_MINDEGREE(Tree, &toDelete, key);
				}
			}
		}
	}

}


VOID B_TREE_MERGE_CHILD(b_tree *Tree, BTreeNode *x, int pivot)
{
	BTreeNode left;
	B_TREE_NODE_READ_DISK(Tree, &left, x->children[pivot]);
	BTreeNode right;
	B_TREE_NODE_READ_DISK(Tree, &right, x->children[pivot + 1]);
	B_TREE_FREE_NODE(Tree, &left);
	B_TREE_FREE_NODE(Tree, &right);

	BTreeNode y;
	ALLOCATE_NODE(Tree, &y);
	y.leaf = left.leaf;
	y.n = left.n + right.n + 1;

	int i;
	for (i = 0; i < left.n; ++i)
		y.key[i] = left.key[i];

	y.key[i] = x->key[pivot];

	for (i = 0; i < right.n; ++i)
		y.key[i + left.n + 1] = right.key[i];
	

	if (left.leaf == TRUE) {
		for (i = 0; i <= left.n; ++i)
			y.children[i] = left.children[i];

		for (i = 0; i <= right.n; ++i)
			y.children[i + left.n + 1] = right.children[i];
	}

	for (i = pivot; i < x->n; ++i)
		x->key[i] = x->key[i + 1];
	for (i = pivot + 1; i <= x->n; ++i)
		x->children[i] = x->children[i + 1];
	x->children[pivot] = y.self;

	x->n = x->n - 1;
	if (x->n == 0 && x->self == Tree->root) {
		B_TREE_FREE_NODE(Tree, x);
		Tree->root = y.self;
		Tree->level = Tree->level - 1;
		B_TREE_NODE_WRITE_DISK(Tree, &y);
		B_TREE_WRITE_DISK(Tree);
	}
	else {
		B_TREE_NODE_WRITE_DISK(Tree, &y);
		B_TREE_NODE_WRITE_DISK(Tree, x);
	}
}


VOID B_TREE_DELETE_MAXIMUM(b_tree *Tree, BTreeNode *node)
{
	if (node->leaf == TRUE) {
		PREV_OF_CURRENT = (KEY_TYPE POINTER)malloc(sizeof(KEY_TYPE));
		*PREV_OF_CURRENT = node->key[node->n - 1];
		node->n = node->n - 1;
		Tree->size = Tree->size - 1;
		B_TREE_NODE_WRITE_DISK(Tree, node);
	}
	else {
		BTreeNode c;
		B_TREE_NODE_READ_DISK(Tree, &c, node->children[node->n]);
		if (c.n >= MINIMUM_DEGREE)
			B_TREE_DELETE_MAXIMUM(Tree, &c);
		else {
			BTreeNode r;
			B_TREE_NODE_READ_DISK(Tree, &r, node->children[node->n - 1]);
			if (r.n >= MINIMUM_DEGREE) {
				B_TREE_TRANSPLANT2(Tree, node, &c, &r, node->n);
				B_TREE_DELETE_MAXIMUM(Tree, &c);
			}
			else {
				B_TREE_MERGE_CHILD(Tree, node, node->n - 1);
				B_TREE_NODE_READ_DISK(Tree, &c, node->children[node->n]);
				B_TREE_DELETE_MINIMUM(Tree, &c);
			}
		}
	}
}


VOID B_TREE_DELETE_MINIMUM(b_tree *Tree, BTreeNode *node)
{
	if (node->leaf == TRUE) {
		NEXT_OF_CURRENT = (KEY_TYPE POINTER)malloc(sizeof(KEY_TYPE));
		*NEXT_OF_CURRENT = node->key[0];
		int i;
		for (i = 0; i < node->n; ++i)
			node->key[i] = node->key[i + 1];
		node->n = node->n - 1;
		Tree->size = Tree->size - 1;
		B_TREE_NODE_WRITE_DISK(Tree, node);
	}
	else {
		BTreeNode c;
		B_TREE_NODE_READ_DISK(Tree, &c, node->children[0]);
		if (c.n >= MINIMUM_DEGREE)
			B_TREE_DELETE_MINIMUM(Tree, &c);
		else {
			BTreeNode r;
			B_TREE_NODE_READ_DISK(Tree, &r, node->children[1]);
			if (r.n >= MINIMUM_DEGREE) {
				B_TREE_TRANSPLANT(Tree, node, &c, &r, 0);
				B_TREE_DELETE_MINIMUM(Tree, &c);
			}
			else {
				B_TREE_MERGE_CHILD(Tree, node, 0);
				B_TREE_NODE_READ_DISK(Tree, &c, node->children[0]);
				B_TREE_DELETE_MINIMUM(Tree, &c);
			}
		}
		
	}
}


VOID B_TREE_TRANSPLANT(b_tree *Tree, BTreeNode *x, BTreeNode *l, BTreeNode *r, int pivot)
{
	l->key[l->n] = x->key[pivot];
	l->n = l->n + 1;
	l->children[l->n] = r->children[0];
	x->key[pivot] = r->key[0];
	
	int i;
	for (i = 0; i < r->n; ++i) 
		r->key[i] = r->key[i + 1];
	
	if (l->leaf == FALSE) 
		for (i = 0; i < r->n; ++i)
			r->children[i] = r->children[i + 1];
	r->n = r->n - 1;
	B_TREE_NODE_WRITE_DISK(Tree, l);
	B_TREE_NODE_WRITE_DISK(Tree, r);
	B_TREE_NODE_WRITE_DISK(Tree, x);
}


VOID B_TREE_TRANSPLANT2(b_tree *Tree, BTreeNode *x, BTreeNode *l, BTreeNode *r, int pivot)
{
	int i;
	for (i = r->n - 1; i >= 0; i--)
		r->key[i + 1] = r->key[i];
	if (r->leaf == FALSE) {
		for (i = r->n; i >= 0; i--)
			r->children[i + 1] = r->children[i];
		r->children[0] = l->children[l->n];
	}
	r->key[0] = x->key[pivot - 1];
	x->key[pivot - 1] = l->key[l->n - 1];

	r->n = r->n + 1;
	l->n = l->n - 1;

	B_TREE_NODE_WRITE_DISK(Tree, l);
	B_TREE_NODE_WRITE_DISK(Tree, r);
	B_TREE_NODE_WRITE_DISK(Tree, x);
}

























