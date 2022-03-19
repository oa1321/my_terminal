CC = gcc
AR = ar
FLAGS = -Wall -g
SER = server.o
TER = terminal.o

all: server terminal

server: server.o
	$(CC) $(FLAGS) -o server server.o

server.o: server.c
	$(CC) $(FLAGS) -c server.c

terminal: Terminal.o
	$(CC) $(FLAGS) -o terminal Terminal.o

terminal.o: Terminal.c
	$(CC) $(FLAGS) -c Terminal.c

clean:
	rm -f *.o terminal server
