CFLAGS=-std=c11 -g -fno-common -Wall -Wno-switch
LDFLAGS=-lgc

SRCS=$(wildcard *.c)
OBJS=codegen.c hashmap.c parse.c preprocess.c strings.c tokenize.c type.c unicode.c

all: binder libbinder.so

binder: $(OBJS) main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libbinder.so: $(OBJS) binder.o
	$(CC) $(CFLAGS) -fPIC -o $@ $^ $(LDFLAGS) -shared 

$(OBJS) main.o binder.o: chibicc.h

clean:
	rm -rf binder tmp* $(TESTS) test/*.s test/*.exe stage2
	rm -rf *.o *.so *.json

.PHONY: test clean test-stage2
