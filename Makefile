#Makefile

CC=g++
LD=ld

TARGET=parser
#LIBOPTIONS=-fPIC
CFLAGS=-Wall $(LIBOPTIONS) -g3
LIBS=-lparse -lm
LDFLAGS=-g3 -L./src/ $(LIBS)

all: parser

parser: parser.o
	$(CC)  -o $(TARGET) $(TARGET).o $(LDFLAGS)

parser.o: libparse.so parser.c
	$(CC) $(CFLAGS) -c parser.c

libparse.so:
	make -C src/

clean:
	rm parser *.o *.so.* *.so -rf
	$(MAKE) -C src/ clean
