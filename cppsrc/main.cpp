#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>

#include <SDL2/SDL.h>

#include "windowhandler.h"
#include "boardwindow.h"
#include "lifeforms.h"
#include "board.h"
#include "rng.h"

int scalefactor = 5;
static volatile int running = 1;
Board *board = nullptr;
void intHandler(int dummy)
{
	running = 0;
}

bool autoplay = false;
int frameToRender = 1;
size_t autoplaySpeed = 1000;

void RenderBoard(GameWindow *gw)
{
	// we'll use this texture as our own backbuffer
	static SDL_Texture *boardBuf = SDL_CreateTexture(gw->renderer(), SDL_PIXELFORMAT_RGB888,
													 SDL_TEXTUREACCESS_TARGET, board->dim_x, board->dim_y);

	// draw to board buffer instead of backbuffer
	SDL_SetRenderTarget(gw->renderer(), boardBuf);

	for (int y = 0; y < board->dim_y; y++)
	{
		for (int x = 0; x < board->dim_x; x++)
		{
			if (board->DeltaCells[y][x])
			{
				board->DeltaCells[y][x] = false;
				Cell *thisCell = board->cells[y][x];
				switch (thisCell->type)
				{
				case cell_empty:
					SDL_SetRenderDrawColor(gw->renderer(), 0, 0, 0, 0);
					break;

				case cell_plantmass:
					SDL_SetRenderDrawColor(gw->renderer(), 10, 40, 10, 255);
					break;

				case cell_biomass:
					SDL_SetRenderDrawColor(gw->renderer(), 150, 60, 60, 255);
					break;

				case cell_leaf:
					SDL_SetRenderDrawColor(gw->renderer(), 30, 120, 30, 255);
					break;

				case cell_bark:
					SDL_SetRenderDrawColor(gw->renderer(), 75, 25, 25, 255);
					break;

				case cell_mover:
					SDL_SetRenderDrawColor(gw->renderer(), 50, 120, 255, 255);
					break;

				case cell_herbivore_mouth:
					SDL_SetRenderDrawColor(gw->renderer(), 255, 150, 0, 255);
					break;

				case cell_carnivore_mouth:
					SDL_SetRenderDrawColor(gw->renderer(), 255, 100, 150, 255);
					break;

				case cell_flower:
					SDL_SetRenderDrawColor(gw->renderer(), 50, 250, 150, 255);
					break;

				case cell_fruit:
					SDL_SetRenderDrawColor(gw->renderer(), 200, 200, 0, 255);
					break;

				case cell_killer:
					SDL_SetRenderDrawColor(gw->renderer(), 255, 0, 0, 255);
					break;

				case cell_armor:
					SDL_SetRenderDrawColor(gw->renderer(), 175, 0, 255, 255);
					break;

				case cell_touch:
					SDL_SetRenderDrawColor(gw->renderer(), 255, 255, 255, 255);
					break;

				case cell_null:
					break;
				}
				SDL_RenderDrawPoint(gw->renderer(), x, y);
			}
		}
	}
	// reset render target, copy board buf, and spit it out to the screen
	SDL_SetRenderTarget(gw->renderer(), NULL);
	SDL_RenderCopy(gw->renderer(), boardBuf, NULL, NULL);
	SDL_RenderPresent(gw->renderer());
}

#define BOARD_X 192
#define BOARD_Y 192

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	signal(SIGINT, intHandler);
	WindowingSystem ws;
	std::string name = std::string("LifeEngineLite");
	BoardWindow *boardWindow = static_cast<BoardWindow *>(ws.Add(new BoardWindow(BOARD_X * scalefactor, BOARD_Y * scalefactor, name, scalefactor)));
	board = new Board(BOARD_X, BOARD_Y);
	boardWindow->SetBoard(board);
	printf("created board with dimension %d %d\n", board->dim_x, board->dim_y);
	// srand(0);

	// SDL_Window *window = nullptr;
	// SDL_Renderer *renderer = nullptr;

	// refresh();
	Organism *firstOrganism = board->createOrganism(board->dim_x / 2, board->dim_y / 2);
	firstOrganism->AddCell(0, 0, new Cell_Leaf(0));
	firstOrganism->AddCell(1, 0, new Cell_Leaf(0));
	firstOrganism->lifespan = 5000;

	firstOrganism->RecalculateStats();
	firstOrganism->lifespan = LIFESPAN_MULTIPLIER * firstOrganism->GetMaxEnergy();
	firstOrganism->mutability = 25;
	firstOrganism->AddEnergy(firstOrganism->GetMaxEnergy());
	firstOrganism->Heal(100);
	firstOrganism->reproductionCooldown = 10;
	firstOrganism->species = board->GetNextSpecies();
	board->AddSpeciesMember(firstOrganism->species);

	SDL_Event e;
	while (running /* && board->tickCount < 100*/)
	{
		while(SDL_PollEvent(&e))
		{
			ws.HandleEvent(e);
		}
		ws.Tick();
	}

	unsigned int finalSpeciesCount = board->GetNextSpecies();
	printf("\n");
	printf("%u species have ever lived\n", finalSpeciesCount);

	unsigned int largestSpecies = 0, largestSpeciesPopulation = 0;
	double totalMaxSpeciesSize = 0;
	int speciesSizeCounts[16] = {0};
	for(unsigned int i = 1; i < finalSpeciesCount; i++)
	{
		if(board->peakSpeciesCounts[i] > largestSpeciesPopulation)
		{
			largestSpecies = i;
			largestSpeciesPopulation = board->peakSpeciesCounts[i];
		}
		totalMaxSpeciesSize += board->peakSpeciesCounts[i];
		int index = 0;
		int count = board->peakSpeciesCounts[i];
		while(count > 1)
		{
			index++;
			count /= 2;
		}
		speciesSizeCounts[index]++;
	}
	totalMaxSpeciesSize /= finalSpeciesCount;
	printf("The largest species ever (%u) had %u concurrent living members\n", largestSpecies, largestSpeciesPopulation);
	printf("The average species peaked at %.2f members\n", totalMaxSpeciesSize);
	printf("Species size breakdown:\n");
	for(int i = 0; i < 16; i++)
	{
		int len = printf("\t%d..%d", (int)pow(2, i), (int)(pow(2, i + 1) - 1));
		while(len++ < 15)
		{
			printf(" ");
		}
		printf(":%2.2f%% (%d)\n", 100.0 * (speciesSizeCounts[i] / (double)finalSpeciesCount), speciesSizeCounts[i]);
	}
	// SDL_DestroyWindow(window);
	// SDL_DestroyRenderer(renderer);

	// move(0, 0);
	// mvprintw(0, 0, "Press any key to exit");
	// refresh();

	// getch();
	// endwin();
}
