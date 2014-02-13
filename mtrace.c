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

#include <signal.h>
#include <ctype.h>
#include <sys/time.h>


/*----------------------------------------------------------------------------
 * macro
 *----------------------------------------------------------------------------
*/
#define INIT_TABLE_SIZE		32771		/* Msg hash table size */



/*----------------------------------------------------------------------------
 * type definition
 *----------------------------------------------------------------------------
*/
typedef struct _opt {
	char *sender;
	char *receiver;
	int ignore_cap_sender;
	int ignore_cap_receiver;
	int nfile;	/* argc */
	char **file;	/* argv */
} Opt;

typedef struct _date {
	char *month;
	char *day;
	char *time;
} Date;

typedef struct _hostinfo {
	struct _hostinfo *next;    /* used by Msg hash table msgtbl[] */
	struct _hostinfo *nextqid; /* used by Hostinfo hash table qidtbl[] */
	char *qid;
	int qidlen;
	char *sender;
	char *receiver;
	char *hostname;
	int hostnamelen;
	char *msgsize;
	char *status;
	Date date;
} Hostinfo;

typedef struct _msg {
	struct _msg *next;
	char *msgid;	/* key */
	int msgidlen;
	Hostinfo hostinfo;
} Msg;

/*----------------------------------------------------------------------------
 * global variable
 *----------------------------------------------------------------------------
*/
static Msg **msgtbl;
static Hostinfo **qidtbl;

int debug = 0;


/*----------------------------------------------------------------------------
 * proto type
 *----------------------------------------------------------------------------
*/



/*============================================================================
 * program section
 *============================================================================
*/

/*----------------------------------------------------------------------------
 * print usage
 *----------------------------------------------------------------------------
*/
void
mt_print_usage() {
	fprintf(stderr,
		"usage: \n");
	fprintf(stderr,
		"       mtrace -s sender | -S sender [logfile] ...\n");
	fprintf(stderr,
		"       mtrace -r receiver | -R receiver [logfile] ...\n");
	fprintf(stderr,
		"       mtrace -[sS] sender -[rR] receiver [logfile] ...\n");

	exit(1);
}


/*----------------------------------------------------------------------------
 * print time of excusion
 *----------------------------------------------------------------------------
*/
struct timeval __mt_stp;	/* starting time */
struct timeval __mt_etp;	/* ending time */

void
mt_set_start_time(void) {
	if (gettimeofday(&__mt_stp, NULL)) {
		fprintf(stderr, "gettimeofday failure\n");
		exit (1);
	}

	return;
}

void
mt_print_eraps(void) {
	if (gettimeofday(&__mt_etp, NULL)) {
		fprintf(stderr, "gettimeofday failure\n");
		exit (1);
	}

	fprintf(stderr, "Start Time: %s", ctime((const time_t *)&(__mt_stp.tv_sec)));
	fprintf(stderr, "End   Time: %s", ctime((const time_t *)&(__mt_etp.tv_sec)));

	if (__mt_stp.tv_sec == __mt_etp.tv_sec)
		fprintf(stderr, "Eraps(ms): %ld\n", (__mt_etp.tv_usec - __mt_stp.tv_usec));
	else
		fprintf(stderr, "Eraps(s): %ld\n", (__mt_etp.tv_sec - __mt_stp.tv_sec));

	return;
}


/*----------------------------------------------------------------------------
 * parse option
 *----------------------------------------------------------------------------
*/
char *
mt_tolower(char *p) {
	char *q;
	for (q = p; *q != '\0'; ++q) {
		if (isalpha((int)*q))	/* need to improve performance ?? */
			*q = tolower((int)*q);
	}
	return (p);
}

Opt *
mt_get_option(int argc, char **argv)
{
	Opt *opt;
	int ch;

	opt = xmalloc(sizeof(Opt));

	opt->sender               = NULL;
	opt->receiver             = NULL;
	opt->ignore_cap_sender    = 0;
	opt->ignore_cap_receiver  = 0;
	opt->nfile                = 0;
	opt->file                 = NULL;

	while ((ch = getopt(argc, argv, "hR:S:r:s:")) != -1) {
		switch(ch) {
		case 'R':
			opt->receiver = xstrdup(optarg);
			break;
		case 'r':
			opt->ignore_cap_receiver = 1;
			if (opt->receiver == NULL)
				opt->receiver = mt_tolower(xstrdup(optarg));
			break;
		case 'S':
			opt->sender = strdup(optarg);
			break;
		case 's':
			opt->ignore_cap_sender = 1;
			if (opt->sender == NULL)
				opt->sender = mt_tolower(xstrdup(optarg));
			break;
		case 'h':
		default:
			mt_print_usage();
			break;
		}
	}

	if (!opt->sender && !opt->receiver)
		mt_print_usage();

	argc -= optind;
	argv += optind;

	opt->nfile = argc;
	opt->file = argv;

	return (opt);
}


/*----------------------------------------------------------------------------
 * hash table function
 *----------------------------------------------------------------------------
*/
unsigned int
mt_hash(char *orig) {
        unsigned int h;
        unsigned char *p;

        h = 0;
        for (p = (unsigned char *)orig; *p != '\0'; ++p) {
                h = 37 * h + *p;	/* need to improve ?? */
        }

        return (h % INIT_TABLE_SIZE);
}

Msg *
mt_create_msgid_chunk() {
	return (xmalloc(sizeof(Msg)));
}

Msg *
mt_insert_msgid_chunk(Msg *p) {
	return (p->next = mt_create_msgid_chunk());
}

Msg *
mt_msgid_search(Msg *orig, int create) {
	unsigned int i;
	Msg *chunk, *prev;

	if (!orig->msgid)
		return (NULL);

	i = mt_hash(orig->msgid);
	if (msgtbl[i] == NULL) {
		if (create)
			return (msgtbl[i] = mt_create_msgid_chunk());
	}
	
	for (chunk = msgtbl[i]; chunk != NULL; chunk = chunk->next) {
		prev = chunk;
		if (chunk->msgidlen != orig->msgidlen)
			continue;
		else if (strcmp(chunk->msgid, orig->msgid) == 0)
			return (chunk);
	}

	if (create)
		return (mt_insert_msgid_chunk(prev));

	return (NULL);
}


Hostinfo *
mt_qid_search(Hostinfo *orig, int insert) {
	unsigned int i;
	Hostinfo *chunk, *prev;

	i = mt_hash(orig->qid);
	if (qidtbl[i] == NULL) {
		if (insert)
			return (qidtbl[i] = orig);
	}

	for (chunk = qidtbl[i]; chunk != NULL; chunk = chunk->nextqid) {
		prev = chunk;
		if (chunk->qidlen != orig->qidlen ||
		    chunk->hostnamelen != orig->hostnamelen)
			continue;
		if (strcmp(chunk->qid, orig->qid) == 0 &&
		    strcmp(chunk->hostname, orig->hostname) == 0)
			return (chunk);
	}

	if (insert)
		return (prev->nextqid = orig);

	return (NULL);
}

Hostinfo *
mt_create_hostinfo_chunk(void) {
	return (xmalloc(sizeof(Hostinfo)));
}

Hostinfo *
mt_insert_hostinfo_chunk(Hostinfo *p) {
	return (p->next = mt_create_hostinfo_chunk());
}

Hostinfo *
mt_hostinfo_search(Hostinfo *orig, int insert) {
	Hostinfo *hp;

	if (orig->next == NULL)
		return (mt_insert_hostinfo_chunk(orig));

	for (hp = orig; hp->next != NULL; hp = hp->next)  { }
	if (insert)
		return (mt_insert_hostinfo_chunk(hp));

	return (hp);
}


void
mt_init_msgtbl() {
	msgtbl = xmalloc(INIT_TABLE_SIZE * sizeof(Msg *));
	qidtbl = xmalloc(INIT_TABLE_SIZE * sizeof(Hostinfo *));
	return;
}


/*----------------------------------------------------------------------------
 * print progress
 *----------------------------------------------------------------------------
*/

static char *__mt_file = NULL;
static off_t __mt_total = 0;
static off_t __mt_current = 0;

void
__mt_print_cr(void) {
	fprintf(stderr, "\r");
	return;
}

void
__mt_print_progress_stdin(void) {
	static char *msg = "read stdin, progress: %ld byte%s read";

	__mt_print_cr();
	fprintf(stderr, msg, __mt_current, (__mt_current > 0 ? "s" : ""));
	alarm(1);
	return;
}

void
__mt_print_progress_rfile(void) {
	static char *msg = "read %s, progress: (%d%%), total %ld bytes, %ld bytes read";
	off_t prog, current, total;

	if (__mt_current > 1000000)
		current = __mt_current / 1000000;
	else
		current = __mt_current;

	if (__mt_total > 1000000)
		total = __mt_total / 1000000;
	else
		total = __mt_total;

	prog = (100 * current) / total;
	
	__mt_print_cr();
	fprintf(stderr, msg, __mt_file, prog, __mt_total, __mt_current);
	alarm(1);
	return;
}

void
__mt_set_signal_handler(int regular) {
	struct sigaction act, oact;

	if (regular)
		act.sa_handler = (void (*)())__mt_print_progress_rfile;
	else
		act.sa_handler = (void (*)())__mt_print_progress_stdin;

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	//act.sa_flags |= SA_RESTART;

	if (sigaction(SIGALRM, &act, &oact) < 0)
		fprintf(stderr, "can not set signal hander SIGALRM\n");

	alarm(1);

	return;
}

int
mt_set_progress_bar(char *file) {
	struct stat fs;

	__mt_total = 0;
	__mt_current = 0;
	
	if (file == NULL) {
		__mt_set_signal_handler(0);
		return (1);
	}

	if (stat(file, &fs) == 0) {
		__mt_file = file;
		__mt_total = fs.st_size;
		__mt_set_signal_handler(1);
	}
	else {
		fprintf(stderr, "%s\n", strerror(errno));
		return (0);
	}

	return (1);
}

void
mt_progress_countup(off_t c) {
	__mt_current += c;
	return;
}

void
mt_sigsend(pid_t myself) {
#ifdef SOLARIS
	if (sigsend(P_PID, myself, SIGALRM) != 0) {
#else
	if (kill(myself, SIGALRM) != 0) {
#endif
		fprintf(stderr, "can not send sigalrm, quit immediately\n");
		exit (1);
	}
	return;
}


/*----------------------------------------------------------------------------
 * comparation
 *----------------------------------------------------------------------------
*/
int
mt_strcmp_cap(char *log, char *opt) {
	int rc;
	char *tmp;

	tmp = mt_tolower(xstrdup(log));
	rc = strcmp(tmp, opt);
	xfree(tmp);
	return (rc);
}

int
mt_strcmp_nocap(char *log, char *opt) {
	return (strcmp(log, opt));
}

static int (*mt_strcmp[])() = {
	mt_strcmp_nocap,
	mt_strcmp_cap,
	NULL,
};

int
mt_strcmp_sender(char *sender, Opt *opt) {
	return (*mt_strcmp[opt->ignore_cap_sender])(sender, opt->sender);
}

int
mt_strcmp_receiver(Opt *opt) {
	char *rcpt;
	int i;
	for (i = 0; (rcpt = get_smfield_to(i)) != NULL; ++i) {
		if ((*mt_strcmp[opt->ignore_cap_receiver])(rcpt, opt->receiver) == 0)
			return (0); /* match */
	}

	return (1);
}


/*----------------------------------------------------------------------------
 * store message
 *----------------------------------------------------------------------------
*/
#define MSGIDLEN	16
char *
mt_assign_msgid() {
	static int msgidlen = MSGIDLEN;
	static char format[BUFSIZ];
	static int num = 0;
	char *msgid;

	msgid = xmalloc(msgidlen);
	sprintf(format, "%%%03dd", (msgidlen - 1));
	snprintf(msgid, msgidlen, format, ++num);

	return (msgid);
}

void
mt_set_tempmsg_sender(Msg *p) {
	char *q;

	if ((q = get_smfield(SM_MSGID)) != NULL)
		p->msgid = xstrdup(q);
	else
		p->msgid = mt_assign_msgid();

	p->msgidlen              = strlen(p->msgid);
	p->hostinfo.sender       = xstrdup(get_smfield(SM_FROM));
	p->hostinfo.qid          = xstrdup(get_smfield(SM_QID));
	p->hostinfo.qidlen       = strlen(p->hostinfo.qid);
	p->hostinfo.hostname     = xstrdup(get_smfield(SM_HOSTNAME));
	p->hostinfo.hostnamelen  = strlen(p->hostinfo.hostname);
	p->hostinfo.msgsize      = xstrdup(get_smfield(SM_SIZE));
	return;
}

void
mt_set_tempmsg_receiver(Msg *p) {
	p->hostinfo.receiver     = xstrdup(get_smfield(SM_TO));
	p->hostinfo.qid          = xstrdup(get_smfield(SM_QID));
	p->hostinfo.qidlen       = strlen(p->hostinfo.qid);
	p->hostinfo.hostname     = xstrdup(get_smfield(SM_HOSTNAME));
	p->hostinfo.hostnamelen  = strlen(p->hostinfo.hostname);
	p->hostinfo.status       = xstrdup(get_smfield(SM_STAT));
	p->hostinfo.date.month   = xstrdup(get_smfield(SM_MONTH));
	p->hostinfo.date.day     = xstrdup(get_smfield(SM_DAY));
	p->hostinfo.date.time    = xstrdup(get_smfield(SM_TIME));
	return;
}

void
mt_store_msg_sender(Msg *dst, Msg *src) {
	Hostinfo *hp;
	
	if (dst->hostinfo.next == NULL) {
		dst->msgid     = src->msgid;
		dst->msgidlen  = src->msgidlen;
	}

	hp = mt_hostinfo_search(&(dst->hostinfo), 1);
	hp->sender       = src->hostinfo.sender;
	hp->qid          = src->hostinfo.qid;
	hp->qidlen       = src->hostinfo.qidlen;
	hp->hostname     = src->hostinfo.hostname;
	hp->hostnamelen  = src->hostinfo.hostnamelen;
	hp->msgsize      = src->hostinfo.msgsize;

	if (mt_qid_search(hp, 1) == NULL) {
		fprintf(stderr, "\ncan not insert qid hash table, quid immediately\n");
		exit (1);
	}

	return;
}

void
mt_store_msg_receiver(Hostinfo *dst, Msg *src) {
	dst->receiver  = src->hostinfo.receiver;
	dst->status    = src->hostinfo.status;
	dst->date      = src->hostinfo.date;
	return;
}

void
mt_store_message(Opt *opt) {
	char *addr;
	Msg *chunk, temp;
	Hostinfo *hpchunk;

	memset(&(temp), 0, sizeof(temp));

	/*
	 * store msgid hash table in case of the followings
	 * (1) if not given a sender address by -s/S, all envelope sender 
	 *     address are stored in the table
	 * (2) if given a sender address by -s/S and fit in envelope sender
	 *
	*/
	if ((addr = get_smfield(SM_FROM)) != NULL) {
		if (!opt->sender || (*mt_strcmp_sender)(addr, opt) == 0) {
			mt_set_tempmsg_sender(&temp);
			chunk = mt_msgid_search(&temp, 1);
			mt_store_msg_sender(chunk, &temp);
		}
	}
	else if ((addr = get_smfield(SM_TO)) != NULL) {
		mt_set_tempmsg_receiver(&temp);
		if (!opt->receiver || (*mt_strcmp_receiver)(opt) == 0) {
			if ((hpchunk = mt_qid_search(&(temp.hostinfo), 0)) != NULL)
				mt_store_msg_receiver(hpchunk, &temp);
		}
	}
	
	return;
}

/*----------------------------------------------------------------------------
 * print result
 *----------------------------------------------------------------------------
*/
void
mt_print_char(int i, int c, int lf) {
	for (; i > 0; --i) {
		fprintf(stdout, "%c", c);
	}
	if (lf)
		fprintf(stdout, "\n");
	return;
}

void
mt_print_hostinfo(Hostinfo *p, int tab) {
	mt_print_char(tab, ' ', 0);
	fprintf(stdout, "Hostname: %s\n", (p->hostname ? p->hostname : "(null)"));
	mt_print_char(tab, ' ', 0);
	fprintf(stdout, "Sender:   %s\n", (p->sender ? p->sender : "(null)"));
	mt_print_char(tab, ' ', 0);
	fprintf(stdout, "Receiver: %s\n", (p->receiver ? p->receiver : "(null)"));
	mt_print_char(tab, ' ', 0);
	fprintf(stdout, "Date:     %s %s %s\n", (p->date.month ? p->date.month : "(null)"), (p->date.day ? p->date.day : "(null)"), (p->date.time ? p->date.time : "(null)"));
	mt_print_char(tab, ' ', 0);
	fprintf(stdout, "Status:   %s\n", (p->status ? p->status : "(null)"));
	fprintf(stdout, "\n");
}

void
mt_print_result() {
	unsigned int i, n;
	Msg *p;
	Hostinfo *q;

	n = 0;
	mt_print_char(72, '-', 1);
	for (i = 0; i < INIT_TABLE_SIZE; ++i) {
		for (p = msgtbl[i]; p != NULL; p = p->next) {
			int tab = 0;
			/*
			 * print only completed data
			*/
			if (p->hostinfo.next == NULL || p->hostinfo.next->receiver == NULL)
				continue;
			fprintf(stdout, "(%-4.4d) message-id: %s\n", ++n, p->msgid);
			for (q = p->hostinfo.next; q != NULL; q = q->next) {
				mt_print_hostinfo(q, (tab += 3));
			}
		}
	}
	mt_print_char(72, '-', 1);

	return;
}


/*----------------------------------------------------------------------------
 * main
 *----------------------------------------------------------------------------
*/
FILE *
mt_getfd(Opt *opt, int i) {
	if (opt->nfile == 0)
		return (stdin);

	return (fopen((opt->file)[i], "r"));
}

int
main(int argc, char **argv) {
	Opt *opt;
	int i;
	pid_t myself = getpid();
	int tty = isatty(STDERR_FILENO);

	mt_set_start_time();
	opt = mt_get_option(argc, argv);

	i = 0;
	do {
		FILE *fd;
		off_t current = 0;
		char *line;
		int alrmon = 0;

		if ((fd = mt_getfd(opt, i)) == NULL) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit (1);
		}

		if (init_getlog() < 0)
			exit (1);
		mt_init_msgtbl();

		if (tty && (alrmon = mt_set_progress_bar(opt->file[i])) > 0)
			mt_sigsend(myself);

		while ((line = getlog(fd, &current)) != NULL) {
			mt_progress_countup(current);
			mt_store_message(opt);
		}

		if (alrmon) {
			mt_sigsend(myself);
			fprintf(stderr, "...completed\n");
			alarm(0);
		}
		++i;
	} while (i < opt->nfile);

	mt_print_result();
	mt_print_eraps();

	exit(0);
}

/* end of source */
