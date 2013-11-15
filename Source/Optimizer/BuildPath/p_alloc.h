#ifndef _P_ALLOC_H_
#define _P_ALLOC_H_

#include <stddef.h>

#define G_SUCCESS     1
#define G_ERROR       0
#define GUARD(EXPR)   if ((EXPR) == G_ERROR){ return G_ERROR; }
#define G_FREE(PTR)   if (PTR) free(PTR)

#define P_ALLOC_ULIST  0
#define P_ALLOC_VERTEX 1

#define P_PAGE_SIZE 33554432

struct p_page {
	size_t         index;
	size_t         len;  // number of element_size objects in heap
	unsigned char *heap;
	struct p_page *next;
};

struct p_iterator {
	struct p_page *page;
	size_t        local_index;
	size_t        global_index;
	size_t         type_size;
};



struct p_pool {
	size_t element_size;
	struct p_page *pages;
	size_t page_count;
};

extern struct p_pool *p_glbpool;


#define FOR_X_IN_POOLS(I,TYPE_ID) \
for ((I).page = p_glbpool[TYPE_ID].pages, \
	(I).type_size = p_glbpool[TYPE_ID].element_size, \
	(I).global_index = 0; \
	(I).page; \
	(I).page = (I).page->next) \
for ((I).local_index = 0; \
	(I).local_index < (I).page->index; \
	(I).local_index++, (I).global_index++)

#define P_X(I)	\
	((void*)((I).page->heap + ((I).local_index * (I).type_size)))


void  p_release(size_t type_id);
int   p_init(size_t *typsiz, int n);
void* p_alloc(size_t type_id);
void p_free(void *p, size_t type_id);
size_t p_memusage(size_t type_id);
uintptr_t p_index(void *ptr, size_t type_id);
void * p_ptr(uintptr_t ptr, size_t type_id);
#endif
