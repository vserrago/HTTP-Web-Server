CC=gcc
OBJ=myhttpd.o stserver.o
BINARY=server

${BINARY}: Makefile ${OBJ}
	${CC} -o ${BINARY} ${OBJ}

 myhttpd.o: myhttpd.c stserver.h
	${CC} -c myhttpd.c
 
 stserver.o: stserver.c stserver.h
	${CC} -c stserver.c
 
clean:
	rm ${OBJ} ${BINARY}

