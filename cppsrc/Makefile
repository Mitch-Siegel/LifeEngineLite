CC = g++
CFLAGS = -g -Werror -Wall -O2
programs: lel

lel: main.o organism.o cell.o board.o rng.o brain.o
	$(CC) $(CFLAGS) -lSDL2 -o $@ $^


main.o: main.cpp 
	$(CC) -c $(CFLAGS) main.cpp

board.o: board.cpp
	$(CC) -c $(CFLAGS) board.cpp

cell.o: cell.cpp
	$(CC) -c $(CFLAGS) cell.cpp

organism.o: organism.cpp
	$(CC) -c $(CFLAGS) organism.cpp

brain.o: brain.cpp
	$(CC) -c $(CFLAGS) brain.cpp


rng.o: rng.cpp
	$(CC) -c $(CFLAGS) rng.cpp

clean:
	rm *.o
	rm ./lel
