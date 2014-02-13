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
#include "getlog.h"

#include <ctype.h>
#include <string.h>



/*----------------------------------------------------------------------------
 * macro
 *----------------------------------------------------------------------------
*/


/*----------------------------------------------------------------------------
 * global variable
 *----------------------------------------------------------------------------
*/

/*
 * main
*/
#define MAXBUFSZ	4096
#define MAXFIELDNUM	1024
static char *log	= NULL;		/* original log */
static char *slog	= NULL;		/* buffer copy for splitting */
static char **field	= NULL;		/* field table */
static int nfield	= 0;		/* number of field */
static size_t lsize	= 0;
static size_t fsize	= 0;
static int fnum         = 0;

static char *sm_field_to[SM_FIELD_TO];
static char *sm_field[SM_FIELD];


/*
 * const separator for upper version 8.11 sendmail
*/
static char * const Separator[] = {
	" ",	/* HEAD_MONTH */
	" ",	/* HEAD_DAY */
	" ",	/* HEAD_TIME */
	" ",	/* HEAD_HOSTNAME */
	" ",	/* HEAD_IDENT */
	" ",	/* HEAD_SYSLOG1 */
	" ",	/* HEAD_SYSLOG2 */
	" ",	/* HEAD_SYSLOG3 */
	" ",	/* HEAD_QID */
	",",	/* BODY_ADDR */
	",",	/* BODY_NO11 */
	",",	/* BODY_NO12 */
	",",	/* BODY_NO13 */
	",",	/* BODY_NO14 */
	",",	/* BODY_NO15 */
	",",	/* BODY_NO16 */
	",",	/* BODY_NO17 */
	",",	/* BODY_NO18 */
	NULL 	/* terminator */
};



/*----------------------------------------------------------------------------
 * prototype
 *----------------------------------------------------------------------------
*/
extern void *xrealloc(void *, size_t);
extern void *xmalloc(size_t);
extern void *xfree(void *);
extern char *xstrdup(char *);


/* for local */
static int offseparator(char *, int);
static int set_tail(int);
static char *offbracket(char *, int, int);

static void set_smfield_to(char *);
static int issplit(char *, int);
static void split(char *);
static void expand_field(void);
static int expand_log(void);
static void store_smfield(char *, int);
static void clear_smfield();


/* for public */
char *get_smfield(int);
char *get_smfield_to(int);

int init_getlog(void);
int getnfield(void);
char *getfield(int);
char *getlog(FILE *, off_t *);


/*----------------------------------------------------------------------------
 * off separator
 *----------------------------------------------------------------------------
*/
int
offseparator(char *p, int separator) {
	char *tmp;
	size_t len;

	if (p == NULL)
		return (-1);

	len = strlen(p);
	if (p[len - 1] == separator)
		p[len - 1] = '\0';
	else {
		if ((tmp = strrchr(p, separator)) != NULL)
			*tmp = '\0';
		else
			return (-1);
	}

	return (0);
}


/*----------------------------------------------------------------------------
 * off bracket
 *----------------------------------------------------------------------------
*/
int
set_tail(int c) {
	switch (c) {
	case '<':
		return '>';
		break;
	case '[':
		return ']';
		break;
	case '(':
		return ')';
		break;
	default:
		return (0);
	}
}

char *
offbracket(char *p, int bracket, int separator) {
	static char new[BUFSIZ];
	size_t len;
	char *left;
	char *right;
	char *tmp;
	int tail;

	if (p == NULL || bracket == '\0' || separator == '\0')
		return (p);
	if ((len = strlen(p)) >= sizeof(new))
		return (p);

	memset(new, 0, sizeof(new));
	strncpy(new, p, len);
	offseparator(new, separator);

	if ((tail = set_tail(bracket)) == 0)
		return (new);

	left = new;
	while ((tmp = strchr(left, bracket)) != NULL) {
		left = tmp + 1;
	}
	if (left == new)
		return (new);

	if ((right = strchr(left, tail)) != NULL)
		*right = '\0';
	else
		return (new);

	return (left);
}


/*----------------------------------------------------------------------------
 * parse sendmail'log
 *----------------------------------------------------------------------------
*/
void
set_smfield_to(char *orig) {
	char *p, *q, *buff;
	int i;

	if (!orig)
		return;

	buff = p = xstrdup(orig);
	for (i = 0; (q = strchr(p, COMMA)) != NULL; p = q + 1, ++i) {
		*q = '\0';
		sm_field_to[i] = xstrdup(offbracket(p, '<', COMMA));
	}
	sm_field_to[i] = xstrdup(offbracket(p, '<', COMMA));

	xfree(buff);
	return;
}


void
store_smfield(char *p, int i) {
	size_t len;
	
	if (p == NULL)
		return;
		
	len = strlen(p);
	if (i ==0 && len == 3) {
		if (strcmp(p, "Jan") == 0 ||
		    strcmp(p, "Feb") == 0 ||
		    strcmp(p, "Mur") == 0 ||
		    strcmp(p, "Apr") == 0 ||
		    strcmp(p, "May") == 0 ||
		    strcmp(p, "Jun") == 0 ||
		    strcmp(p, "Jul") == 0 ||
		    strcmp(p, "Aug") == 0 ||
		    strcmp(p, "Sep") == 0 ||
		    strcmp(p, "Oct") == 0 ||
		    strcmp(p, "Nov") == 0 ||
		    strcmp(p, "Dec"))
		    	sm_field[SM_MONTH] = xstrdup(p);
	}
	else if (i == 1) {
		switch (len) {
		case 2:
			if (!isdigit((int)p[1]))
				break;
		case 1:
			if (!isdigit((int)p[0]))
				break;
			sm_field[SM_DAY] = xstrdup(p);
		default:
			break;
		}
	}
	else if (i == 2 && len == 8) {
		if (p[2] == ':' && p[5] == ':' && 
		    isdigit((int)p[0]) && isdigit((int)p[1]) &&
		    isdigit((int)p[3]) && isdigit((int)p[4]) &&
		    isdigit((int)p[6]) && isdigit((int)p[7]))
		    	sm_field[SM_TIME] = xstrdup(p);
	}
	else if (i == 3) {
		sm_field[SM_HOSTNAME] = xstrdup(p);
	}
	else if (i == 4) {
		sm_field[SM_SYSLOGID] = xstrdup(p);
	}
	else if (p[len - 1] == ':' && isalpha((int)p[0]) && isdigit((int)p[1])) {
		sm_field[SM_QID] = xstrdup(p);
	}

	if (strncmp(p, "from=", 5) == 0) {
		char *q = offbracket(p + 5, '<', ',');
		if (strlen(q) > 0)
			sm_field[SM_FROM] = xstrdup(q);
		else
			sm_field[SM_FROM] = xstrdup("NULL-SENDER");
	}
	else if (strncmp(p, "size=", 5) == 0)
		sm_field[SM_SIZE] = xstrdup(p + 5);
	else if (strncmp(p, "class=", 6) == 0)
		sm_field[SM_CLASS] = xstrdup(p + 6);
	else if (strncmp(p, "nrcpts=", 7) == 0)
		sm_field[SM_NRCPTS] = xstrdup(p + 7);
	else if (strncmp(p, "msgid=", 6) == 0)
		sm_field[SM_MSGID] = xstrdup(offbracket(p + 6, '<', ','));
	else if (strncmp(p, "relay=", 6) == 0)
		sm_field[SM_RELAY] = xstrdup(p + 6);
	else if (strncmp(p, "to=", 3) == 0) {
		sm_field[SM_TO] = xstrdup(p + 3);
		set_smfield_to(p + 3);
	}
	else if (strncmp(p, "ctladdr=", 3) == 0)
		sm_field[SM_CTLADDR] = xstrdup(p + 3);
	else if (strncmp(p, "delay=", 6) == 0)
		sm_field[SM_DELAY] = xstrdup(p + 6);
	else if (strncmp(p, "xdelay=", 7) == 0)
		sm_field[SM_XDELAY] = xstrdup(p + 7);
	else if (strncmp(p, "mailer=", 7) == 0)
		sm_field[SM_MAILER] = xstrdup(p + 7);
	else if (strncmp(p, "pri=", 4) == 0)
		sm_field[SM_PRI] = xstrdup(p + 4);
	else if (strncmp(p, "DSN=", 4) == 0)
		sm_field[SM_DSN] = xstrdup(p + 4);
	else if (strncmp(p, "stat=", 5) == 0)
		sm_field[SM_STAT] = xstrdup(p + 5);
		
	return;
}

void
clear_smfield() {
	int i;
	for (i = 0; i < SM_FIELD; ++i) {
		if (sm_field[i] != NULL)
			xfree(sm_field[i]);
	}
	memset(sm_field, 0, sizeof(sm_field));

	for (i = 0; i < SM_FIELD_TO; ++i) {
		if (sm_field_to[i] != NULL)
			xfree(sm_field_to[i]);
	}
	memset(sm_field_to, 0, sizeof(sm_field_to));

	return;
}

char *
get_smfield(int index) {
	return (sm_field[index]);
}

char *
get_smfield_to(int index) {
	return (sm_field_to[index]);
}


/*----------------------------------------------------------------------------
 * be able to split??
 *----------------------------------------------------------------------------
*/
int
issplit(char *p, int i) {
	if (i < BODY_ADDR) {
		if (*p == *Separator[i])
			return (1);
	}
	else {
		if (*p == *Separator[i] && *(p + 1) == SPACE)
			return (1);
	}

	return (0);
}

/*----------------------------------------------------------------------------
 * split slog
 *----------------------------------------------------------------------------
*/
void
expand_field() {
	fnum *= 2;
	fsize = fnum * sizeof(field);
	field = xrealloc(field, fsize);
}

void
split(char *p) {
	char *q;	/* starting pointer of each "field"s */
	int i;		/* index of "field" */

	if (!*p)
		return;

	clear_smfield();
	q = p;
	for (i = 0; *p != '\0' && i < TOTAL_FIELD; ++p) {
		if (issplit(p, i)) {
			for (; (*p == SPACE) || (*p == COMMA); ++p) {
				*p = '\0';
			}
			if (i == (fnum))
				expand_field();
			field[i] = q;
			store_smfield(field[i], i);
			++i;
			q = p;
		}
	}

	/*
	 * last field
	*/
	if (i == fnum)
		expand_field();
	if (*p == '\0') {
		field[i] = q;
		store_smfield(field[i], i);
		++i;
	}

	nfield = i;	/* i + 1 */

	return;
}

/*----------------------------------------------------------------------------
 * get nfield
 *----------------------------------------------------------------------------
 */
int
getnfield(void) {
	return nfield;
}

/*----------------------------------------------------------------------------
 * get field
 *----------------------------------------------------------------------------
 */
char *
getfield(int index) {
	if (index < 0 || index >= nfield)
		return (char *)NULL;

	return field[index];
}

/*----------------------------------------------------------------------------
 * get log
 *----------------------------------------------------------------------------
 */
int
init_getlog(void) {
	if (log == NULL && field == NULL) {
		lsize = MAXBUFSZ * sizeof(char);
		log = xmalloc(lsize);
		slog = xmalloc(lsize);
		fnum = MAXFIELDNUM;
		fsize = fnum * sizeof(field);
		field = xmalloc(fsize);
		return (0);
	}

	return (-1);
}

int
expand_log() {
	lsize += lsize;
	log = xrealloc(log, lsize);
	slog = xrealloc(slog, lsize);
	return (1);
}

char *
getlog(FILE *fp, off_t *n) {
	char *q;
	size_t len = 0;

	do {
		if (fgets((log + len), (lsize - len), fp) != NULL) {
			if ((q = strchr(log, NEWLINE)) != NULL)
				*q = '\0';
			if ((len = strlen(log)) == (lsize - 1))
				continue;
			else {
				*n = len + 1;
				break;
			}
		}
		else {
			if (ferror(fp) || feof(fp)) {
				*n = 0;
				return (NULL);
			}
		}
	} while (expand_log());

	strncpy(slog, log, lsize);
	split(slog);

	return (log);
}


#ifdef DEBUG_GETLOG
/*----------------------------------------------------------------------------
 * debug section
 *----------------------------------------------------------------------------
 *
 * following code is the driver for getlog().
 * if you want to test getlog() only, you can do "make getlog".
 *
 */

#define MAILLOG	"/var/log/syslog"

int
main(int argc, char **argv)
{
	char *name;
	char *buff;
	int i, j, n;

	FILE *fp;


	if (argc > 1) {
		name = xstrdup(argv[1]);
	}
	else {
		name = xstrdup(MAILLOG);
	}
		
	if ((fp = fopen(name, "r")) == (FILE *)NULL) {
		fprintf(stderr, "can not open %s\n", name);
	}

	if (init_getlog() < 0) {
		fprintf(stderr, "can not allocate buff\n");
		exit (1);
	}

	for (; (buff = getlog(fp, &n)) != (char *)NULL;) {
		j = getnfield();
		for (i = 0; i < j; ++i) {
			fprintf(stdout, "%s ", getfield(i));
		}
		fprintf(stdout, "\n");	/* end of field */
	}

	exit(0);
}

#endif

/* end of source */
