CC = gcc
#compiler flags
FLAGS = -Wall
#linker flags ! check line 17
LDFLAGS = -lpthread -D_REENTRANT 
OBJS = config.o dnsserver.o main.o mmapped_file.o queues.o stats.o
PROG = dnsserver

###########

all: ${PROG}

clean:
	rm ${OBJS} ${PROG}

${PROG}:  ${OBJS}
	${CC} ${FLAGS} ${OBJS} -o $@ ${LDFLAGS}

.c.o:
	${CC} $< -c -o $@ 

###########
config.o: main.h config.c

dnsserver.o: main.h dnsserver.c

main.o: main.h main.c

mmapped_file.o: main.h mmapped_file.c

queues.o: main.h queues.c

stats.o: main.h stats.c



