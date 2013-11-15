#include <stdlib.h>
#include <string.h>

#include "../u_list.h"

#define TEST_SAMPLE_SIZE ((LIST_UNROLL_SIZE * 3) + (LIST_UNROLL_SIZE/2))
struct u_list *
ul_pushpage(struct u_list **head, int v)
{
	struct u_list *result;
	int i;
	for (i = 0; i < LIST_UNROLL_SIZE; ++i)
		result = ul_push(head, (void *)v);
	
	return result;
}

void
unittest_ul_push()
{
	uintptr_t buf[TEST_SAMPLE_SIZE + 1];
	uintptr_t j, offset;
	struct u_iterator i;
	struct u_list *list = 0;
	struct u_list *listAHead = 0;
	// linking test
	struct u_list *listA1,*listA2, *listA3, *listA4;
	listA1 = ul_pushpage(&listAHead, 1);
	assert(listAHead->next_page == listA1);
	assert(listAHead->prev_page == listA1);
	listA2 = ul_pushpage(&listAHead, 2);
	assert(listAHead == listA2 && listA2 != listA1);
	assert(listAHead->next_page == listA1);
	assert(listAHead->prev_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA2);
	listA3 = ul_pushpage(&listAHead, 3);
	assert(listAHead == listA3);
	assert(listAHead->next_page == listA2);
	assert(listAHead->prev_page == listA1);
	assert(listA2->prev_page == listA3);
	assert(listA2->next_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA3);
	listA4 = ul_pushpage(&listAHead, 4);
	assert(listAHead == listA4);
	assert(listAHead->next_page == listA3);
	assert(listAHead->prev_page == listA1);
	assert(listA3->prev_page == listA4);
	assert(listA3->next_page == listA2);
	assert(listA2->prev_page == listA3);
	assert(listA2->next_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA4);

	j = 4;
	_FXL_OUTER(i, listAHead, next_page)
		assert(i.node->value[0] == (void*)j--);

	assert(j == 0);

	_FXL_OUTER(i, listAHead->prev_page, prev_page)
		assert(i.node->value[0] == (void*)++j);
	
	assert(j == 4);
	// storage test

	memset(buf, 0, sizeof buf); buf[TEST_SAMPLE_SIZE] = 7777; // canary

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		ul_push(&list, (void*)j);

	// test forward traversal
	j = 0;
	FOR_X_IN_LIST(i, list)
		buf[j++] = (uintptr_t)UL_X(i);

	assert(buf[TEST_SAMPLE_SIZE] == 7777);

	offset = TEST_SAMPLE_SIZE - 1;
	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		assert(buf[j] == offset - (uintptr_t)j);

	// test reverse traversal
	memset(buf, 0, sizeof buf); buf[TEST_SAMPLE_SIZE] = 7777; // canary

	j = 0;
	FOR_X_IN_LIST_REVERSE(i, list)
		buf[j++] = (uintptr_t)UL_X(i);
	
	assert(buf[TEST_SAMPLE_SIZE] == 7777);

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		assert(buf[j] == j);
}

void
unittest_ul_append()
{
	uintptr_t expected[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19 };
	uintptr_t buf[sizeof expected / sizeof *expected];
	uintptr_t j;
	struct u_iterator i;
	struct u_list *list = 0;
	memset(buf, 0, sizeof buf);

	for (j = 11; j > 0; --j)
		ul_push(&list, (void*)(j-1));

	for (j = 11; j < 20; ++j)
		ul_append(&list, (void*)j);

	j = 0;
	FOR_X_IN_LIST(i, list){
		assert(j < sizeof buf / sizeof *buf);
		buf[j++] = (uintptr_t)UL_X(i);
	}
	assert(memcmp(buf, expected, sizeof buf) == 0);
}

struct u_list *
ul_appendpage(struct u_list **head, int v)
{
	struct u_list *result;
	int i;
	for (i = 0; i < LIST_UNROLL_SIZE; ++i)
		result = ul_append(head, (void *)v);

	return result;
}

void unittest_ul_append_linking()
{
	uintptr_t j, offset;
	struct u_iterator i;
	struct u_list *list = 0;
	struct u_list *listAHead = 0;
	// linking test
	struct u_list *listA1, *listA2, *listA3, *listA4;
	listA1 = ul_pushpage(&listAHead, 1);
	assert(listAHead->next_page == listA1);
	assert(listAHead->prev_page == listA1);
	listA2 = ul_pushpage(&listAHead, 2);
	assert(listAHead == listA2 && listA2 != listA1);
	assert(listAHead->next_page == listA1);
	assert(listAHead->prev_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA2);
	listA3 = ul_pushpage(&listAHead, 3);
	assert(listAHead == listA3);
	assert(listAHead->next_page == listA2);
	assert(listAHead->prev_page == listA1);
	assert(listA2->prev_page == listA3);
	assert(listA2->next_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA3);
	listA4 = ul_appendpage(&listAHead, 4);
	assert(listAHead != listA4);
	assert(listAHead == listA3);
	assert(listAHead->next_page == listA2);
	assert(listAHead->prev_page == listA4);
	assert(listA2->prev_page == listA3);
	assert(listA2->next_page == listA1);
	assert(listA1->prev_page == listA2);
	assert(listA1->next_page == listA4);
	assert(listA4->next_page == listA3);
	assert(listA4->prev_page == listA1);

	{
		uintptr_t expected[] = { 3, 2, 1, 4 };
		uintptr_t expected_r[] = { 4, 1, 2, 3 };

		j = 0;
		_FXL_OUTER(i, listAHead, next_page)
			assert(i.node->value[0] == (void*)expected[j++]);

		assert(j == 4);
		j = 0;
		_FXL_OUTER(i, listAHead->prev_page, prev_page)
			assert(i.node->value[0] == (void*)expected_r[j++]);

		assert(j == 4);
	}
}

void
unittest_ul_unlink()
{
	uintptr_t expected[7] = { 1, 2, 4, 5, 6, 8, 9 };
	uintptr_t buf[sizeof expected / sizeof *expected];
	uintptr_t j;
	struct u_iterator i;
	struct u_list *list = 0;
	memset(buf, 0, sizeof buf);

	ul_push(&list, (void*)8);
	ul_push(&list, (void*)7);
	ul_push(&list, (void*)6);
	ul_push(&list, (void*)5);
	ul_push(&list, (void*)4);
	ul_push(&list, (void*)3);
	ul_push(&list, (void*)2);
	ul_push(&list, (void*)1);
	ul_append(&list, (void*)9);

	ul_unlink(&list, (void*)3);
	ul_unlink(&list, (void*)7);

	j = 0;
	FOR_X_IN_LIST(i, list)
		buf[j++] = (uintptr_t)UL_X(i);

	assert(memcmp(buf, expected, sizeof buf) == 0);
}


void
unittest_ul_first()
{
	uintptr_t j;
	struct u_list *list = 0;
	uintptr_t x;
	uintptr_t y;

	for (j = 0; j < TEST_SAMPLE_SIZE; ++j)
		ul_push(&list, (void*)j);

	x = (uintptr_t) ul_first(list);
	y = TEST_SAMPLE_SIZE-1;

	assert(x == y);
}



void
unittest_u_list()
{
	//size_t size = sizeof(struct u_list);
	//assert(size == 64);

	unittest_ul_push();
	unittest_ul_append();
	unittest_ul_append_linking();
	unittest_ul_unlink();
	unittest_ul_first();
}
