#include <stdint.h>
#include <malloc.h>
#include <assert.h>

#include "p_alloc.h"


struct p_pool *p_glbpool;


static struct p_page * addpage(struct p_page **head, size_t n)
{
	struct p_page *p = malloc(sizeof *p);
	void *heap = malloc(P_PAGE_SIZE);

	if (!p || !heap)
	{
		G_FREE(p);
		G_FREE(heap);
		return G_ERROR;
	}

	p->next  = *head;
	p->index = 0;
	p->len   = n;
	p->heap = heap;
	
	*head = p;
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

		if (!addpage(&p_glbpool[i].pages, P_PAGE_SIZE / typsiz[i]))
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
	struct p_page *page;
	void *mem;
	
	typesiz = p_glbpool[type_id].element_size;
	page    = p_glbpool[type_id].pages;

	if (page->index >= page->len)
		GUARD(page = addpage(&p_glbpool[type_id].pages, P_PAGE_SIZE / typesiz));

	mem = page->heap + (page->index * typesiz);
	page->index++;
	return mem;
}


uintptr_t p_index(void *ptr, size_t type_id)
{
	struct p_page *page;
	uintptr_t element = (uintptr_t) ptr;

	for (page = p_glbpool[type_id].pages; page; page = page->next)
	{
		uintptr_t start = (uintptr_t) page->heap;
		uintptr_t end   = start + P_PAGE_SIZE;
		if (start <= element && element < end)
			return (element - start) / p_glbpool[type_id].element_size;
	}

	assert(0);
	return -1;
}


void p_release(size_t type_id)
{
	struct p_page *page, *next;

	for (page = p_glbpool[type_id].pages->next; page; page = next)
	{
		free(page->heap);
		next = page->next;
		free(page);
	}

	p_glbpool[type_id].pages->next  = 0;
	p_glbpool[type_id].pages->index = 0;
}
