/*
 * Copyright (c) 2014, Tsuyoshi Tanai <skmt.japan@gmail.com>,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE. 
*/

/*----------------------------------------------------------------------------
 * include file
 *----------------------------------------------------------------------------
 */
#include "mtrace.h"


/*----------------------------------------------------------------------------
 * macro
 *----------------------------------------------------------------------------
 */
#define SWAP(t, s, o) \
{ \
	void **n; \
	n = t + o; \
	*n = s; \
	t = s; \
}


/*----------------------------------------------------------------------------
 * type definition
 *----------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 * global variable
 *----------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 * prototype
 *----------------------------------------------------------------------------
 */
static void *mergelist(void *, void *, sort_t, sort_t, cmp_t *);

/* for public */
void *msort(void *, sort_t, sort_t, cmp_t *);


/*----------------------------------------------------------------------------
 * merge list
 *----------------------------------------------------------------------------
 */
void *
mergelist(void *a, void *b, sort_t offset, sort_t key, cmp_t func)
{
	void *p;	/* working structure's pointer */
	void **q;	/* working structure's pointer for remainder */
	void *base;	/* base structure */
	int cmp;

	base = NULL;

	if (a == NULL || b == NULL)
		fprintf(stderr, "invalid data\n");

	/*
	 * NOTICE: compare a with b under following condition,
	 *   called comparison function as "func" treads...
	 *   1. string data
	 *      "a" and "b" is bouble pointer as (void **)a.
	 *   2. numeric data
	 *      "a" and "b" is single pointer as (void *)a.
	 */
	while (a != NULL && b != NULL) {
		cmp = (*func)((const void *)(a + key), (const void *)(b + key));
		if (cmp <= 0) {
			/* a <= b */
			base == NULL ? (base = p = a) : (SWAP(p, a, offset));
			a = Void(a + offset);	/* a = a->next */
		}
		else {
			/* a > b */
			base == NULL ? (base = p = b) : (SWAP(p, b, offset));
			b = Void(b + offset);	/* b = b->next */
		}
	}

	/*
	 * joint a remainder
	 */
	q = p + offset;		/* p->next */
	if (a)
		*q = a;		/* p->next = a */
	else if (b)
		*q = b;		/* p->next = b */
	else
		fprintf(stderr, "mismatching sort\n");

	return base;
}


/*----------------------------------------------------------------------------
 * general sort function for linked-list
 *----------------------------------------------------------------------------
 */
void *
msort(void *begin, sort_t offset, sort_t key, cmp_t function)
{
	void *half;		/* middle point */
	void *last;		/* end point */
	void *new;		/* half+1 point */

	if (begin == NULL || Void(begin + offset) == NULL) {
		return begin;
	}

	/*
	 * find half point
	 */
	half = begin;
	last = Void(begin + offset);		/* last = begin->next */

	if (last) {
		last = Void(last + offset);	/* last = last->next */
	}

	while (last) {
		half = Void(half + offset);	/* half = half->next */
		last = Void(last + offset);	/* last = last->next */
		if (last) {
			last = Void(last + offset);
		}
	}

	/*
	 * set NULL pointer for "half->next" to divide list into halves
	 */
	{
		void **p;
		new = Void(half + offset);	/* new = half->next; */
		p = half + offset;		/* half->next = NULL */
		*p = NULL;
	}

	return mergelist(msort(begin, offset, key, function),
			 msort(new, offset, key, function),
			 offset,
			 key,
			 function);
}


#ifdef DEBUG_MSORT
/*----------------------------------------------------------------------------
 * debug section
 *----------------------------------------------------------------------------
 *
 * the following code is a driver for msort().
 * if you want to test msort() only, you can do "make msort".
 *
 */


/*
 * constant number
 */
enum {
	END		= 999,
	MATCH		= 1,
	MISMATCH	= 0
};


/*
 * test structure (1)
 *    link-list pointer is middle of struct
 */
typedef struct _dummy1 {
	struct _dummy1 *prev;
	struct _dummy1 *next;
	char *key;
	int value;
} Dummy1;

/*
 * test structure (2)
 *    link-list pointer is last of struct
 */
typedef struct _dummy2 {
	char *key;
	int value;
	struct _dummy2 *prev;
	struct _dummy2 *next;
} Dummy2;

/*
 * test structure (3)
 *    link-list pointer is head of struct
 */
typedef struct _dummy3 {
	struct _dummy3 *next;
	struct _dummy3 *prev;
	char *key;
	int value;
} Dummy3;

/*
 * test structure (4)
 *    link-list pointer is middle of struct
 */
typedef struct _dummy4 {
	struct _dummy4 *prev;
	struct _dummy4 *next;
	int key;
	int value;
} Dummy4;

/*
 * test structure (5)
 *    link-list pointer is last of struct
 */
typedef struct _dummy5 {
	int key;
	int value;
	struct _dummy5 *prev;
	struct _dummy5 *next;
} Dummy5;

/*
 * test structure (6)
 *    link-list pointer is head of struct
 */
typedef struct _dummy6 {
	struct _dummy6 *next;
	struct _dummy6 *prev;
	int key;
	int value;
} Dummy6;


/*----------------------------------------------------------------------------
 * compare dummy1-3
 *----------------------------------------------------------------------------
 */
int
comp123(d1, d2, d3)
	char *d1;
	char *d2;
	char *d3;
{
	int cmp1;	/* compare with D1 and D2 */
	int cmp2;	/* compare with D2 and D3 */
	int cmp3;	/* compare with D1 and D3 */

	cmp1 = strcmp(d1, d2);
	cmp2 = strcmp(d2, d3);
	cmp3 = strcmp(d1, d3);

	if (cmp1 == 0 && cmp2 == 0 && cmp3 == 0) {
		return MATCH;
	}

	return MISMATCH;
}

/*----------------------------------------------------------------------------
 * compare dummy4-6
 *----------------------------------------------------------------------------
 */
int
comp456(d1, d2, d3)
	int d1;
	int d2;
	int d3;
{
	int cmp1;	/* compare with D1 and D2 */
	int cmp2;	/* compare with D2 and D3 */
	int cmp3;	/* compare with D1 and D3 */

	cmp1 = d1 - d2;
	cmp2 = d2 - d3;
	cmp3 = d1 - d3;

	if (cmp1 == 0 && cmp2 == 0 && cmp3 == 0) {
		return MATCH;
	}

	return MISMATCH;
}


/*----------------------------------------------------------------------------
 * check table 1-3
 *----------------------------------------------------------------------------
 */
void
check123(elem)
	unsigned int elem[];
{
	Dummy1 dummy1, *p1, *new1;
	Dummy2 dummy2, *p2, *new2;
	Dummy3 dummy3, *p3, *new3;

	int i;
	char buff[11];
	sort_t n1, k1, n2, k2, n3, k3;

	p1 = &dummy1;
	p2 = &dummy2;
	p3 = &dummy3;

	/*
	 * set
	*/
	memset(buff, 0, sizeof(buff));
	for (i = 0; elem[i] != END; ++i) {
		memset(buff, (int)(elem[i]+0x30), (sizeof(buff) - 1));
		new1 = xmalloc(sizeof(Dummy1));
		new1->key = xstrdup(buff);
		p1->next = new1;
		p1 = new1;

		memset(buff, (int)(elem[i]+0x30), (sizeof(buff) - 1));
		new2 = xmalloc(sizeof(Dummy2));
		new2->key = xstrdup(buff);
		p2->next = new2;
		p2 = new2;

		memset(buff, (int)(elem[i]+0x30), (sizeof(buff) - 1));
		new3 = xmalloc(sizeof(Dummy3));
		new3->key = xstrdup(buff);
		p3->next = new3;
		p3 = new3;
	}

	/*
	 * sort by scomp
	*/
	n1 = OFFSET(Dummy1, next);
	k1 = OFFSET(Dummy1, key);
	dummy1.next = msort(dummy1.next, n1, k1, scomp);

	n2 = OFFSET(Dummy2, next);
	k2 = OFFSET(Dummy2, key);
	dummy2.next = msort(dummy2.next, n2, k2, scomp);

	n3 = OFFSET(Dummy3, next);
	k3 = OFFSET(Dummy3, key);
	dummy3.next = msort(dummy3.next, n3, k3, scomp);

	/*
	 * compare
	*/
	for (p1 = dummy1.next, p2 = dummy2.next, p3 = dummy3.next;
	     p1 != NULL;
	     p1 = p1->next, p2 = p2->next, p3 = p3->next) {
		if (comp123(p1->key, p2->key, p3->key) == MISMATCH) {
			fprintf(stderr, "p1(%s), p2(%s), p3(%s)\n",
				p1->key, p2->key, p3->key);
			abort();
		}
	}

	return;
}

/*----------------------------------------------------------------------------
 * check table 4-6
 *----------------------------------------------------------------------------
 */
void
check456(elem)
	unsigned int elem[];
{
	Dummy4 dummy4, *p4, *new4;
	Dummy5 dummy5, *p5, *new5;
	Dummy6 dummy6, *p6, *new6;

	int i;
	sort_t n4, k4, n5, k5, n6, k6;

	p4 = &dummy4;
	p5 = &dummy5;
	p6 = &dummy6;

	/*
	 * set
	 */
	for (i = 0; elem[i] != END; ++i) {
		new4 = xmalloc(sizeof(Dummy4));
		new4->key = elem[i];
		p4->next = new4;
		p4 = new4;

		new5 = xmalloc(sizeof(Dummy5));
		new5->key = elem[i];
		p5->next = new5;
		p5 = new5;

		new6 = xmalloc(sizeof(Dummy6));
		new6->key = elem[i];
		p6->next = new6;
		p6 = new6;
	}

	/*
	 * sort by ncomp
	 */
	n4 = OFFSET(Dummy4, next);
	k4 = OFFSET(Dummy4, key);
	dummy4.next = msort(dummy4.next, n4, k4, ncomp);

	n5 = OFFSET(Dummy5, next);
	k5 = OFFSET(Dummy5, key);
	dummy5.next = msort(dummy5.next, n5, k5, ncomp);

	n6 = OFFSET(Dummy6, next);
	k6 = OFFSET(Dummy6, key);
	dummy6.next = msort(dummy6.next, n6, k6, ncomp);

	/*
	 * compare
	 */
	for (p4 = dummy4.next, p5 = dummy5.next, p6 = dummy6.next;
	     p4 != NULL;
	     p4 = p4->next, p5 = p5->next, p6 = p6->next) {
		if (comp456(p4->key, p5->key, p6->key) == MISMATCH) {
			fprintf(stderr, "p4(%d), p5(%d), p6(%d)\n",
				p4->key, p5->key, p6->key);
			abort();
		}
	}

	return;
}


/*----------------------------------------------------------------------------
 * main
 *----------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
	unsigned int **pelem;
	unsigned int elem0[] = { 0, END };
	unsigned int elem1[] = { 9, END };
	unsigned int elem2[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, END };
	unsigned int elem3[] = { 3, 5, 9, 8, 4, 7, 2, 1, 6, 0, END };
	unsigned int elem4[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, END };
	unsigned int elem5[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, END };
	unsigned int elem6[] = { 1, 3, 2, 4, 5, END };

	pelem[0] = elem0;
	pelem[1] = elem1;
	pelem[2] = elem2;
	pelem[3] = elem3;
	pelem[4] = elem4;
	pelem[5] = elem5;
	pelem[6] = elem6;
	pelem[7] = NULL;

	for(; *pelem != (unsigned int *)NULL; ++pelem) {
		check123(*pelem);	/* sort by scomp */
		check456(*pelem);	/* sort by ncomp */
	}

	exit(0);
}

#endif

/* end of source */
