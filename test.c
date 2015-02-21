#include <stdio.h>
#include <string.h>
#include "ax-coro.h"

#define AX_CORO_RESUME		(0)
#define AX_CORO_FINISHED	(1)

static int 
function_a(coro_t* coro)
{
	void* data = coro_get_data(coro);

	for (int i = 0; i < 2; i++) {
		printf("[%d]function_a, arg=%s\n", i, (const char*)data);
		
		coro_yield(coro, AX_CORO_RESUME);
	}

	return AX_CORO_FINISHED;
}

static int
function_b(coro_t* coro)
{
	void* data = coro_get_data(coro);

	for (int i = 0; i < 5; i++) {
		printf("[%d]function_b, arg=%s\n", i, (const char*)data);

		coro_yield(coro, AX_CORO_RESUME);
	}

	return AX_CORO_FINISHED;
}


int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	coro_switcher_t* switcher = coro_switcher_new();
	coro_t *coro_a = coro_new(switcher, function_a, "foo");
	coro_t *coro_b = coro_new(switcher, function_b, "bar");

	do {
		int ret_a = coro_get_ended(coro_a) ? AX_CORO_FINISHED : coro_resume(coro_a);
		int ret_b = coro_get_ended(coro_b) ? AX_CORO_FINISHED : coro_resume(coro_b);

		if (ret_a == AX_CORO_FINISHED &&
			ret_b == AX_CORO_FINISHED) break;
	} while (1);


	coro_free(coro_a);
	coro_free(coro_b);
	return 0;
}


