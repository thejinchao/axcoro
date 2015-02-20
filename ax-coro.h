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

/*
	from lwan (https://github.com/lpereira/lwan)
	commit sha-1: b2f7cbfba00041074240ab11b717165adf554c77 
*/

/*
 * lwan - simple web server
 * Copyright (c) 2012 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __AXIA_COROUTINE_H__
#define __AXIA_COROUTINE_H__

typedef struct coro_t_			coro_t;
typedef struct coro_switcher_t_		coro_switcher_t;

typedef enum {
	CONN_CORO_ABORT = -1,
	CONN_CORO_MAY_RESUME = 0,
	CONN_CORO_FINISHED = 1
} lwan_connection_coro_yield_t;

typedef int(*coro_function_t)	(coro_t *coro);

coro_switcher_t *coro_switcher_new(void);
void			 coro_switcher_free(coro_switcher_t* switcher);

coro_t *coro_new(coro_switcher_t *switcher, coro_function_t function, void *data);
void	coro_free(coro_t *coro);

void    coro_reset(coro_t *coro, coro_function_t func, void *data);

int	coro_resume(coro_t *coro);
int	coro_resume_value(coro_t *coro, int value);
int	coro_yield(coro_t *coro, int value);

void   *coro_get_data(coro_t *coro);
int		coro_get_ended(coro_t *coro);

void    coro_defer(coro_t *coro, void (*func)(void *data), void *data);
void    coro_defer2(coro_t *coro, void (*func)(void *data1, void *data2),
            void *data1, void *data2);
void   *coro_malloc(coro_t *coro, size_t sz);
void   *coro_malloc_full(coro_t *coro, size_t size, void (*destroy_func)());
char   *coro_strdup(coro_t *coro, const char *str);
char   *coro_printf(coro_t *coro, const char *fmt, ...);

#define CORO_DEFER(fn)		((void (*)(void *))(fn))
#define CORO_DEFER2(fn)		((void (*)(void *, void *))(fn))

#endif
