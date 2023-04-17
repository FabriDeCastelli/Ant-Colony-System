CC=gcc
CFLAGS=-g -Wall -O3 -std=c99
LDLIBS=-lm

MAIN=main
 
all: $(MAIN)

$(MAIN): $(MAIN).o

clean:
	rm -f $(MAIN) *.o  

zip:
	zip $(MAIN).zip makefile *.c *.h *.py *.md *.dat
