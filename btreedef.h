#ifdef _cplusplus
#if _cplusplus
extern "C" {
#endif /* _cplusplus */
#endif /* _cplusplus */




#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <linux/kernel.h>


static ssize_t n_read;
static ssize_t n_write;
static ssize_t erropn;


typedef off_t b_tree_node;



#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

#ifndef BOOL
#define BOOL int
#endif /* BOOL */


#ifndef MINIMUM_DEGREE
#define MINIMUM_DEGREE 8
#endif /* MINIMUM_DEGREE */

#ifndef CHAR
#define CHAR char
#endif /* CHAR */


#ifndef KEY_TYPE
#define KEY_TYPE int
#endif /* KEY_TYPE */


#ifndef TRUE
#define TRUE 1
#endif /* TRUE */


#ifndef FALSE
#define FALSE 0
#endif /* FALSE */



#ifndef STATIC
#define STATIC static
#endif /* STATIC */


#ifndef INLINE
#define INLINE inline
#endif /* INLINE */

#ifndef POINTER
#define POINTER *
#endif /* POINTER */


#ifndef VOID
#define VOID void
#endif /* VOID */

#ifndef VOID_POINTER
#define VOID_POINTER (void *)
#endif /* VOID_POINTER */



#ifndef STRING_FUNCTIONS_H
#define STRING_FUNCTIONS_H



static inline char * STRCPY(char * dest,const char *src)
{
__asm__ ("cld\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	::"S" (src),"D" (dest):"si","di","ax");
return dest;
}

static inline char * STRNCPY(char * dest,const char *src,int count)
{
__asm__  ("cld\n"
	"1:\tdecl %2\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"rep\n\t"
	"stosb\n"
	"2:"
	::"S" (src),"D" (dest),"c" (count):"si","di","ax","cx");
return dest;
}

static inline char * STRCAT(char * dest,const char * src)
{
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	::"S" (src),"D" (dest),"a" (0),"c" (0xffffffff):"si","di","ax","cx");
return dest;
}

static inline char * STRNCAT(char * dest,const char * src,int count)
{
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n\t"
	"movl %4,%3\n"
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %2,%2\n\t"
	"stosb"
	::"S" (src),"D" (dest),"a" (0),"c" (0xffffffff),"g" (count)
	:"si","di","ax","cx");
return dest;
}

static inline int STRCMP(const char * cs,const char * ct)
{
register int __res __asm__("ax");
__asm__("cld\n"
	"1:\tlodsb\n\t"
	"scasb\n\t"
	"jne 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"xorl %%eax,%%eax\n\t"
	"jmp 3f\n"
	"2:\tmovl $1,%%eax\n\t"
	"jl 3f\n\t"
	"negl %%eax\n"
	"3:"
	:"=a" (__res):"D" (cs),"S" (ct):"si","di");
return __res;
}

static inline int STRNCMP(const char * cs,const char * ct,int count)
{
register int __res __asm__("ax");
__asm__("cld\n"
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"scasb\n\t"
	"jne 3f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %%eax,%%eax\n\t"
	"jmp 4f\n"
	"3:\tmovl $1,%%eax\n\t"
	"jl 4f\n\t"
	"negl %%eax\n"
	"4:"
	:"=a" (__res):"D" (cs),"S" (ct),"c" (count):"si","di","cx");
return __res;
}



static inline void STRFREEBLOCKS(char * dst, char * src)
{
	strcpy(dst, "free_block_of_");
	char *p = dst + 14;
	strcpy(p, src);
}



#endif /* STRING_FUNCTION_H */




#ifndef _DISK_RDWR_H
#define _DISK_RDWR_H

#define B_TREE_NODE_WRITE_DISK(tree, node) \
	pwrite((tree)->fd, node, sizeof(BTreeNode), (node)->self)
		
	
#define B_TREE_NODE_READ_DISK(tree, node, offset) \
	pread((tree)->fd, node, sizeof(BTreeNode), offset)

#define B_TREE_WRITE_DISK(tree) \
	pwrite((tree)->fd, tree, sizeof(b_tree), 0)

#define B_TREE_READ_DISK(tree, fd) \
	pread(fd, tree, sizeof(b_tree), 0)
	
#define B_TREE_FREE_NODE(Tree, node) ({ \
	free_block *__t = (free_block *)malloc(sizeof(free_block)); \
	(__t)->offset = (node)->self; \
	ListTailInsert(&(Tree)->free_blocks, &(__t)->link); \
})
	
#define B_TREE_SYNC(tree) \
	pwrite((tree)->fd, tree, sizeof(b_tree), 0)

#endif /* _DISK_RDWR */






#ifdef _cplusplus 
#if _cplusplus 
}
#endif /* _cplusplus */
#endif /* _cplusplus */
