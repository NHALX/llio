#include "u_list.h"
#include <string.h>


static struct u_list *PageNew(void *v)
{
	struct u_list *node;
	GUARD(node = p_alloc(P_ALLOC_ULIST));

	node->value[PUSH_PAGE_TOP] = v;
	node->value_len = 1;
	node->prev_page = node;
	node->next_page = node;
	return node;
}


void *ul_first(struct u_list *x)
{
	assert(x);
	assert(x->value_len >= 1);
	return x->value[LIST_UNROLL_SIZE - x->value_len];
}


struct u_list * ul_push(struct u_list **head, void *v)
{
	struct u_list *first = *head;

	if (first && first->value_len < LIST_UNROLL_SIZE)
	{
		int i = first->value_len;
		first->value[PUSH_PAGE_TOP - i] = v;
		first->value_len++;
		return first;
	}
	else
	{
		struct u_list *node;
		GUARD(node = PageNew(v));

		if (first){
			node->next_page = first;
			node->prev_page = first->prev_page;
			
			first->prev_page->next_page = node;
			first->prev_page = node;
		}

		*head = node;
		return node;
	}
}



/*
// OPTIMIZATION: this accounts for too much time, fix it.
void ul_unlink(struct u_list **head, void *v)
{
	struct u_iterator k;
	struct u_list *prev = 0;
	int i;

	FOR_X_IN_LIST(k, *head)
	{
		if (UL_X(k) == v)
		{
			if (k.node->value_len - 1 == 0) // unlink node
			{
				if (*head == k.node)
					*head = k.node->next_page;

				k.node->prev_page->next_page = k.node->next_page;

				if (k.node->next_page)
					k.node->next_page->prev_page = k.node->prev_page;
			}
			else
			{
				int j = PUSH_PAGE_TOP;
				for (i = PUSH_PAGE_TOP; i >= 0; --i)
					if (k.node->value[i] != v)
						k.node->value[j--] = k.node->value[i];

				k.node->value_len--;
			}

			return;
		}
	}
}


struct u_list * ul_append(struct u_list **head, void *v)
{
	struct u_list *tail;

	if (!(*head))
		return ul_push(head, v);
	
	tail = (*head)->prev_page;

	if (tail->value_len < LIST_UNROLL_SIZE)
	{
		size_t n = tail->value_len;
		memmove(
			&tail->value[PUSH_PAGE_TOP - n], 
			&tail->value[PUSH_PAGE_TOP - (n-1)],
			n*sizeof *tail->value);

		tail->value[PUSH_PAGE_TOP] = v;
		tail->value_len++;
		return tail;
	}
	else
	{
		struct u_list *node;
		GUARD(node = PageNew(v));
		
		node->next_page = tail->next_page;
		node->prev_page = tail;

		tail->next_page    = node;
		(*head)->prev_page = node;
		return node;
	}
}
*/
