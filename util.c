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

/*
 * return code of ncomp()
*/
enum ncomp_tag {
	COMP_EQUAL	= 0,
	COMP_GREATER	= 1,
	COMP_LESS	= -1
};

/*
 * return code of strccmp()
*/
enum strccmp_tag {
	MATCH		= 0,
	GREATER		= 1,
	LESS		= -1,
};


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

/* for public */
sec_t convsec(char *);
int strccmp(const char *, const char *);
int scomp(const void *, const void *);
int ncomp(const void *, const void *);
int rncomp(const void *, const void *);
FILE *xfopen(const char *, const char *);
int xfclose(FILE *);
char *xfgets(char *, int, FILE *);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(char *);
void xfree(void *);


/*----------------------------------------------------------------------------
 * convert character to second
 *----------------------------------------------------------------------------
*/
sec_t
convsec(char *src) {
	static char *base = NULL;
	static size_t maxlen = 0;

	sec_t sec;
	size_t len;
	char *begin;
	char *separator;

	len = strlen(src) + 1;
	if (base == NULL) {	/* at first */
		base = xmalloc(len);
		maxlen = len;
	}
	if (maxlen < len) {
		base = xrealloc(base, len);
		maxlen = len;
	}

	memcpy(base, src, len);
	begin = base;
	sec = 0;

	/*
	 * separate day and hour and minute and second part
	 *    ex) str=1+22:33:44
	 *        1, taking off "1 day" to conver to seconds.
	 *        2, taking off "22 hours" to conver to seconds.
	 *        3, taking off "33 mins" to conver to seconds.
	 *        4, taking off "44 secs".
	 *        last, adding all seconds.
	*/
	if ((separator = strchr(begin, '+')) != NULL) {
		*separator = 0;
		sec = 86400 * atoi(begin);
		begin = separator + 1;
	}

	*(begin + 2) = *(begin + 5) = 0;
	sec +=  3600 * atoi(begin);
	sec +=    60 * atoi(begin + 3);
	sec +=     1 * atoi(begin + 6);

	return sec;
}

/*----------------------------------------------------------------------------
 * strcmp with string length check
 *----------------------------------------------------------------------------
*/
int
strccmp(const char *s1, const char *s2) {
	size_t z1, z2;
	int cmp;

	z1 = strlen(s1);
	z2 = strlen(s2);

	if (z1 == z2) {
		return strcmp(s1, s2);
	}
	else if (z1 < z2) {
		if ((cmp = strncmp(s1, s2, z1)) == MATCH) {
			return LESS;
		}
		return cmp;
	}
	else {  /* if (z1 > z2) */
		if ((cmp = strncmp(s1, s2, z2)) == MATCH) {
			return GREATER;
		}
		return cmp;
	}

	return MATCH;
}

/*----------------------------------------------------------------------------
 * compare by string
 *----------------------------------------------------------------------------
*/
int
scomp(const void *p1, const void *p2) {
	char *s1, *s2;

	s1 = (char *)(Void(p1));
	s2 = (char *)(Void(p2));

	return strccmp(s1, s2);
}

/*----------------------------------------------------------------------------
 * compare by number
 *----------------------------------------------------------------------------
*/
int
ncomp(const void *p1, const void *p2) {
	int *n1, *n2;

	n1 = (int *)p1;
	n2 = (int *)p2;

	if (*n1 > *n2) {
		return COMP_GREATER;
	}
	else if (*n1 < *n2) {
		return COMP_LESS;
	}
	else {
		return COMP_EQUAL;
	}
}

/*----------------------------------------------------------------------------
 * compare by number (reverse)
 *----------------------------------------------------------------------------
*/
int
rncomp(const void *p1, const void *p2) {
	int *n1, *n2;

	n1 = (int *)p1;
	n2 = (int *)p2;

	if (*n1 > *n2) {
		return COMP_LESS;
	}
	else if (*n1 < *n2) {
		return COMP_GREATER;
	}
	else {
		return COMP_EQUAL;
	}
}


/*----------------------------------------------------------------------------
 * fopen driver
 *----------------------------------------------------------------------------
*/
FILE *
xfopen(const char *path, const char *mode) {
	FILE *file;

	if ((file = fopen(path, mode)) == NULL) {
		fprintf(stderr, "open failure (%s)(%s)\n", path, mode);
		if (debug)
			exit (1);
	}

	return (file);
}


/*----------------------------------------------------------------------------
 * fclose driver
 *----------------------------------------------------------------------------
*/
int
xfclose(FILE *file) {
	int rc;

	if ((rc = fclose(file)) != 0) {
		fprintf(stderr, "close failure\n");
		if (debug)
			exit (1);
	}

	return (rc);
}


/*----------------------------------------------------------------------------
 * fgets driver
 *----------------------------------------------------------------------------
*/
char *
xfgets(char *buff, int size, FILE *file) {
	char *c;

	memset(buff, 0, size);

	if ((c = fgets(buff, size, file)) == NULL) {
		if (ferror(file) > 0) {
			fprintf(stderr, "close failure\n");
			if (debug)
				exit (1);
		}
	}

	return (c);
}


/*----------------------------------------------------------------------------
 * malloc family driver
 *----------------------------------------------------------------------------
*/
void *
xmalloc(size_t size) {
	void *tmp;

	if (!size)
		return NULL;

	if ((tmp = malloc(size)) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		if (debug)
			exit (1);
		return NULL;
	}
	memset(tmp, 0, size);
	
	return (tmp);
}

void *
xrealloc(void *orig, size_t size) {
	void *tmp;

	if (orig == NULL)
		return NULL;
	if (!size)
		return orig;

	if ((tmp = realloc(orig, size)) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		if (debug)
			exit (1);
		return (orig);
	}

	return (tmp);
}

void
xfree(void *p) {
	free(p);
	return;
}


/*----------------------------------------------------------------------------
 * strdup driver
 *----------------------------------------------------------------------------
*/
char *
xstrdup(char *orig) {
	char *res;

	if (orig == NULL)
		return NULL;

	if ((res = strdup(orig)) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		if (debug)
			exit (1);
	}

	return res;
}


#ifdef DEBUG_UTIL
/*----------------------------------------------------------------------------
 * debug section
 *----------------------------------------------------------------------------
 *
 * following code is the driver for scomp() and so on.
 * if you want to these functions only, you can do "make util".
 *
*/

/*----------------------------------------------------------------------------
 * main
 *----------------------------------------------------------------------------
*/
int
main(int argc, char **argv) {
	exit(0);
}

#endif

/* end of source */
