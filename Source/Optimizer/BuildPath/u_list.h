#include <assert.h>
#include <stdalign.h> 
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
	struct vertex *value[LIST_UNROLL_SIZE];
	intptr_t       value_len;
};

static_assert(sizeof(struct u_list) <= 64, "sizeof (struct u_list) <= 64)");
static_assert(LIST_UNROLL_SIZE > 1, "sizeof (struct u_list) <= 64)");

#define ULIST_STATIC_INITIALIZER {NULL,NULL}

struct u_lhead {
	struct u_list *first;
	struct u_list *last;
};

struct u_iterator {
	struct u_list *node;
	intptr_t      i;
	intptr_t      n;
};

#define FOR_X_IN_LIST(I,HEAD) \
	for ((I).node = (HEAD)->first, \
		 (I).n = 0; \
		 (I).node; \
		 (I).node = (I).node->next_page) \
		for ((I).i = LIST_UNROLL_SIZE - (I).node->value_len; \
			 (I).i < LIST_UNROLL_SIZE; \
			 (I).i++, (I).n++)

#define FOR_X_IN_LIST_REVERSE(I,HEAD) \
	for ((I).node = (HEAD)->last, \
		 (I).n = 0; \
		 (I).node; \
		 (I).node = (I).node->prev_page) \
		for ((I).i = LIST_UNROLL_SIZE-1; \
			 (I).i >= LIST_UNROLL_SIZE - (I).node->value_len; \
			 (I).i--, (I).n++)

#define UL_X(I)  (I).node->value[(I).i]
#define UL_N(I)  (I).n


void ul_unlink(struct u_lhead *head, struct vertex *v);
struct u_list * ul_push(struct u_lhead *head, struct vertex *v);
struct vertex *ul_first(struct u_lhead *x);

