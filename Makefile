TARGET  := server
SRCS    := myhttpd.c stserver.c util.c globsem.c mthread.c
OBJS    := ${SRCS:.c=.o} 
DEPS    := ${SRCS:.c=.dep} 
XDEPS   := $(wildcard ${DEPS}) 

CC      = gcc
#CC      = clang
CCFLAGS = 
LDFLAGS = -lpthread 
LIBS    =

.PHONY: all clean distclean 
	all:: ${TARGET} 

ifneq (${XDEPS},) 
	include ${XDEPS} 
endif 

${TARGET}: ${OBJS} 
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.c %.dep 
	${CC} ${CCFLAGS} -o $@ -c $< 

${DEPS}: %.dep: %.c Makefile 
	${CC} ${CCFLAGS} -MM $< > $@ 

clean:: 
	-rm -f *~ *.o *dep ${TARGET} 

distclean:: clean

