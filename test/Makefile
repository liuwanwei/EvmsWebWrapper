TGT=libtest.so
OBJ=test.o


all: 
	gcc -g -Wall -fPIC -c test.c
	gcc -g -Wall -fPIC -c main.c
	ld  -shared  -fPIC -o $(TGT) -lc $(OBJ)
	gcc -g -Wall main.o -o main $(TGT) -ltest

clean:
	rm $(OBJ) $(TGT)
