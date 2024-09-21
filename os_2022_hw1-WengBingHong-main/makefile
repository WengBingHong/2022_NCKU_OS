CC = gcc
FLAGS = -Wall -g #-std=c11

all: my_shell

my_shell: 
	$(CC) $(FLAGS) -o my_shell my_shell.c -lreadline

.PHONY: clean
clean_exc:
	rm -f my_shell
clean:
	rm -f *.o 
