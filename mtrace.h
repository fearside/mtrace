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

/*-----------------------------------------------------------------------------
 * include
 *-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

/*
 * external library
*/
#include "getlog.h"


/*-----------------------------------------------------------------------------
 * global variable
 *-----------------------------------------------------------------------------
*/
extern int debug;


/*-----------------------------------------------------------------------------
 * typedef
 *-----------------------------------------------------------------------------
*/
typedef unsigned long int count_t;		/* counter */
typedef unsigned int sort_t;			/* offset for sorted key */
typedef unsigned long int sec_t;		/* second */
typedef unsigned int day_t;			/* day */

typedef int cmp_t(const void *, const void *);	/* for msort() */


/*-----------------------------------------------------------------------------
 * macro / constant value
 *-----------------------------------------------------------------------------
*/

/*
 * for msort()
*/
#define Void(a) *((void **)(a))

/*
 * for calculating offset of structure
*/
#define OFFSET(type, field) ((unsigned int)&(((type *)NULL)->field))



/*-----------------------------------------------------------------------------
 * function
 *-----------------------------------------------------------------------------
*/

/* msort.c */

/* util.c */
extern sec_t convsec(char *);
extern int strccmp(const char *, const char *);
extern int scomp(const void *, const void *);
extern int ncomp(const void *, const void *);
extern int rncomp(const void *, const void *);
extern FILE *xfopen(const char *, const char *);
extern int xfclose(FILE *);
extern char *xfgets(char *, int, FILE *);
extern char *offbracket(char *, int);

extern void *xmalloc(size_t);
extern void *xrealloc(void *, size_t);
extern char *xstrdup(char *);
extern void xfree(void *);


/* end of header */
