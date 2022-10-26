CC = g++
CFLAGS = -g -Werror -Wall -O2
BACKEND = ./imgui-1.88/backends/imgui_impl_sdl.cpp ./imgui-1.88/backends/imgui_impl_sdlrenderer.cpp
programs: lel

#c++ `sdl2-config --cflags` -I .. -I ../.. main.cpp 
# ../../backends/imgui_impl_sdl.cpp ../../backends/imgui_impl_sdlrenderer.cpp 
# ../../imgui*.cpp `sdl2-config --libs` -lGL
lel: main.o windowhandler.o boardwindow.o organismwindow.o organism.o cell.o board.o rng.o brain.o
	$(CC) $(CFLAGS) `sdl2-config --cflags` -o $@ $^ `sdl2-config --libs`


main.o: main.cpp 
	$(CC) -c $(CFLAGS) main.cpp $(BACKEND)

windowhandler.o: windowhandler.cpp
	$(CC) -c $(CFLAGS) windowhandler.cpp

boardwindow.o: boardwindow.cpp
	$(CC) -c $(CFLAGS) boardwindow.cpp

organismwindow.o: organismwindow.cpp
	$(CC) -c $(CFLAGS) organismwindow.cpp

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
