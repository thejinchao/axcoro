/*
	The MIT License (MIT)
	
	Copyright (c) 2015 thecodeway.com
	
	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __AXIA_COROUTINE_H__
#define __AXIA_COROUTINE_H__

typedef struct coro_t_				coro_t;
typedef struct coro_switcher_t_		coro_switcher_t;
typedef int(*coro_function_t)		(coro_t *coro);

coro_switcher_t *coro_switcher_new(void);
void			 coro_switcher_free(coro_switcher_t* switcher);

coro_t *coro_new(coro_switcher_t *switcher, coro_function_t function, void *data);
void	coro_free(coro_t *coro);

int		coro_resume(coro_t *coro);
int		coro_yield(coro_t *coro, int value);

void   *coro_get_data(coro_t *coro);
int		coro_get_ended(coro_t *coro);

#endif
