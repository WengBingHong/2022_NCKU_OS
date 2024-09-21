TARGET 	= scheduler_simulator
CC     	= gcc
FLAGS  	= -Wall -lpthread
OBJ    	= builtin.o command.o shell.o function.o resource.o task.o
INCLUDE = ./include/
SRC		= ./src/

all: $(TARGET) 

$(TARGET): main.c $(OBJ) 
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ) $<

%.o: ${SRC}%.c ${INCLUDE}%.h
	$(CC) $(FLAGS) -c $<

.PHONY: clean
clean:
	rm -f ${TARGET} *.o out*
clean_obj:
	rm -f *.o
