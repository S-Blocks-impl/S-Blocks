// -*- c-basic-offset: 4; related-file-name: "../include/click/glue.hh" -*-
/*
 * glue.{cc,hh} -- minimize portability headaches, and miscellany
 * Robert Morris, Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2007 Regents of the University of California
 * Copyright (c) 2008 Mazu Networks, Inc.
 * Copyright (c) 2008-2011 Meraki, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

// #include <click/config.h>

// #include <click/glue.hh>
// #include <click/timestamp.hh>
// #include <click/error.hh>

// #if CLICK_USERLEVEL || CLICK_MINIOS
// # include <stdarg.h>
// # include <unistd.h>
// # include <sys/types.h>
// # include <sys/stat.h>
// # if CLICK_USERLEVEL
// #  include <fcntl.h>
// # endif
// #elif CLICK_LINUXMODULE
// # if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
// #  include <click/cxxprotect.h>
// CLICK_CXX_PROTECT
// #  include <linux/vmalloc.h>
// CLICK_CXX_UNPROTECT
// #  include <click/cxxunprotect.h>
// # endif
// #elif CLICK_BSDMODULE
// # include <click/cxxprotect.h>
// CLICK_CXX_PROTECT
// # include <sys/malloc.h>
// CLICK_CXX_UNPROTECT
// # include <click/cxxunprotect.h>
// #endif

// Include header structures so we can check their sizes with static_assert.
// #include <clicknet/ether.h>
// #include <clicknet/fddi.h>
// #include <clicknet/ip.h>
// #include <clicknet/ip6.h>
// #include <clicknet/icmp.h>
// #include <clicknet/tcp.h>
// #include <clicknet/udp.h>
// #include <clicknet/rfc1483.h>

// Allocate space for static constants from integer_traits.



// SORTING

// This reimplementation of click_qsort is a version of
// Engineering a Sort Function, Jon L. Bentley and M. Douglas McIlroy,
// Software---Practice & Experience, 23(11), 1249-1265, Nov. 1993
#include "../include/config.h"
#include "glue.h"
#include <stdarg.h>
#include "./error.h"

namespace {

typedef long cq_word_t;
#define CQ_WORD_SIZE		sizeof(cq_word_t)
#define cq_exch(a, b, t)	(t = a, a = b, b = t)
#define cq_swap(a, b)						\
    (swaptype ? cq_swapfunc(a, b, size, swaptype)		\
     : (void) cq_exch(*(cq_word_t *) (a), *(cq_word_t *) (b), swaptmp))

static char *
cq_med3(char *a, char *b, char *c,
	int (*compar)(const void *, const void *, void *), void *thunk)
{
    int ab = compar(a, b, thunk);
    int bc = compar(b, c, thunk);
    if (ab < 0)
	return (bc < 0 ? b : (compar(a, c, thunk) < 0 ? c : a));
    else
	return (bc > 0 ? b : (compar(a, c, thunk) > 0 ? c : a));
}

static void
cq_swapfunc(char *a, char *b, size_t n, int swaptype)
{
    if (swaptype <= 1) {
	cq_word_t t;
	for (; n; a += CQ_WORD_SIZE, b += CQ_WORD_SIZE, n -= CQ_WORD_SIZE)
	    cq_exch(*(cq_word_t *) a, *(cq_word_t *) b, t);
    } else {
	char t;
	for (; n; ++a, ++b, --n)
	    cq_exch(*a, *b, t);
    }
}

}


#define CQ_STACKSIZ	((8 * SIZEOF_SIZE_T + 1) * 2)

int
click_qsort(void *base, size_t n, size_t size,
	    int (*compar)(const void *, const void *, void *), void *thunk)
{
    int swaptype;
    if ((((char *) base - (char *) 0) | size) % CQ_WORD_SIZE)
	swaptype = 2;
    else
	swaptype = (size > CQ_WORD_SIZE ? 1 : 0);
    cq_word_t swaptmp;

    size_t stackbuf[CQ_STACKSIZ];
    size_t *stack = stackbuf;
    *stack++ = 0;
    *stack++ = n;

    while (stack != stackbuf) {
	stack -= 2;
	char *a = (char *) base + stack[0] * size;
	n = stack[1] - stack[0];

	// insertion sort for tiny arrays
	if (n < 7) {
	    for (char *pi = a + size; pi < a + n * size; pi += size)
		for (char *pj = pi;
		     pj > a && compar(pj - size, pj, thunk) > 0;
		     pj -= size)
		    cq_swap(pj, pj - size);
	    continue;
	}

	// find a pivot
	char *pm = a + (n / 2) * size;
	if (n > 7) {
	    char *p1 = a;
	    char *p2 = a + (n - 1) * size;
	    if (n > 40) {     /* Big arrays, pseudomedian of 9 */
		size_t offset = (n / 8) * size;
		p1 = cq_med3(p1, p1 + offset, p1 + 2*offset, compar, thunk);
		pm = cq_med3(pm - offset, pm, pm + offset, compar, thunk);
		p2 = cq_med3(p2 - 2*offset, p2 - offset, p2, compar, thunk);
	    }
	    pm = cq_med3(p1, pm, p2, compar, thunk);
	}

	// 2009.Jan.21: A tiny change makes the sort complete even with a
	// bogus comparator, such as "return 1;".  Guarantee that "a" holds
	// the pivot.  This means we don't need to compare "a" against the
	// pivot explicitly.  (See initialization of "pa = pb = a + size".)
	// Subdivisions will thus never include the pivot, even if "cmp(pivot,
	// pivot)" returns nonzero.  We will thus never run indefinitely.
	cq_word_t pivottmp;
	char *pivot;
	if (swaptype)
	    pivot = a;
	else
	    pivot = (char *) &pivottmp, pivottmp = *(cq_word_t *) pm;
	cq_swap(a, pm);

	// partition
	char *pa, *pb, *pc, *pd;
	pa = pb = a + size;
	pc = pd = a + (n - 1) * size;
	int r;
	while (1) {
	    while (pb <= pc && (r = compar(pb, pivot, thunk)) <= 0) {
		if (r == 0) {
		    cq_swap(pa, pb);
		    pa += size;
		}
		pb += size;
	    }
	    while (pc >= pb && (r = compar(pc, pivot, thunk)) >= 0) {
		if (r == 0) {
		    cq_swap(pc, pd);
		    pd -= size;
		}
		pc -= size;
	    }
	    if (pb > pc)
		break;
	    cq_swap(pb, pc);
	    pb += size;
	    pc -= size;
	}

	// swap the extreme ranges, which are equal to the pivot, into the
	// middle
	char *pn = a + n * size;
	size_t s = (pa - a < pb - pa ? pa - a : pb - pa);
	if (s)
	    cq_swapfunc(a, pb - s, s, swaptype);
	size_t pd_minus_pc = pd - pc;
	s = (pd_minus_pc < pn - pd - size ? pd_minus_pc : pn - pd - size);
	if (s)
	    cq_swapfunc(pb, pn - s, s, swaptype);

	// mark subranges to sort
	if (pb == pa && pd == pc)
	    continue;
	assert(stack + 4 < stackbuf + CQ_STACKSIZ);
	stack[2] = stack[1] - (pd - pc) / size;
	stack[3] = stack[1];
	stack[1] = stack[0] + (pb - pa) / size;
	// Push stack items biggest first.  This limits the stack size to
	// log2 MAX_SIZE_T!  Optimization in Hoare's original paper, suggested
	// by Sedgewick in his own qsort implementation paper.
	if (stack[3] - stack[2] > stack[1] - stack[0]) {
	    size_t tx;
	    tx = stack[0], stack[0] = stack[2], stack[2] = tx;
	    tx = stack[1], stack[1] = stack[3], stack[3] = tx;
	}
	stack += (stack[2] != stack[3] ? 4 : 2);
    }

    return 0;
}

int
click_qsort(void *base, size_t n, size_t size, int (*compar)(const void *, const void *))
{
    int (*compar2)(const void *, const void *, void *);
    compar2 = reinterpret_cast<int (*)(const void *, const void *, void *)>(compar);
    return click_qsort(base, n, size, compar2, 0);
}

extern "C" {
void
click_chatter(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  char buf[512];
  memset(buf,'\0',sizeof(char)*512);

  if (ErrorHandler *errh = ErrorHandler::default_handler()) {
    errh->xmessage(ErrorHandler::e_info, fmt, val);
  } else {
// #if CLICK_LINUXMODULE
// # if __MTCLICK__
//     static char buf[NR_CPUS][512];	// XXX
//     click_processor_t cpu = click_get_processor();
//     int i = vsnprintf(buf[cpu], 512, fmt, val);
//     printk(KERN_ALERT "%.*s\n", i, buf[cpu]);
//     click_put_processor();
// # else
//     static char buf[512];		// XXX
//     int i = vsnprintf(buf, 512, fmt, val);
//     printk(KERN_ALERT "%.*s\n", i, buf);
// # endif
// #elif CLICK_BSDMODULE
//     vprintf(fmt, val);
// #else /* User-space */
    // vfprintf(stderr, fmt, val);
    // fprintf(stderr, "\n");
// #endif
  	vsprintf(buf, fmt, val);
  	ocall_print(String(buf).data());
  }

  va_end(val);
}
}


