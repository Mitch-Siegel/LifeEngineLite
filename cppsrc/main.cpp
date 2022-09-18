#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>

#include <SDL2/SDL.h>

#include "lifeforms.h"
#include "board.h"

static volatile int running = 1;

void intHandler(int dummy)
{
	running = 0;
}
// Cell *board[BOARD_DIM][BOARD_DIM];

Board board = Board(256, 128);

void Render(SDL_Window *window, SDL_Renderer *renderer)
{

	for (int y = 0; y < board.dim_y; y++)
	{
		for (int x = 0; x < board.dim_x; x++)
		{
			// if (board.DeltaCells[(y * board.dim_y) + x])
			// {
				Cell *thisCell = board.cells[y][x];
				// attron(COLOR_PAIR((int)thisCell->type));
				switch (thisCell->type)
				{
				case cell_empty:
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
					break;

				case cell_biomass:
					SDL_SetRenderDrawColor(renderer, 25, 25, 100, 255);
					break;

				case cell_leaf:
					SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
					break;

				case cell_mover:
					SDL_SetRenderDrawColor(renderer, 70, 70, 255, 255);
					break;

				case cell_herbivore_mouth:
					SDL_SetRenderDrawColor(renderer, 200, 175, 0, 255);
					break;

				case cell_flower:
					SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
					break;

				case cell_null:
					break;
				}
				SDL_RenderDrawPoint(renderer, x, y);
				// board.DeltaCells[(y * board.dim_y) + x] = false;
			// }
			// attron(COLOR_PAIR(10));
		}
	}
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{

	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(1024, 512, 0, &window, &renderer);
	SDL_RenderSetScale(renderer, 4, 4);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawPoint(renderer, 640 / 2, 640 / 2);

	SDL_RenderPresent(renderer);

	signal(SIGINT, intHandler);

	// refresh();
	Organism *firstOrganism = board.createOrganism(10, 10);

	// Cell_Leaf plantLeaf = Cell_Leaf();
	if (firstOrganism->AddCell(0, 0, new Cell_Leaf()))
	{
		std::cerr << "Error adding cell!";
	}
	else
	{
		std::cout << "added leaf cell " << std::endl;
	}
	Organism *realFirstOrganism = firstOrganism->Reproduce();
	realFirstOrganism->AddEnergy(1);
	board.Organisms.push_back(realFirstOrganism);

	SDL_Event e;
	bool autoplay = false;
	// getch();
	// clear();
	// refresh();

	while (running)
	{
		if (autoplay)
		{
			board.Tick();

			if (board.tickCount % 100 == 0)
			{
				Render(window, renderer);
			}
			// SDL_Delay(1);
		}

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
			if (e.type == SDL_KEYDOWN)
			{

				switch (e.key.keysym.sym)
				{
				case SDLK_RETURN:
					board.Tick();
					Render(window, renderer);
					break;

				default:
					autoplay = !autoplay;
					break;
				}
			}
		}

		// SDL_Delay(1000);
		// refresh();
		// attron(COLOR_PAIR(10));
		// getch();
	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	// move(0, 0);
	// mvprintw(0, 0, "Press any key to exit");
	// refresh();

	// getch();
	// endwin();
}
