CC = g++
CFLAGS = -g -Werror -Wall
programs: lel

lel: main.o organism.o
	$(CC) $(CFLAGS) -lncurses -o $@ $^


main.o: main.cpp 
	$(CC) -c $(CFLAGS) main.cpp

organism.o: organism.cpp
	$(CC) -c $(CFLAGS) organism.cpp
