#Makefile

CC=g++
LD=ld

TARGET=parser
#LIBOPTIONS=-fPIC
CFLAGS=-Wall $(LIBOPTIONS) -g3
LIBS=-lmparse -lm
LDFLAGS=-g3 -L./src/ $(LIBS)

all: parser

parser: parser.o
	$(CC)  -o $(TARGET) $(TARGET).o $(LDFLAGS)

parser.o: libmparse.so parser.c
	$(CC) $(CFLAGS) -c parser.c

libmparse.so:
	make -C src/

clean:
	rm parser *.o *.so.* *.so -rf
	$(MAKE) -C src/ clean

install:
	make -C src/ install

remove:
	make -C src/ remove