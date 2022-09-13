CC = g++
CFLAGS = -g -Werror -Wall
programs: lel

lel: main.o cell.o organism.o
	$(CC) $(CFLAGS) -o $@ $^


main.o: main.cpp cell.o
	$(CC) -c $(CFLAGS) main.cpp

cell.o: cell.cpp
	$(CC) -c $(CFLAGS) cell.cpp

organism.o: organism.cpp
	$(CC) -c $(CFLAGS) organism.cpp
