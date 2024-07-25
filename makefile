CC = gcc -Wall
NAME = test
LIBNAME = libhlib
SRC = *.c

all:
	$(CC) -o $(NAME) $(SRC)

debug:
	$(CC) -o $(NAME) $(SRC) -g

lib:
	gcc -c *.c
	ar rcs $(LIBNAME).a *.o
	rm *.o

git:
	git add hallocator.c hallocator.h makefile README.md test.c
	git commit -m "Updating files"
	git push

clean:
	rm $(NAME) $(LIBNAME).a
