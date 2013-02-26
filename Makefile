ALLFILES=$(shell find -name '*.*' -not -name '*.yy.*')
CSOURCES=$(shell find -name '*.c' -not -name '*.yy.c')
BISONSOURCES=$(shell find -name '*.y')
BISONC=$(patsubst %.y, %.yy.c, $(BISONSOURCES))
LEXSOURCES=$(shell find -name '*.lex')
LEXC=$(patsubst %.lex, %.yy.c, $(LEXSOURCES))
COBJECTS=$(patsubst %.c, %.o, $(BISONC) $(LEXC) $(CSOURCES))
EXISTINGOBJS=$(shell find -name '*.o')
EXISTINGYYC=$(shell find -name '*.yy.c')

OUTNAME=cheshirec

LD=gcc
CC=gcc
LEX=flex
BISON=bison

LDFLAGS=-lm
CFLAGS=-Wall -Werror -g -Wno-unused
LEXFLAGS=
BISONFLAGS=-rall

all: clean build todos
	-rm *.o

clean:
	-rm $(OUTNAME)
	-rm *.yy.* *.o *.tab.*

cleangen:
	-rm *.yy.* *.o *.tab.*

build: generate $(COBJECTS)
	@echo " LD	*.o"
	@$(LD) $(LDFLAGS) -o $(OUTNAME) $(COBJECTS)

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

todos:
	-@for file in $(ALLFILES); do grep -H TODO $$file; done; true
	-@for file in $(ALLFILES); do grep -H FIXME $$file; done; true
