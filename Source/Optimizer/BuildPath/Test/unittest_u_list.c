#include <stdlib.h>
#include <string.h>

#include "../u_list.h"

#define TEST_SAMPLE_SIZE ((LIST_UNROLL_SIZE * 3) + (LIST_UNROLL_SIZE/2))

void
unittest_ul_push()
{
	uintptr_t buf[TEST_SAMPLE_SIZE + 1];
	int j, offset;
	struct u_iterator i;
	struct u_lhead list = ULIST_STATIC_INITIALIZER;

	memset(buf, 0, sizeof buf); buf[TEST_SAMPLE_SIZE] = 7777; // canary

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		ul_push(&list, (void*)j);

	// test forward traversal
	j = 0;
	FOR_X_IN_LIST(i, &list)
		buf[j++] = (uintptr_t)UL_X(i);

	assert(buf[TEST_SAMPLE_SIZE] == 7777);

	offset = TEST_SAMPLE_SIZE - 1;
	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		assert(buf[j] == offset - (uintptr_t)j);

	// test reverse traversal
	memset(buf, 0, sizeof buf); buf[TEST_SAMPLE_SIZE] = 7777; // canary

	j = 0;
	FOR_X_IN_LIST_REVERSE(i, &list)
		buf[j++] = (uintptr_t)UL_X(i);
	
	assert(buf[TEST_SAMPLE_SIZE] == 7777);

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		assert(buf[j] == j);
}

/*
void
unittest_slh_append()
{
	uintptr_t buf[TEST_SAMPLE_SIZE + 1];
	int j;
	struct u_iterator i;
	struct u_lhead head;

	memset(&head, 0, sizeof head);
	memset(buf, 0, sizeof buf);

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		slh_append(&head, (void*)j);

	j = 0;
	FOR_X_IN_LIST(i, head.first)
		buf[j++] = (uintptr_t)UL_X(i);

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		assert(buf[j] == (uintptr_t)j);
}
*/

void
unittest_ul_unlink()
{
	uintptr_t buf[6];
	uintptr_t expected[6] = { 1, 2, 4, 5, 6, 8 };
	int j;
	struct u_iterator i;
	struct u_lhead list = ULIST_STATIC_INITIALIZER;
	memset(buf, 0, sizeof buf);

	ul_push(&list, (void*)8);
	ul_push(&list, (void*)7);
	ul_push(&list, (void*)6);
	ul_push(&list, (void*)5);
	ul_push(&list, (void*)4);
	ul_push(&list, (void*)3);
	ul_push(&list, (void*)2);
	ul_push(&list, (void*)1);

	ul_unlink(&list, (void*)3);
	ul_unlink(&list, (void*)7);

	j = 0;
	FOR_X_IN_LIST(i, &list)
		buf[j++] = (uintptr_t)UL_X(i);

	assert(memcmp(buf, expected, sizeof buf) == 0);
}

void
unittest_ul_first()
{
	int j;
	struct u_lhead list = ULIST_STATIC_INITIALIZER;
	uintptr_t x;
	uintptr_t y;

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		ul_push(&list, (void*)j);

	x = (uintptr_t) ul_first(&list);
	y = TEST_SAMPLE_SIZE-1;

	assert(x == y);
}



void
unittest_u_list()
{
	size_t size = sizeof(struct u_list);
	assert(size == 64);

	unittest_ul_push();
	//unittest_slh_append();
	unittest_ul_unlink();
	unittest_ul_first();
}
