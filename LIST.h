#ifdef _cplusplus
#if _cplusplus
extern "C" {
#endif /* _BTREENODE_H */
#endif /* _BTREENODE_H */

#ifndef BOOL
#define BOOL int
#endif /* BOOL */

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */



#ifndef _LIST_H
#define _LIST_H

#include "btreedef.h"


/**
 *@auther: jckeep
 * structure of a doubly linked list
 */
typedef struct LIST {
	struct LIST *prev;
	struct LIST *next;
} LIST;



/**
 *@auther: jckeep
 *@Description
 * Create a list head
 *
 *@parameter:
 * list: LIST *list
 */
#define LIST_HEAD(list) LIST list = {&(list), &(list)}



/**
 *@auther: jckeep
 *@Description
 * Obtain the offset of the member of a structure type
 *@parameter:
 * type: struct name
 * member: a member of type
 */
#define LIST_OFF_SET_OF(type, member) ((unsigned long)&((type *)0)->member)



#define LIST_MAKE_HEAD(list) ((list)->next = (list)->prev = (list))



/**
 *@auther: jckeep
 *@Description
 * Obtain a address from item
 *
 *@parameter:
 * item: a member of type
 * type: a structure
 * member:
 */
#define LIST_ENTRY(item, type, member) \
	((type *)(void *)((char *)(item) - LIST_OFF_SET_OF(type, member)))

/**
 *@auther: jckeep
 *
 */
#define LIST_FOR_EACH(item, list) \
	for (item = (list)->next; (item) != (list); item = (item)->next)



/**
 *@auther: jckeep
 *
 */
#define LIST_FOR_EACH_ENTRY(item, list, type, member) \
	for (item = LIST_ENTRY((list)->next, type, member);\
		&(item)->member != (list);\
		item = LIST_ENTRY((item)->member.next, type, member))



/**
 *@auther: jckeep
 *
 */
#define LIST_FOR_EACH_ENTRY_SAFE(item, next, list, type, member) \
	for (item = LIST_ENTRY((list)->next, type, member), \
		next = LIST_ENTRY((item)->member.next, type, member);\
		&(item)->member != (list);\
		item = next, next = LIST_ENTRY((item)->member.next, type, member))



STATIC VOID ListInit(LIST *list) 
{
	list->next = list;
	list->prev = list;
}


STATIC VOID ListDelete(LIST *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	node->next = NULL;
	node->prev = NULL;
}


STATIC VOID ListAdd(LIST *list, LIST *node)
{
    node->next = list->next;
    node->prev = list;
    list->next->prev = node;
    list->next = node;
}


STATIC VOID ListTailInsert(LIST *list, LIST *node) 
{
	ListAdd(list->prev, node);
}


STATIC VOID listHeadInsert(LIST *list, LIST *node)
{
	ListAdd(list, node);
}


STATIC BOOL ListEmpty(LIST *list)
{
	return (BOOL)(list->next == list);
}


#endif /* _LIST_H */




#ifdef _cplusplus
#if _cplusplus
}
#endif /* _cplusplus */
#endif /* _cplusplus */
