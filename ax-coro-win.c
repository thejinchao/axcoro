#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <Windows.h>
#include "ax-coro.h"

struct coro_t_ {
	coro_switcher_t *switcher;
	void* fiber;
	int yield_value;
	coro_function_t func;
	void* data;
	bool ended;
};

struct coro_switcher_t_ {
	coro_t  primary_fiber;
	coro_t  *current;
	coro_t	*caller;
	coro_t	*callee;
};

coro_switcher_t *coro_switcher_new(void)
{
	coro_switcher_t* switcher = malloc(sizeof(*switcher));
	memset((void*)switcher, 0, sizeof(*switcher));

	/* create main fiber */
	void* primary_fiber = ConvertThreadToFiber(&(switcher->primary_fiber));

	switcher->primary_fiber.switcher = switcher;
	switcher->primary_fiber.fiber = primary_fiber;
	switcher->primary_fiber.func = 0;
	switcher->primary_fiber.data = 0;
	switcher->primary_fiber.ended = false;
	switcher->primary_fiber.yield_value = 0;

	switcher->current = primary_fiber;
	switcher->caller = 0;
	switcher->callee = 0;
	return switcher;
}

void coro_switcher_free(coro_switcher_t* switcher)
{
	assert(switcher);
	free(switcher);
}

static void __stdcall
coro_entry_point(void* param)
{
	coro_t* coro = (coro_t*)param;

	int return_value = coro->func(coro);

	coro->ended = true;
	coro_yield(coro, return_value);
}

void
coro_reset(coro_t *coro, coro_function_t func, void *data)
{
	coro->func = func;
	coro->data = data;
	coro->ended = false;
	coro->yield_value = 0;
}

coro_t* 
coro_new(coro_switcher_t *switcher, coro_function_t function, void *data)
{
	coro_t *coro = malloc(sizeof(*coro));
	if (!coro)
		return NULL;

	void* fiber = CreateFiber(0, coro_entry_point, coro);

	coro->switcher = switcher;
	coro->fiber = fiber;
	coro_reset(coro, function, data);

	return coro;
}

void *
coro_get_data(coro_t *coro)
{
	return coro ? coro->data : NULL;
}

int
coro_get_ended(coro_t *coro)
{
	return coro ? (coro->ended ? 1 : 0) : 1;
}

int
coro_resume(coro_t *coro)
{
	assert(coro);
	assert(coro->ended == false);

	void* prev_caller = coro->switcher->caller;
	coro->switcher->caller = coro->switcher->current;
	coro->switcher->current = coro->fiber;

	SwitchToFiber(coro->fiber);

	coro->switcher->caller = prev_caller;
	return coro->yield_value;
}

int
coro_resume_value(coro_t *coro, int value)
{
	assert(coro);
	assert(coro->ended == false);

	coro->yield_value = value;
	return coro_resume(coro);
}


int
coro_yield(coro_t *coro, int value)
{
	assert(coro);
	coro->yield_value = value;
	coro->switcher->callee = coro->switcher->current;
	coro->switcher->current = coro->switcher->caller;

	SwitchToFiber(coro->switcher->caller);

	return coro->yield_value;
}

void
coro_free(coro_t *coro)
{
	assert(coro);
	free(coro);
}

