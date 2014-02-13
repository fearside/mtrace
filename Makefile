#
# mtrace's Makefile
#

#OSTYPE	= -DSOLARIS
OSTYPE	= -DMACOSX

#CC	= cc
CC	= gcc
TAGS	= ctags	# for vi
#COMP 	= compress
COMP 	= gzip
DEBUG	= # -DDEBUG
DATE	= `date +%Y%m%d`
OPTIM	= -O2
#OPTIM	= -O2 -pg
#CFLAGS	= -pg ${OPTIM} ${DEBUG}
CFLAGS	= ${OSTYPE} -g -Wall ${OPTIM} ${DEBUG}
LDFLAGS	= # -static
LIBS	= 
INCS	= mtrace.h
OBJS	= util.o \
	  getlog.o \
	  mtrace.o
SRCS	= util.c \
	  getlog.c \
	  mtrace.c

TARGET	= mtrace


all:${TARGET}

ctags:
	ctags *.c *.h

etags:
	etags *.c *.h

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}

touch:
	touch *.c

.c.o:

.h.c:


clean: clean-getlog clean-util
	rm -f core *.exe.stackdump *.o *.exe ${TARGET} gmon.out mtrace.out

clean-getlog:
	rm -f getlog getlog.txt

clean-util:
	rm -f util util.txt

tar:
	tar cvf - ${SRCS} ${INCS} Makefile | ${COMP} - > ${TARGET}.tgz
	[ ! -d ./Backup ] && mkdir Backup
	-mv ${TARGET}.tgz Backup/${TARGET}.tgz.${DATE}

#
# test suite
#
test: getlog msort util test-all

getlog: getlog.c
	${CC} ${CFLAGS} ${LDFLAGS} -DDEBUG_GETLOG -o $@ $^ ${LIBS}

util: util.c
	${CC} ${CFLAGS} ${LDFLAGS} -DDEBUG_UTIL -o $@ $^ ${LIBS}


test-all: test-getlog test-msort test-util

test-getlog:
	@/bin/echo " --- start getlog test ==> \c"
	@./getlog ./Test/getlog/getlog.in1 > ./Test/getlog/.result.getlog.out1
	@./getlog ./Test/getlog/getlog.in2 > ./Test/getlog/.result.getlog.out2
	@diff -c ./Test/getlog/.result.getlog.out1 ./Test/getlog/.result.getlog.out2 > /dev/null
	@/bin/echo "successfully done --- "
	@rm ./Test/getlog/.result*out?

test-util:
	@/bin/echo " --- start util test ==> \c"
	@/bin/echo "successfully done --- "

# end of makefile
