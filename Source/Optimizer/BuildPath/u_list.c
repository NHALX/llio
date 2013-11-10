#include "u_list.h"

void *ul_first(struct u_lhead *x)
{
	assert(x->first);
	assert(x->first->value_len >= 1);
	return x->first->value[LIST_UNROLL_SIZE - x->first->value_len];
}

// OPTIMIZATION: this accounts for too much time, fix it.
void ul_unlink(struct u_lhead *head, void *v)
{
	struct u_iterator k;
	struct u_list *prev = 0;
	int i;

	FOR_X_IN_LIST(k, head)
	{
		if (UL_X(k) == v)
		{
			if (k.node->value_len - 1 == 0) // unlink node
			{
				if (k.node == head->first)
					head->first = k.node->next_page;

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


struct u_list * ul_push(struct u_lhead *head, void *v)
{
	struct u_list *first = head->first;

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

		GUARD(node = p_alloc(P_ALLOC_ULIST));
		//GUARD(node = malloc(sizeof *node));
		node->value[PUSH_PAGE_TOP] = v;
		node->value_len = 1;

		if (first){
			node->next_page = first;
			node->prev_page = first->prev_page;
			first->prev_page = node;
		}
		else
		{
			node->prev_page = node;
			node->next_page = 0; // list is only cyclical when traversing backwards.
		}

		head->first = node;
		return node;
	}
}

