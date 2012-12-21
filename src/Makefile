CC=g++
LD=ld
CPP=g++

OBJ= list.o btree.o exception.o debug.o lib_parse.o
LIBNAME=parse
LIBOPTIONS=-fPIC
CFLAGS=-Wall $(LIBOPTIONS) -g3 -fpermissive
LIBS=-l$(LIBNAME) -lm
LDFLAGS=-g3 -L. $(LIBS)

all: libparse.so

libparse.so: $(OBJ)
	$(CC) -shared -Wl,-soname,lib$(LIBNAME).so -o lib$(LIBNAME).so  $(OBJ)

lib_parse.o: list.cpp list.h btree.cpp btree.h lib_parse.cpp lib_parse.h exception.c exception.h defines.h debug.c debug.h std_functions.h
	$(CPP) $(CFLAGS) -c lib_parse.cpp

list.o: list.cpp list.h
	$(CPP) $(CFLAGS) -c list.cpp

btree.o: btree.cpp btree.h
	$(CPP) $(CFLAGS) -c btree.cpp

exception.o: exception.c exception.h
	$(CC) $(CFLAGS) -c exception.c

debug.o: debug.c debug.h
	$(CC) $(CFLAGS) -c debug.c
clean:
	rm parser *.o *.so.* *.so -rf
	