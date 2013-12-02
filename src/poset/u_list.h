#ifndef _U_LIST_H_
#define _U_LIST_H_

#include <assert.h>
//#include <stdalign.h> 
#include <stdint.h>
#include "p_alloc.h"

#define STATIC

////////////////////// list //////////////////////
#define CACHE_LINE_SIZE			64
#define LIST_UNROLL_SIZE		(intptr_t) ((CACHE_LINE_SIZE/sizeof(void*))-3)

#define PUSH_PAGE_TOP (LIST_UNROLL_SIZE-1)

//__declspec(align(64))
// TODO: alignas isn't working on clang..
struct u_list  {
	struct u_list  *next_page;
	struct u_list  *prev_page;
	void           *value[LIST_UNROLL_SIZE];
	intptr_t        value_len;
};

static_assert(sizeof(struct u_list) <= CACHE_LINE_SIZE, "sizeof (struct u_list) > CACHE_LINE_SIZE)");
static_assert(LIST_UNROLL_SIZE >= 1, "LIST_UNROLL_SIZE < 1");


struct u_iterator {
	struct u_list *node;
	intptr_t      i;
	intptr_t      n;
	size_t        page;
};


#define _FXL_OUTER(I,HEAD,DIRECTION) \
for ((I).node = (HEAD), (I).n = 0, (I).page = 0; \
	(I).node && ((I).node != (HEAD) || (I).page == 0); \
	(I).node = (I).node->##DIRECTION, (I).page++) 


// Don't forget this is a nested loop - use goto to break.
#define FOR_X_IN_LIST(I,HEAD) \
	_FXL_OUTER(I,HEAD,next_page) \
		for ((I).i = LIST_UNROLL_SIZE - (I).node->value_len; \
			 (I).i < LIST_UNROLL_SIZE; \
			 (I).i++, (I).n++)

#define FOR_X_IN_LIST_REVERSE(I,HEAD) \
	_FXL_OUTER(I,(HEAD) ? (HEAD)->prev_page : 0, prev_page) \
		for ((I).i = LIST_UNROLL_SIZE-1; \
			 (I).i >= LIST_UNROLL_SIZE - (I).node->value_len; \
			 (I).i--, (I).n++)

#define UL_X(I)  ((I).node->value[(I).i])
#define UL_N(I)  ((I).n)


void ul_unlink(struct u_list **head, void *v);
struct u_list * ul_push(struct u_list **head, void *v);
struct u_list * ul_append(struct u_list **head, void *v);
void *ul_first(struct u_list *x);

#endif
