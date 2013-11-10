#include <stdint.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "p_alloc.h"


struct p_pool *p_glbpool;


struct p_page *findpage(void *ptr, size_t type_id, uintptr_t *out_index, struct p_page **prev)
{
	struct p_page *page;
	uintptr_t element = (uintptr_t)ptr;
	struct p_page **reverse = alloca(p_glbpool[type_id].page_count * sizeof *reverse);
	size_t i = 0;
	size_t index;

	for (page = p_glbpool[type_id].pages; page; page = page->next)
		reverse[i++] = page;
	
	for (index = 0; i > 0; --i)
	{
		uintptr_t start = (uintptr_t)reverse[i - 1]->heap;
		uintptr_t end = start + P_PAGE_SIZE;

		if (start <= element && element < end)
		{
			if (out_index)
				*out_index = (index + (element - start)) / p_glbpool[type_id].element_size;
			
			if (prev)
				*prev = (i >= 2) ? reverse[i - 2] : 0;

			return reverse[i - 1];
		}
		index += P_PAGE_SIZE;
	}

	return 0;
}


static struct p_page * addpage(size_t type_id, size_t n)
{
	struct p_page *p = malloc(sizeof *p);
	void *heap = malloc(P_PAGE_SIZE);

	if (!p || !heap)
	{
		G_FREE(p);
		G_FREE(heap);
		return G_ERROR;
	}

	p->next  = p_glbpool[type_id].pages;
	p->index = 0;
	//p->free  = 0;
	p->len   = n;
	p->heap  = heap;
	
	p_glbpool[type_id].pages = p;
	p_glbpool[type_id].page_count++;
	return p;
}



int p_init(size_t *typsiz, int n)
{
	int i;

	GUARD(p_glbpool = malloc(n * sizeof *p_glbpool));

	for (i = 0; i < n; ++i)
	{
		if (typsiz[i] > P_PAGE_SIZE)
			goto FAIL;

		p_glbpool[i].element_size = typsiz[i];
		p_glbpool[i].pages        = 0;
		p_glbpool[i].page_count   = 0;

		if (!addpage(i, P_PAGE_SIZE / typsiz[i]))
		{
		FAIL:
			for (i=i-1; i>=0; --i)
			{
				free(p_glbpool[i].pages->heap);
				free(p_glbpool[i].pages);
			}

			free(p_glbpool);
			return G_ERROR;
		}
	}

	return G_SUCCESS;
}


void* p_alloc(size_t type_id)
{
	size_t typesiz;
	struct p_pool *pool = &p_glbpool[type_id];
	struct p_page *page;
	void *mem;

	typesiz = pool->element_size;
	page = pool->pages;

	if (!page || page->index >= page->len)
		GUARD(page = addpage(type_id, P_PAGE_SIZE / typesiz));

	mem = page->heap + (page->index * typesiz);
	page->index++;
	return mem;
}


void * p_ptr(uintptr_t index, size_t type_id)
{
	struct p_page *page;
	uintptr_t *reverse = alloca(p_glbpool[type_id].page_count * sizeof *reverse);
	size_t i = 0;

	for (page = p_glbpool[type_id].pages; page; page = page->next)
		reverse[i++] = (uintptr_t)page->heap;

	index *= p_glbpool[type_id].element_size;

	for (; i > 0; --i)
	{
		uintptr_t start = reverse[i - 1];
		uintptr_t end = start + P_PAGE_SIZE;
		uintptr_t ptr = start + index;

		if (start <= ptr && ptr < end)
			return (void *) ptr;

		index -= P_PAGE_SIZE;
	}

	assert(0);
	return 0;
}

uintptr_t p_index(void *ptr, size_t type_id)
{
	size_t index;
	struct p_page *page = findpage(ptr, type_id, &index, 0);
	assert(page);
	if (!page)
		return -1;
	
	return index;
}

void p_release(size_t type_id)
{
	struct p_page *page, *next;

	for (page = p_glbpool[type_id].pages; page; page = next)
	{
		free(page->heap);
		next = page->next;
		free(page);
	}

	p_glbpool[type_id].page_count = 0;
	p_glbpool[type_id].pages      = 0;
}

size_t p_memusage(size_t type_id)
{
	struct p_page *page;
	size_t size = 0;

	for (page = p_glbpool[type_id].pages; page; page = page->next)
		size += P_PAGE_SIZE;

	return size;
}