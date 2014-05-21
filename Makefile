

CC=gcc
LINK=${CC} ${LIB_DIR}
CFLAGS=-g -Wall ${INCLUDE_DIR} ${DEFINES}
LIBS=
INCLUDE_DIR=-I.
LIB_DIR=-L.

SRCS=unmo3.c mo3_unpack.c mo3_parse.c mo3_mp3.c

OBJS=$(SRCS:%.c=%.o)

MAKEDEPEND=makedepend

TARGETS=unmo3 

FILES=${SRCS} Makefile mo3_unpack.h mo3_parse.h endian_macros.h README.txt mo3_mp3.h 16to8.c demo.sh dannyelf_ll.mo3

COMPILE=$(CC) -c ${CFLAGS}

all: $(TARGETS)

unmo3: ${OBJS}
	${LINK} -o unmo3 $^

test: unmo3
	./test.sh

demo: unmo3
	./demo.sh

clean:
	rm -f *.o $(TARGETS)

tar:
	tar cvf unmo3.tar ${FILES}

ziparc:
	./zip -9r unmo3-`date +%Y%m%d_%H%M%S`.zip ${FILES}

ziptest:
	./zip -9r unmo3_test.zip test.sh Testdata
	
.c.o:
	$(COMPILE) $<

dep:
	$(MAKEDEPEND) -f.depend -- ${CFLAGS} -- ${SRCS}

include .depend
