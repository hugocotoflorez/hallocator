CC = gcc -Wall
NAME = test
SRC = *.c

all:
	$(CC) -o $(NAME) $(SRC)

debug:
	$(CC) -o $(NAME) $(SRC) -g

lib:
	gcc -c *.c
	ar rcs libhlib.a *.o
	rm *.o
