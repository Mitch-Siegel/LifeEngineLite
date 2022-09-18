#include <stdlib.h>
#include <iostream>
#include <curses.h>
#include <signal.h>
#include <vector>

#include "lifeforms.h"
#include "board.h"

static volatile int running = 1;

void intHandler(int dummy)
{
	running = 0;
}
// Cell *board[BOARD_DIM][BOARD_DIM];

Board board = Board(32, 32);


int main(int argc, char *argv[])
{
	// srand(0x134134u);
	initscr();
	start_color();
	init_pair(10, COLOR_WHITE, COLOR_BLACK);

	init_pair(cell_empty, COLOR_WHITE, COLOR_BLACK);
	init_pair(cell_food, COLOR_WHITE, COLOR_BLUE);
	init_pair(cell_leaf, COLOR_WHITE, COLOR_GREEN);
	init_pair(cell_herbivore_mouth, COLOR_WHITE, COLOR_YELLOW);
	init_pair(cell_flower, COLOR_WHITE, COLOR_CYAN);
	init_pair(cell_mover, COLOR_WHITE, COLOR_BLUE);
	
	signal(SIGINT, intHandler);

	refresh();
	Organism *firstOrganism = board.createOrganism(5, 5);
	firstOrganism->energy = 1;
	// Cell_Leaf plantLeaf = Cell_Leaf();
	if (firstOrganism->AddCell(0, 0, new Cell_Leaf()))
	{
		std::cerr << "Error adding cell!";
	}
	else
	{
		firstOrganism->lifespan = 100;
		std::cout << "added leaf cell " << std::endl;
	}
	getch();
	clear();
	refresh();

	while (running)
	{
		erase();
		board.Tick();

		for (int y = 0; y < board.dim_y; y++)
		{
			move(y, 0);
			for (int x = 0; x < board.dim_x; x++)
			{
				Cell *thisCell = board.cells[y][x];
				attron(COLOR_PAIR((int)thisCell->type));
				switch (thisCell->type)
				{
				case cell_empty:
					addch(' ');
					break;

				case cell_food:
					addch('F');
					break;

				case cell_leaf:
					addch('L');
					break;

				case cell_mover:
				case cell_herbivore_mouth:
					addch('M');
					break;

				case cell_flower:
					addch('*');
					break;

				case cell_null:
					addch('_');
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
