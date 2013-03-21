ALLFILES=$(shell find -name '*.*' -not -name '*.yy.*')
CSOURCES=$(shell find -name '*.c' -not -name '*.yy.c')
CPPSOURCES=$(shell find -name '*.cpp' -not -name '*.yy.cpp')
BISONSOURCES=$(shell find -name '*.y')
BISONC=$(patsubst %.y, %.yy.c, $(BISONSOURCES))
LEXSOURCES=$(shell find -name '*.lex')
LEXC=$(patsubst %.lex, %.yy.c, $(LEXSOURCES))
COBJECTS=$(patsubst %.c, %.o, $(BISONC) $(LEXC) $(CSOURCES))
CPPOBJECTS=$(patsubst %.cpp, %.o, $(CPPSOURCES))
EXISTINGOBJS=$(shell find -name '*.o')
EXISTINGYYC=$(shell find -name '*.yy.c')

OUTNAME=cheshirec

LD=gcc
CC=gcc
CPP=g++
LEX=flex
BISON=bison

LDFLAGS=-lm -lstdc++
CFLAGS=-Wall -Wextra -Werror -g -Wno-unused
CPPFLAGS=-Wall -Wextra -Werror -g -Wno-unused -std=c++0x
LEXFLAGS=
BISONFLAGS=-rall

all: clean build todos
	-rm *.o *.yy.* *.tab.*

clean:
	-rm $(OUTNAME)
	-rm *.yy.* *.o *.tab.*
	-rm *.gch

cleangen:
	-rm *.yy.* *.o *.tab.*

build: generate $(COBJECTS) $(CPPOBJECTS)
	@echo " LD	*.o"
	@$(LD) $(LDFLAGS) -o $(OUTNAME) $(COBJECTS) $(CPPOBJECTS)

generate: $(BISONC) $(LEXC)

$(BISONC): $(BISONSOURCES)
	@echo " BISON   $<"
	@$(BISON) $(BISONFLAGS) -o $@ $<

$(LEXC): $(LEXSOURCES)
	@echo " LEX	$<"
	@$(LEX) -o $@ $<

.c.o:
	@echo " CC	$<"
	@$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	@echo " C++	$<"
	@$(CPP) $(CPPFLAGS) -o $@ -c $<

todos:
	-@for file in $(ALLFILES); do grep -H TODO $$file; done; true
	-@for file in $(ALLFILES); do grep -H todo $$file; done; true
	-@for file in $(ALLFILES); do grep -H FIXME $$file; done; true
