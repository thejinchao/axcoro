/*
copy from lwan (https://github.com/lpereira/lwan)
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

#define _GNU_SOURCE
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include "ax-coro.h"

#if defined(__x86_64__)
#include <stdint.h>
typedef uintptr_t coro_context_t[10];
#elif defined(__i386__)
#include <stdint.h>
typedef uintptr_t coro_context_t[7];
#else
#include <ucontext.h>
typedef ucontext_t coro_context_t;
#endif

#define CORO_STACK_MIN		((3 * (PTHREAD_STACK_MIN)) / 2)

#define ALWAYS_INLINE inline __attribute__((always_inline))

#ifdef DISABLE_BRANCH_PREDICTION
#  define LIKELY_IS(x,y) (x)
#else
#  define LIKELY_IS(x,y)	__builtin_expect((x), (y))
#endif

#define LIKELY(x)	LIKELY_IS(!!(x), 1)
#define UNLIKELY(x)	LIKELY_IS((x), 0)

struct coro_switcher_t_ {
	coro_context_t caller;
	coro_context_t callee;
};

struct coro_t_ {
    coro_switcher_t *switcher;
    coro_context_t context;
    int yield_value;
    void *data;
    bool ended;
};

coro_switcher_t *coro_switcher_new(void)
{
	coro_switcher_t* switcher = malloc(sizeof(*switcher));
	memset((void*)switcher, 0, sizeof(*switcher));

	return switcher;
}

void coro_switcher_free(coro_switcher_t* switcher)
{
	assert(switcher);
	free(switcher);
}

/*
 * This swapcontext() implementation was obtained from glibc and modified
 * slightly to not save/restore the floating point registers, unneeded
 * registers, and signal mask.  It is Copyright (C) 2001, 2002, 2003 Free
 * Software Foundation, Inc and are distributed under GNU LGPL version 2.1
 * (or later).  I'm not sure if I can distribute them inside a GPL program;
 * they're straightforward so I'm assuming there won't be any problem; if
 * there is, I'll just roll my own.
 *     -- Leandro
 */
#if defined(__x86_64__)
void coro_swapcontext(coro_context_t *current, coro_context_t *other)
                __attribute__((noinline));
    asm(
    ".text\n\t"
    ".p2align 4\n\t"
    ".globl coro_swapcontext\n\t"
    "coro_swapcontext:\n\t"
    "mov    %rbx,0(%rdi)\n\t"
    "mov    %rbp,8(%rdi)\n\t"
    "mov    %r12,16(%rdi)\n\t"
    "mov    %r13,24(%rdi)\n\t"
    "mov    %r14,32(%rdi)\n\t"
    "mov    %r15,40(%rdi)\n\t"
    "mov    %rdi,48(%rdi)\n\t"
    "mov    %rsi,56(%rdi)\n\t"
    "mov    (%rsp),%rcx\n\t"
    "mov    %rcx,64(%rdi)\n\t"
    "lea    0x8(%rsp),%rcx\n\t"
    "mov    %rcx,72(%rdi)\n\t"
    "mov    72(%rsi),%rsp\n\t"
    "mov    0(%rsi),%rbx\n\t"
    "mov    8(%rsi),%rbp\n\t"
    "mov    16(%rsi),%r12\n\t"
    "mov    24(%rsi),%r13\n\t"
    "mov    32(%rsi),%r14\n\t"
    "mov    40(%rsi),%r15\n\t"
    "mov    48(%rsi),%rdi\n\t"
    "mov    64(%rsi),%rcx\n\t"
    "mov    56(%rsi),%rsi\n\t"
    "jmp    *%rcx\n\t");
#elif defined(__i386__)
void coro_swapcontext(coro_context_t *current, coro_context_t *other)
                __attribute__((noinline));
    asm(
    ".text\n\t"
    ".p2align 16\n\t"
    ".globl coro_swapcontext\n\t"
    "coro_swapcontext:\n\t"
    "movl   0x4(%esp),%eax\n\t"
    "movl   %ecx,0x1c(%eax)\n\t" /* ECX */
    "movl   %ebx,0x0(%eax)\n\t"  /* EBX */
    "movl   %esi,0x4(%eax)\n\t"  /* ESI */
    "movl   %edi,0x8(%eax)\n\t"  /* EDI */
    "movl   %ebp,0xc(%eax)\n\t"  /* EBP */
    "movl   (%esp),%ecx\n\t"
    "movl   %ecx,0x14(%eax)\n\t" /* EIP */
    "leal   0x4(%esp),%ecx\n\t"
    "movl   %ecx,0x18(%eax)\n\t" /* ESP */
    "movl   8(%esp),%eax\n\t"
    "movl   0x14(%eax),%ecx\n\t" /* EIP (1) */
    "movl   0x18(%eax),%esp\n\t" /* ESP */
    "pushl  %ecx\n\t"            /* EIP (2) */
    "movl   0x0(%eax),%ebx\n\t"  /* EBX */
    "movl   0x4(%eax),%esi\n\t"  /* ESI */
    "movl   0x8(%eax),%edi\n\t"  /* EDI */
    "movl   0xc(%eax),%ebp\n\t"  /* EBP */
    "movl   0x1c(%eax),%ecx\n\t" /* ECX */
    "ret\n\t");
#else
#define coro_swapcontext(cur,oth) swapcontext(cur, oth)
#endif

static void
coro_entry_point(coro_t *coro, coro_function_t func)
{
    int return_value = func(coro);
    coro->ended = true;
    coro_yield(coro, return_value);
}

void
coro_reset(coro_t *coro, coro_function_t func, void *data)
{
    unsigned char *stack = (unsigned char *)(coro + 1);

    coro->ended = false;
    coro->data = data;

#if defined(__x86_64__)
    coro->context[6 /* RDI */] = (uintptr_t) coro;
    coro->context[7 /* RSI */] = (uintptr_t) func;
    coro->context[8 /* RIP */] = (uintptr_t) coro_entry_point;
    coro->context[9 /* RSP */] = (uintptr_t) stack + CORO_STACK_MIN;
#elif defined(__i386__)
    /* Align stack and make room for two arguments */
    stack = (unsigned char *)((uintptr_t)(stack + CORO_STACK_MIN -
        sizeof(uintptr_t) * 2) & 0xfffffff0);

    uintptr_t *argp = (uintptr_t *)stack;
    *argp++ = 0;
    *argp++ = (uintptr_t)coro;
    *argp++ = (uintptr_t)func;

    coro->context[5 /* EIP */] = (uintptr_t) coro_entry_point;
    coro->context[6 /* ESP */] = (uintptr_t) stack;
#else
    getcontext(&coro->context);

    coro->context.uc_stack.ss_sp = stack;
    coro->context.uc_stack.ss_size = CORO_STACK_MIN;
    coro->context.uc_stack.ss_flags = 0;
    coro->context.uc_link = NULL;

    makecontext(&coro->context, (void (*)())coro_entry_point, 2, coro, func);
#endif
}

ALWAYS_INLINE coro_t *
coro_new(coro_switcher_t *switcher, coro_function_t function, void *data)
{
    coro_t *coro = malloc(sizeof(*coro) + CORO_STACK_MIN);
    if (!coro)
        return NULL;

    coro->switcher = switcher;
    coro_reset(coro, function, data);

    return coro;
}

ALWAYS_INLINE void *
coro_get_data(coro_t *coro)
{
    return LIKELY(coro) ? coro->data : NULL;
}

ALWAYS_INLINE int
coro_get_ended(coro_t *coro)
{
	return LIKELY(coro) ? (coro->ended ? 1 : 0) : 1;
}

ALWAYS_INLINE int
coro_resume(coro_t *coro)
{
    assert(coro);
    assert(coro->ended == false);

#if defined(__x86_64__) || defined(__i386__)
    coro_swapcontext(&coro->switcher->caller, &coro->context);
    if (!coro->ended)
        memcpy(&coro->context, &coro->switcher->callee,
                    sizeof(coro->context));
#else
    coro_context_t prev_caller;

    memcpy(&prev_caller, &coro->switcher->caller, sizeof(prev_caller));
    coro_swapcontext(&coro->switcher->caller, &coro->context);
    if (!coro->ended) {
        memcpy(&coro->context, &coro->switcher->callee,
                    sizeof(coro->context));
        memcpy(&coro->switcher->caller, &prev_caller,
                    sizeof(coro->switcher->caller));
    }
#endif

    return coro->yield_value;
}

ALWAYS_INLINE int
coro_yield(coro_t *coro, int value)
{
    assert(coro);
    coro->yield_value = value;
    coro_swapcontext(&coro->switcher->callee, &coro->switcher->caller);
    return coro->yield_value;
}

void
coro_free(coro_t *coro)
{
    assert(coro);
    free(coro);
}

