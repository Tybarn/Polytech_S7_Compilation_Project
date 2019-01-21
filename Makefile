OBJ=projet.o projet_l.o projet_y.o
CC=gcc
CFLAGS=-Wall -std=c99 -I./ -g
LDFLAGS= -g -lfl

compil : $(OBJ)
	$(CC) -o compil $(OBJ) $(LDFLAGS)

test_lex : projet_l.o test_lex.c projet_y.h projet.h
	$(CC) $(CFLAGS) -Wno-unused-function -Wno-implicit-function-declaration -o test_lex test_lex.c  projet_l.o $(LDFLAGS)

projet.c :
	echo ''

projet.o: projet.c projet_y.h projet.h
	$(CC) $(CFLAGS) -c projet.c

projet_l.o: projet_l.c projet_y.h
	$(CC) $(CFLAGS) -Wno-unused-function -Wno-implicit-function-declaration -c projet_l.c

projet_l.c : projet.l projet_y.h projet.h
	flex --yylineno -o projet_l.c projet.l

projet_y.o : projet_y.c
	$(CC) $(CFLAGS) -c projet_y.c

projet_y.h projet_y.c : projet.y projet.h
	bison -v -b projet_y -o projet_y.c -d projet.y

genCode.c :
	echo ''

genCode.o: genCode.c projet.h projet_y.h genCode.h
	$(CC) $(CFLAGS) -c genCode.c

main.c :
	echo ''

main.o: main.c projet_y.h projet.h genCode.h
	$(CC) $(CFLAGS) -c main.c

.Phony: clean

clean:
	rm -f *~ projet.exe* ./projet *.o projet_l.* projet_y.* test_lex
	rm -f test/*~ test/*.out test/*.res test/*/*~ test/*/*.out test/*/*.res
