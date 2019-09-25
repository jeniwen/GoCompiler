CC = gcc

CFLAGS = -Wall

all: parser.tab.o lex.yy.o main.o tree.o pretty.o weed.o symbol.o typecheck.o codegen.o
	$(CC) lex.yy.o parser.tab.o main.o tree.o pretty.o weed.o symbol.o typecheck.o codegen.o -o golite -ll

main: parser.tab.o lex.yy.o main.o tree.o pretty.o weed.o symbol.o typecheck.o codegen.o
	$(CC) lex.yy.o parser.tab.o main.o tree.o pretty.o weed.o symbol.o typecheck.o codegen.o -o golite -ll

parser.tab.c parser.tab.h: golite.y
	bison --yacc --verbose --defines=parser.tab.c --output=parser.tab.h golite.y

lex.yy.c: golite.l parser.tab.h
	flex golite.l

tree.c: symbol.o
	$(CC) tree.c -c

weed.c:
	$(CC) weed.c -c

pretty.c:
	$(CC) pretty.c -c

symbol.c: pretty.o
	$(CC) symbol.c -c

typecheck.c:
	$(CC) typecheck.c -c

# codegen.c:
	#$(CC) codegen.c -c

clean:
		rm parser.tab.* lex.yy.*; rm *.o
