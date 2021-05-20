CC = gcc

INCLUDE = .

INSTDIR = ~/C

App: main.o BTree.o
	${CC} -o App main.o BTree.o

BTree.o: BTree.c BTree.h btreedef.h LIST.h
	${CC} -I${INCLUDE} -c BTree.c BTree.h btreedef.h LIST.h

main.o: main.c BTree.c BTree.h btreedef.h
	${CC} -I${INCLUDE} -c main.c BTree.c BTree.h btreedef.h

clean:
	-rm *.o *.gch

install:
	@if [ -d  ${INSTDIR} ];\
		then\
		cp App ${INSTDIR}/MyBTREE;\
		chmod +x ${INSTDIR}/MyBTREE;\
		chmod go-x ${INSTDIR}/MyBTREE;\
	fi
		
