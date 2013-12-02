#define G_SUCCESS     1
#define G_ERROR       0
#define GUARD(EXPR)   if ((EXPR) == G_ERROR){ return G_ERROR; }
#define G_FREE(PTR)   if (PTR) free(PTR)
