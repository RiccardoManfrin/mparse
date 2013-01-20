CC=g++
LD=ld
CPP=g++

OBJ=list.o btree.o parser.o
LIBNAME=mparse
LIBOPTIONS=-fPIC
CFLAGS=-Wall $(LIBOPTIONS) -g3 -fpermissive
LIBS=-l$(LIBNAME) -lm
LDFLAGS=-g3 -L. $(LIBS)

all: libmparse.so

libmparse.so: $(OBJ)
	$(CC) -shared -Wl,-soname,lib$(LIBNAME).so -o lib$(LIBNAME).so  $(OBJ)

parser.o: list.cpp list.h btree.cpp btree.h parser.cpp parser.h
	$(CPP) $(CFLAGS) -c parser.cpp
	
list.o: list.cpp list.h
	$(CPP) $(CFLAGS) -c list.cpp

btree.o: btree.cpp btree.h
	$(CPP) $(CFLAGS) -c btree.cpp

clean:
	rm parser *.o *.so.* *.so -rf
	