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
#include <sys/types.h>


/*-----------------------------------------------------------------------------
 * MACRO
 *-----------------------------------------------------------------------------
*/

/*
 * common phrase
*/
#define TAB             '\t'
#define CR              '\r'
#define NEWLINE         '\n'
#define EQUAL           '='
#define COMMA           ','
#define SPACE           ' '
#define BRACKET1        '<'
#define BRACKET2        '['
#define DASH            '-'


/*
 * sendmail field
*/
#define SM_MONTH	1
#define SM_DAY		2
#define SM_TIME		3
#define SM_HOSTNAME	4
#define SM_PROCESS	5
#define SM_SYSLOGID	6
#define SM_QID		7
#define SM_FROM		8
#define SM_SIZE		9
#define SM_CLASS	10
#define SM_NRCPTS	11
#define SM_MSGID	12
#define SM_RELAY	13
#define SM_TO		14
#define SM_CTLADDR	15
#define SM_DELAY	16
#define SM_XDELAY	17
#define SM_MAILER	18
#define SM_PRI		19
#define SM_DSN		20
#define SM_STAT		21

#define SM_FIELD	64
#define SM_FIELD_TO	1024


/*
 * for syslog
*/
enum syslog_field_tag {                 /* field label */
	HEAD_MONTH      = 0,
	HEAD_DAY        = 1,
	HEAD_TIME       = 2,
	HEAD_HOSTNAME   = 3,
	HEAD_IDENT      = 4,
	HEAD_SYSLOG1    = 5,
	HEAD_SYSLOG2    = 6,
	HEAD_SYSLOG3    = 7,
	HEAD_QID        = 8,
	BODY_ADDR       = 9,
	BODY_NO11       = 10,
	BODY_NO12       = 11,
	BODY_NO13       = 12,
	BODY_NO14       = 13,
	BODY_NO15       = 14,
	BODY_NO16       = 15,
	BODY_NO17       = 16,
	BODY_NO18       = 17,
	TOTAL_FIELD     = 18
};



/*-----------------------------------------------------------------------------
 * function
 *-----------------------------------------------------------------------------
*/

extern int init_getlog(void);
extern int getnfield(void);
extern char *getfield(int);
extern char *getlog(FILE *, off_t *);
extern char *get_smfield(int);
extern char *get_smfield_to(int);

/* end of header */
