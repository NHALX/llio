#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

/* // debug:
#define MATHEMATICA_EXPORT
#include "D:\Program Files\Wolfram Research\Mathematica\9.0\SystemFiles\Links\MathLink\DeveloperKit\Windows\CompilerAdditions\mldev32\include\mathlink.h"
#include "D:\Program Files\Wolfram Research\Mathematica\9.0\SystemFiles\IncludeFiles\C\WolframLibrary.h"
*/

#ifdef MATHEMATICA_EXPORT
#include "mathlink.h"
#include "WolframLibrary.h"

DLLEXPORT mint WolframLibrary_getVersion( ) {
    return WolframLibraryVersion;
}

DLLEXPORT int WolframLibrary_initialize( WolframLibraryData libData) {
    return 0;
}

DLLEXPORT void WolframLibrary_uninitialize( WolframLibraryData libData) {
    return;
}

#else
typedef int mint;
typedef int mbool;
#endif

#define false 0
#define true  1
typedef struct{mint* ptr; mint len;} set_t;


jmp_buf exception;
__declspec(noreturn) void error(){ longjmp(exception, 1); }

#define ERROR_NO_MEM 1234

////////////////////////////////////////////////////////////////////////////////////
// CONTEXT

struct context {

	mbool isPlus;
	mint maxPair;
	set_t le;
	set_t b;
	set_t a;
	mint *matrix;
	mint  matrix_width;
	mint  count;
	void (*record)(struct context*, void *);
	void *record_context;

	mint (*aborted)();
	mbool abort_processed;
};

static void context_init(struct context *ctx, 
	mint *le, mint le_n, 
	mint *a, mint a_n, 
	mint *b, mint b_n, 
	mint maxPair, 
	mint *matrix, mint matrix_width)
{
	ctx->record          = NULL;
	ctx->record_context  = NULL;
	ctx->aborted         = NULL;
	ctx->abort_processed = false;

	ctx->isPlus       = true;
	ctx->maxPair      = maxPair;
	ctx->le.ptr       = le;
	ctx->le.len       = le_n;
	ctx->a.ptr        = a;
	ctx->a.len        = a_n;
	ctx->b.ptr        = b;
	ctx->b.len        = b_n;
	ctx->matrix       = matrix;
	ctx->matrix_width = matrix_width;
	ctx->count        = 1;
}

////////////////////////////////////////////////////////////////////////////////////
// RECORDING WITH LINKED LIST 

struct Snode {
	struct Snode *next;
	mint         *data;
};

struct record_context {
	struct Snode *last;
	mint error_code;
};

#define RECORD_LL_STATIC_INIT(HEAD)  {HEAD,0}


static void ll_free(struct Snode *n)
{
	while (n)
	{
		void *x = n;
		
		if (n->data)
			free(n->data);
		
		n = n->next;
		free(x);
	}
}


static void record_callback_ll(struct context *ctx, void *args)
{
	struct record_context *r = (struct record_context *) args;
	struct Snode *n;
	
	if ((n = (struct Snode *) malloc(sizeof(struct Snode))) == NULL)
		{ r->error_code = ERROR_NO_MEM; error(); }

	n->next = NULL;

	if ((n->data = (mint*) malloc(sizeof(*ctx->le.ptr)*ctx->le.len)) == NULL)
	{
		free(n);
		r->error_code = ERROR_NO_MEM; 
		error();
	}


	memcpy(n->data,	ctx->le.ptr, ctx->le.len*sizeof(*ctx->le.ptr));

	r->last->next = n;
	r->last = n;
}


////////////////////////////////////////////////////////////////////////////////////
// Main algorithm
/*
 * "Generating Linear Extensions Fast" by Gara Pruesse and Frank Ruskey
 * SIAM J. Comput., 23(2), 373–386. 
 */

void record(struct context *ctx)
{
	if (ctx->aborted && ctx->aborted())
		{ ctx->abort_processed = true; error(); }

	if ((ctx->count++ % 2) == 1 && ctx->record)
		ctx->record(ctx, ctx->record_context);
}


static mint idx(struct context *ctx, mint e)
{
	mint i;

	for (i = 0; i < ctx->le.len; i++)
	{
		if (ctx->le.ptr[i] == e)
				return i;
	}

	printf("error: idx, %p, %d\n", &ctx->le, e);
	abort();
}



static void swap(struct context *ctx, mint i)
{
	if (i == -1) // PORTING: assumes C zero-based indices
		ctx->isPlus = !ctx->isPlus;
	
	else if (i >= 0)
	{
		mint temp;
		mint aindex=idx(ctx, ctx->a.ptr[i]); 
		mint bindex=idx(ctx, ctx->b.ptr[i]);

		ctx->le.ptr[aindex] = ctx->b.ptr[i];
		ctx->le.ptr[bindex] = ctx->a.ptr[i];
		temp = ctx->b.ptr[i];
		ctx->b.ptr[i] = ctx->a.ptr[i];
		ctx->a.ptr[i] = temp;
	}
	
	record(ctx);
}

static void move(struct context *ctx, mint element, mint d)
{
	mint i = idx(ctx, element);
	ctx->le.ptr[i]   = ctx->le.ptr[i+d];
	ctx->le.ptr[i+d] = element;
	record(ctx);
}

#define moveL(E) move(ctx, ctx->E,-1)
#define moveR(E) move(ctx, ctx->E, 1)

#define rightA(i)  right(ctx, ctx->a, i, false) 
#define rightB(I)  right(ctx, ctx->b, i, true)



static mbool incomparable(struct context *ctx, mint x, mint y)
{
	mint index = ((x-1)*ctx->matrix_width)+(y-1); // PORTING: assumes C zero-based indices
	return ctx->matrix[index] == 0;
}

static mbool right(struct context *ctx, set_t ab, mint i, mbool skip_test)
{
	mint x, yi, y;

	x  = ab.ptr[i];
	yi = idx(ctx,x) + 1;

	if (yi >= ctx->le.len) // PORTING: assumes C zero-based indices
		return false;

	y = ctx->le.ptr[yi];
	return ((skip_test || y != ctx->b.ptr[i]) && incomparable(ctx,x,y));
}

#define OddQ(X) (((X) % 2) == 1)

static void genLE(struct context *ctx, mint i)
{
	mint j;
	mint mrb,mra;
	mbool typical;

	if (i < 0) // PORTING: assumes C zero-based indices
		return;

	genLE(ctx,i-1);
	mrb     = 0;
	typical = false;

	while (rightB(i))
	{
		mrb += 1;
		moveR(b.ptr[i]); genLE(ctx,i-1);
		mra = 0;

		if (rightA(i))
		{
			typical = true;
			do
			{
				mra += 1;
				moveR(a.ptr[i]); genLE(ctx,i-1);

			} while (rightA(i));
		}

		if (typical)
		{
			mint mla;
			swap(ctx,i-1); genLE(ctx,i-1);
			mla = (OddQ(mrb)) ? mra-1 : mra+1;
				
			for (j = 0; j < mla; ++j)
			{
				moveL(a.ptr[i]); genLE(ctx,i-1);
			}					
		}
	}

	if (typical && OddQ(mrb))
		moveL(a.ptr[i]);
	else
		swap(ctx,i-1);
			
	genLE(ctx,i-1);

	for (j = 0; j < mrb; ++j)
	{
		moveL(b.ptr[i]); genLE(ctx,i-1);
	}
	
}

////////////////////////////////////////////////////////////////////////////////////


static void start(struct context *ctx)
{
	record(ctx);
	genLE(ctx,ctx->maxPair);
	swap(ctx, ctx->maxPair);
	genLE(ctx, ctx->maxPair);
}


#ifndef MATHEMATICA_EXPORT
static void record_callback_console(struct context *ctx, void *unused)
{
	printf("%d, {%d,%d,%d,%d,%d,%d}\n", 
		ctx->count/2, 
		ctx->le.ptr[0], ctx->le.ptr[1], 
		ctx->le.ptr[2], ctx->le.ptr[3], 
		ctx->le.ptr[4], ctx->le.ptr[5]
	);
}


void main2()
{
	struct Snode head = { NULL, NULL };
	struct record_context r = RECORD_LL_STATIC_INIT(&head);

	mint matrix[][6] = {{0, 1, 1, 0, 0, 0},
					   {1, 0, 0, 1, 0, 0}, 
	                   {1, 0, 0, 0, 0, 0}, 
	                   {0, 1, 0, 0, 1, 1}, 
	                   {0, 0, 0, 1, 0, 0}, 
	                   {0, 0, 0, 1, 0, 0}}; 
	
	mint le[] = {3,5,1,6,4,2};
	mint a[] = {3,1};
	mint b[] = {5,6};
	struct context ctx;

	context_init(&ctx, le, 6, a, 2, b, 2, 2-1, (mint*)matrix, 6);

	if (setjmp(exception) == 1)
		return;
	
	else
	{
		struct Snode *n;

		ctx.record_context = &r;
		ctx.record         = &record_callback_ll;
		//ctx.record = &record_console;
		start(&ctx);

		for (n = head.next; n; n = n->next)
			printf("{%d,%d,%d,%d,%d,%d}\n", 
				n->data[0], n->data[1], n->data[2], 
				n->data[3], n->data[4], n->data[5]);

		ll_free(head.next);

		printf("%d\n", ctx.count/2);
	}
}
#endif

#ifdef MATHEMATICA_EXPORT

#define WOLFRAM_EXPORT(NAME)                                                               \
	DLLEXPORT int NAME##(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res)  


static void wolfram_ctx_init(struct context *ctx, WolframLibraryData libData, mint Argc, MArgument *Args)
{
	MTensor le, a, b, matrix;
	mint maxCount, matrix_width;

	le           = MArgument_getMTensor(Args[0]);
	a            = MArgument_getMTensor(Args[1]);
	b            = MArgument_getMTensor(Args[2]);
	maxCount     = MArgument_getInteger(Args[3]);
	matrix       = MArgument_getMTensor(Args[4]);
	matrix_width = MArgument_getInteger(Args[5]);
		
	context_init(ctx, 
		libData->MTensor_getIntegerData(le),   libData->MTensor_getFlattenedLength(le), 
		libData->MTensor_getIntegerData(a),    libData->MTensor_getFlattenedLength(a), 
		libData->MTensor_getIntegerData(b),    libData->MTensor_getFlattenedLength(b), 
		maxCount-1, // PORTING: assumes C zero-based indices
		libData->MTensor_getIntegerData(matrix), matrix_width);

}



WOLFRAM_EXPORT(linearExtensionsN)
{                                                                                                                                                                      
	struct context ctx; 
	wolfram_ctx_init(&ctx, libData, Argc, Args);                                                                                 
	                                                                                       
	if ((setjmp(exception)) != 0)   
		return (ctx.abort_processed) ? LIBRARY_NO_ERROR : LIBRARY_DIMENSION_ERROR;
	
	else
	{       
		ctx.aborted = libData->AbortQ;
		start(&ctx);
		
		///////////////

		MArgument_setInteger(Res, ctx.count/2);
		return LIBRARY_NO_ERROR;                                                           
	}                                                                                      
}




//////////////////////////////////////////////

WOLFRAM_EXPORT(linearExtensions)
{
	struct context ctx; 
	struct Snode head = { NULL, NULL };
	struct record_context r = RECORD_LL_STATIC_INIT(&head);

	wolfram_ctx_init(&ctx, libData, Argc, Args);

	if (setjmp(exception) != 0)   
	{
		int error;

		if (ctx.abort_processed)
			return LIBRARY_NO_ERROR;

		switch(r.error_code){ 
		case ERROR_NO_MEM : error = LIBRARY_MEMORY_ERROR;   break;
		default           : error = LIBRARY_FUNCTION_ERROR; break;
		}

		if (error == LIBRARY_FUNCTION_ERROR)
			libData->Message( "Error: Unknown exception raised.");

		return error;		
	
	} else                                                                                   
	{  
		MTensor T;
		mint dims[2];
		mint i;
		mint *data;
		struct Snode *n;

		ctx.aborted        = libData->AbortQ;
		ctx.record_context = &r;
		ctx.record         = &record_callback_ll;

		start(&ctx);
		
		///////////////

		dims[0] = ctx.count / 2;
		dims[1] = ctx.le.len;

		if (libData->MTensor_new(MType_Integer, 2, (const mint*)&dims, &T))
		{
			ll_free(head.next);
			return LIBRARY_MEMORY_ERROR;
		}
		data = libData->MTensor_getIntegerData(T);

		for (i = 0, n = head.next; n; n = n->next, i+=dims[1])
			memcpy(data+i, n->data, dims[1]*sizeof(*n->data));

		ll_free(head.next);
		MArgument_setMTensor(Res, T);
		return LIBRARY_NO_ERROR;
	}
}


#endif

