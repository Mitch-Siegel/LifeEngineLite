#include <stdlib.h>
#include <iostream>
#include <curses.h>
#include <signal.h>
#include <vector>

#include "organism.h"

static volatile int running = 1;

void intHandler(int dummy)
{
	running = 0;
}
Cell* board[BOARD_DIM][BOARD_DIM];

int main(int argc, char *argv[])
{
	srand(0);
	initscr();
	start_color();
	init_pair(10, COLOR_WHITE, COLOR_BLACK);
	/*
	init_pair(cell_empty, COLOR_WHITE, COLOR_BLACK);
	init_pair(cell_food, COLOR_BLUE, COLOR_BLACK);
	init_pair(cell_producer, COLOR_GREEN, COLOR_BLACK);
	init_pair(cell_mouth, COLOR_YELLOW, COLOR_BLACK);
	*/
	init_pair(cell_empty, COLOR_BLACK, COLOR_BLACK);
	init_pair(cell_food, COLOR_BLUE, COLOR_BLUE);
	init_pair(cell_producer, COLOR_GREEN, COLOR_GREEN);
	init_pair(cell_mouth, COLOR_YELLOW, COLOR_YELLOW);
	signal(SIGINT, intHandler);
	// board = new Cell *[BOARD_DIM];
	for (int y = 0; y < BOARD_DIM; y++)
	{
		for (int x = 0; x < BOARD_DIM; x++)
		{
			board[y][x] = new Cell(x, y, cell_empty, NULL);
		}
	}
	printw("Hello, World!");
	refresh();

	Organism *plant = new Organism(5, 5);
	plant->energy = 31;
	plant->AddCell(0, 0, cell_mouth);
	plant->AddCell(1, 1, cell_producer);

	std::vector<Organism *> Organisms;
	Organisms.push_back(plant);

	while (running)
	{
		erase();
		for (size_t i = 0; i < Organisms.size(); i++)
		{
			if (!Organisms[i]->alive)
			{
				Organisms.erase(Organisms.begin() + i);
			}
			else
			{
				Organism *replicated = Organisms[i]->Tick();
				mvprintw(BOARD_DIM + i, 0, "%d %d: %d energy, %d cells (%d ticks old)", 
				Organisms[i]->x, Organisms[i]->y, Organisms[i]->energy, Organisms[i]->nCells, Organisms[i]->age);
				if(replicated != nullptr)
				{
					Organisms.push_back(replicated);
				}
			}
		}

		for (int y = 0; y < BOARD_DIM; y++)
		{
			move(y, 0);
			for (int x = 0; x < BOARD_DIM; x++)
			{
				Cell *thisCell = board[x][y];
				attron(COLOR_PAIR((int)thisCell->type));
				switch (thisCell->type)
				{
				case cell_empty:
					addch(' ');
					break;

				case cell_food:
					addch('F');
					break;

				case cell_producer:
					addch('P');
					break;

				case cell_mouth:
					addch('M');
					break;
				}
		attron(COLOR_PAIR(10));

			}
		}
		refresh();
		attron(COLOR_PAIR(10));

		getch();
	}

	move(0, 0);
	mvprintw(0, 0, "Press any key to exit");
	refresh();

	getch();
	endwin();
}
