#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>

#include <SDL2/SDL.h>
#include <chrono>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

static volatile int running = 1;

void intHandler(int dummy)
{
	running = 0;
}

Board board = Board(256, 128);

void Render(SDL_Window *window, SDL_Renderer *renderer)
{
	// we'll use this texture as our own backbuffer
	static SDL_Texture *boardBuf = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
													 SDL_TEXTUREACCESS_TARGET, board.dim_x, board.dim_y);

	// draw to board buffer instead of backbuffer
	SDL_SetRenderTarget(renderer, boardBuf);

	for (int y = 0; y < board.dim_y; y++)
	{
		for (int x = 0; x < board.dim_x; x++)
		{
			if (board.DeltaCells[y][x])
			{
				board.DeltaCells[y][x] = false;
				Cell *thisCell = board.cells[y][x];
				switch (thisCell->type)
				{
				case cell_empty:
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
					break;

				case cell_plantmass:
					SDL_SetRenderDrawColor(renderer, 15, 40, 15, 255);
					break;

				case cell_biomass:
					SDL_SetRenderDrawColor(renderer, 150, 10, 10, 255);
					break;

				case cell_leaf:
					SDL_SetRenderDrawColor(renderer, 30, 120, 30, 255);
					break;

				case cell_bark:
					SDL_SetRenderDrawColor(renderer, 75, 25, 25, 255);
					break;

				case cell_mover:
					SDL_SetRenderDrawColor(renderer, 50, 120, 255, 255);
					break;

				case cell_herbivore_mouth:
					SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
					break;

				case cell_carnivore_mouth:
					SDL_SetRenderDrawColor(renderer, 255, 100, 150, 255);
					break;

				case cell_flower:
					SDL_SetRenderDrawColor(renderer, 50, 250, 150, 255);
					break;

				case cell_fruit:
					SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
					break;

				case cell_killer:
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					break;

				case cell_armor:
					SDL_SetRenderDrawColor(renderer, 175, 0, 255, 255);
					break;

				case cell_touch:
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					break;

				case cell_null:
					break;
				}
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}
	// reset render target, copy board buf, and spit it out to the screen
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, boardBuf, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
	srand(0);
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(2816, 1408, 0, &window, &renderer);
	// SDL_CreateWindowAndRenderer(2580, 1280, 0, &window, &renderer);
	SDL_RenderSetScale(renderer, 11, 11);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawPoint(renderer, 640 / 2, 640 / 2);

	SDL_RenderPresent(renderer);

	signal(SIGINT, intHandler);

	// refresh();
	Organism *firstOrganism = board.createOrganism(board.dim_x / 2, board.dim_y / 2);
	// Organism *firstOrganism = board.createOrganism(0, 0);
	// firstOrganism->AddCell(0, 0, new Cell_Bark());
	// int floweringIndex = randInt(0, 3);
	firstOrganism->AddCell(0, 0, new Cell_Leaf(0));
	// firstOrganism->AddCell(0, 0, new Cell_Mover());
	// firstOrganism->AddCell(0, 1, new Cell_Carnivore());
	firstOrganism->AddCell(1, 0, new Cell_Leaf(0));
	firstOrganism->AddCell(0, 1, new Cell_Leaf(0));
	firstOrganism->AddCell(1, 1, new Cell_Leaf(0));
	// Cell_Leaf plantLeaf = Cell_Leaf();
	/*
	if (firstOrganism->AddCell(0, 1, new Cell_Leaf()))
	{
		std::cerr << "Error adding cell!";
	}
	else
	{
		std::cout << "added leaf cell " << std::endl;
	}*/
	// Organism *realFirstOrganism = firstOrganism->Reproduce();
	firstOrganism->RecalculateStats();
	firstOrganism->lifespan = LIFESPAN_MULTIPLIER * firstOrganism->GetMaxEnergy();
	// firstOrganism->mutability = 50;
	firstOrganism->AddEnergy(firstOrganism->GetMaxEnergy());
	firstOrganism->Heal(100);
	firstOrganism->reproductionCooldown = 10;
	// board.Organisms.push_back(firstOrganism);

	SDL_Event e;
	bool autoplay = false;
	// getch();
	// clear();
	// refresh();
	auto lastFrame = std::chrono::high_resolution_clock::now();
	size_t autoplaySpeed = 1;
	int leftover = 0;
	int frameToRender = 1;
	while (running /* && board.tickCount < 100*/)
	{
		if (autoplay)
		{

			board.Tick();

			/*if (board.tickCount % (1000 / autoplaySpeed) == 0)
			{*/
			// if (board.tickCount % 10 == 0)
			// {
			if (board.tickCount % 50 == 0)
			{
				board.Stats();
			}
			if (board.tickCount % frameToRender == 0)
			{
				Render(window, renderer);
			}
			auto thisFrame = std::chrono::high_resolution_clock::now();
			auto diff = thisFrame - lastFrame;
			size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
			lastFrame = thisFrame;
			if (autoplaySpeed)
			{
				if (leftover > 1000000)
				{
					int leftovermillis = leftover / 1000000;
					SDL_Delay(leftovermillis);
					leftover = leftover - (leftovermillis * 1000000);
				}
				if (autoplaySpeed > (micros / 1000))
				{
					SDL_Delay(autoplaySpeed - (micros / 1000));
					leftover += micros - (micros / 1000);
				}
			}
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
					autoplay = false;
					frameToRender = 1;
					board.Tick();
					Render(window, renderer);
					board.Stats();
					break;

				case SDLK_UP:
					if (!autoplay)
					{
						autoplaySpeed = 1000;
						autoplay = true;
					}
					else
					{
						if (autoplaySpeed > 1)
						{
							autoplaySpeed /= 2;
							if (autoplaySpeed < 1)
							{
								autoplaySpeed = 1;
							}
						}
						else
						{
							autoplaySpeed = 0;
						}
					}
					printf("Delay %lums between frames\n", autoplaySpeed);
					break;

				case SDLK_DOWN:
					if (!autoplay)
					{
						autoplaySpeed = 1000;
						autoplay = true;
					}
					else
					{
						if (autoplaySpeed == 0)
						{
							autoplaySpeed = 1;
						}
						else
						{
							autoplaySpeed *= 2;
							if (autoplaySpeed > 1000)
							{
								autoplaySpeed = 1000;
							}
						}
					}
					printf("Delay %lums between frames\n", autoplaySpeed);
					break;

				case SDLK_RIGHT:
					frameToRender *= 2;
					printf("Rendering every %d tick(s)\n", frameToRender);
					break;

				case SDLK_LEFT:
					frameToRender /= 2;
					if (frameToRender < 1)
					{
						frameToRender = 1;
					}
					printf("Rendering every %d tick(s)\n", frameToRender);
					break;

				case SDLK_SPACE:
					// if not autoplaying, set max autoplay speed
					if (!autoplay)
					{
						autoplay = true;
						autoplaySpeed = 0;
						break;
					}

					// if autoplaying, fall through and stop the autoplay

				default:
					autoplay = false;
					frameToRender = 1;
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
