CC = g++
CFLAGS = -g -Werror -Wall
programs: lel

lel: main.o organism.o cell.o board.o
	$(CC) $(CFLAGS) -lncurses -o $@ $^


main.o: main.cpp 
	$(CC) -c $(CFLAGS) main.cpp

board.o: board.cpp
	$(CC) -c $(CFLAGS) board.cpp

cell.o: cell.cpp
	$(CC) -c $(CFLAGS) cell.cpp

organism.o: organism.cpp
	$(CC) -c $(CFLAGS) organism.cpp

clean:
	rm *.o
	rm ./lel
