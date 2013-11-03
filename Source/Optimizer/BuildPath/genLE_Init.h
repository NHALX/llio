#define STATIC

////////////////////// list //////////////////////

struct slist {
	struct slist  *next;
	struct vertex *value;
};

struct slist_head {
	struct slist *first;
	struct slist *last;
};


STATIC __inline void sl_unlink(struct slist **head, struct vertex *v)
{
	struct slist *node;
	struct slist *prev = NULL;

	for (node = *head; node != NULL; node = node->next)
	{
		if (node->value == v)
		{
			if (prev)
				prev->next = node->next;
			else
			{
				assert(node == *head);
				*head = node->next;
			}

			// TODO: free(node);
			return;
		}
		prev = node;
	}
}

#define BULLSHIT_STORAGE_SIZE 9048576
size_t s_alloc_index = 0;
struct slist s_alloc_storage[BULLSHIT_STORAGE_SIZE];

STATIC __inline struct slist * sl_alloc()
{
	assert(s_alloc_index + 1 <= BULLSHIT_STORAGE_SIZE);
	return &s_alloc_storage[s_alloc_index++];
}



STATIC __inline void sl_push(struct slist **head, struct vertex *v)
{
	struct slist *node = sl_alloc();
	node->next = *head;
	node->value = v;
	*head = node;
}


STATIC __inline void slh_append(struct slist_head *head, struct vertex *value)
{
	struct slist *x = sl_alloc();
	x->value = value;
	x->next = NULL;

	if (head->first == NULL)
		head->first = head->last = x;
	else
	{
		head->last->next = x;
		head->last       = x;
	}
}


////////////////////// Vertex //////////////////////

struct vertex
{
	size_t           index; // this can be optimized out since its easy to calculate the offset from the pool start
	struct vertex    *parent;
	struct slist     *children;
	struct slist_head impred;
	c_ideal_t         label;

	// HARD_LIMITS: these will be too small if we have 2^256 vertices in our lattice...
	unsigned char     children_len; 
	unsigned char     edge_len;
};

#define EDGE(X,I,J)	   (X->edges[(I*X->max_neighbors)+J])

struct ctx {
	c_ideal_t *edges;
	size_t max_neighbors;
	size_t vertex_count;
};

size_t v_alloc_index = 0;
struct vertex v_alloc_storage[BULLSHIT_STORAGE_SIZE];
STATIC __inline struct vertex * v_alloc()
{
	assert(v_alloc_index + 1 <= BULLSHIT_STORAGE_SIZE);
	// TODO: memory must be zero initialized
	return &v_alloc_storage[v_alloc_index++];
}

STATIC __inline struct vertex *vertex(struct ctx *x)
{
	size_t i = x->vertex_count++;

	struct vertex *v = v_alloc();
	memset(v, 0, sizeof *v);
	v->index = i;
	return v;
}


STATIC __inline void addChild(struct ctx *x, struct vertex * p, struct vertex * i)
{
	assert(p->children_len < UCHAR_MAX);

	sl_push(&p->children, i);
	p->children_len++;

	if (x->max_neighbors < p->children_len)
		x->max_neighbors = p->children_len;
}

STATIC __inline void delChild(struct vertex *p, struct vertex *c)
{
	sl_unlink(&p->children, c);
	p->children_len--;
}
