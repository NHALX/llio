#include "u_list.h"

struct vertex *ul_first(struct u_lhead *x)
{
	assert(x && x->first->value_len >= 1);
	return x->first->value[LIST_UNROLL_SIZE - x->first->value_len];
}

// OPTIMIZATION: this accounts for too much time, fix it.
void ul_unlink(struct u_lhead *head, struct vertex *v)
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
				if (k.node == head->first && k.node == head->last)
					head->first = head->last = 0;
				else
				{
					if (k.node == head->first)
						head->first = k.node->next_page;

					else if (k.node == head->last)
						head->last = k.node->prev_page;

					if (k.node->prev_page)
						k.node->prev_page->next_page = k.node->next_page;

					if (k.node->next_page)
						k.node->next_page->prev_page = k.node->prev_page;
				}
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


struct u_list * ul_push(struct u_lhead *head, struct vertex *v)
{
	struct u_list *x = head->first;

	if (x && x->value_len < LIST_UNROLL_SIZE)
	{
		int i = x->value_len;
		x->value[PUSH_PAGE_TOP - i] = v;
		x->value_len++;
		return x;
	}
	else
	{
		struct u_list *node;

		if (!(node = p_alloc(P_ALLOC_ULIST)))
			return 0;

		node->prev_page = 0;
		node->next_page = x;
		node->value[PUSH_PAGE_TOP] = v;
		node->value_len = 1;

		if (node->next_page)
			node->next_page->prev_page = node;

		head->first = node;

		if (head->last == 0)
			head->last = head->first;

		return node;
	}
}

