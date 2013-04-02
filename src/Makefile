CC=g++
LD=ld
CPP=g++

OBJ=list.o btree.o mparse.o
LIBNAME=mparse
LIBOPTIONS=-fPIC
CFLAGS=-Wall $(LIBOPTIONS) -g3 -fpermissive
LIBS=-l$(LIBNAME) -lm
LDFLAGS=-g3 -L. $(LIBS)

all: libmparse.so

libmparse.so: $(OBJ)
	$(CC) -shared -Wl,-soname,lib$(LIBNAME).so -o lib$(LIBNAME).so  $(OBJ)

mparse.o: list.cpp list.h btree.cpp btree.h mparse.cpp mparse.h
	$(CPP) $(CFLAGS) -c mparse.cpp
	
list.o: list.cpp list.h
	$(CPP) $(CFLAGS) -c list.cpp

btree.o: btree.cpp btree.h
	$(CPP) $(CFLAGS) -c btree.cpp

clean:
	rm *.o *.so.* *.so -rf

install:
	mkdir -p /usr/local/lib/libmparse/
	cp libmparse.so /usr/local/lib/libmparse/
	mkdir -p /usr/include/mparse
	cp mparse.h /usr/include/mparse/
	cp btree.h /usr/include/mparse/
	cp list.h /usr/include/mparse/
	echo "/usr/local/lib/libmparse/" > /etc/ld.so.conf.d/limparse.conf
	ldconfig

remove:
	rm -rf /usr/local/lib/libmparse
	rm -rf /usr/include/mparse
	rm /etc/ld.so.conf.d/libmparse.conf
	ldconfig

	
	
	
	
	