#include "util.h"
#include <boost/thread.hpp>
#include <chrono>
#include <SDL.h>
#include <cstdint>

#include "imgui.h"
#include "implot.h"
#include "board.h"

extern Board *board;
// extern TickratePID pidController;
extern volatile int running;
extern bool autoplay;
extern bool maxSpeed;
extern int ticksThisSecond;
extern int targetTickrate;
extern long int leftoverMicros;

const ImVec4 cellColors[cell_null] =
	{
		{0, 0, 0, 0},		  // empty
		{10, 40, 10, 255},	  // plantmass
		{150, 60, 60, 255},	  // biomass
		{30, 120, 30, 255},	  // leaf
		{75, 25, 25, 255},	  // bark
		{50, 250, 150, 255},  // flower
		{200, 200, 0, 255},	  // fruit
		{255, 150, 0, 255},	  // mouth
		{50, 120, 255, 255},  // mover
		{255, 0, 0, 255},	  // killer
		{175, 0, 255, 255},	  // armor
		{150, 150, 150, 255}, // touch
		{255, 255, 255, 255}  // eye

};

const ImVec4 classColors[class_null] =
	{
		{30.0 / 255, 120.0 / 255, 30.0 / 255, 255.0 / 255},	  // plant
		{255.0 / 255, 150.0 / 255, 0.0 / 255, 255.0 / 255},	  // mover
};

const char *cellNames[cell_null] =
	{"empty",
	 "plantmass",
	 "biomass",
	 "leaf",
	 "bark",
	 "flower",
	 "fruit",
	 "mouth",
	 "mover",
	 "killer",
	 "armor",
	 "touch",
	 "eye"};

const char *classNames[class_null] =
	{"plant",
	 "mover"};

const uint32_t cellXs[cell_null] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const double cellXs_double[cell_null] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};

ImPlotColormap CellColormap;
ImPlotColormap ClassColormap;

void AddImPlotColorMap()
{
	ImVec4 cellColorFractions[cell_null];
	memcpy(cellColorFractions, cellColors, cell_null * sizeof(ImVec4));
	for (int i = 0; i < cell_null; i++)
	{
		cellColorFractions[i].w /= 255.0;
		cellColorFractions[i].x /= 255.0;
		cellColorFractions[i].y /= 255.0;
		cellColorFractions[i].z /= 255.0;
	}
	CellColormap = ImPlot::AddColormap("CellColors", cellColorFractions, cell_null);
	ClassColormap = ImPlot::AddColormap("ClassColors", classColors, cell_null);
}

void SetColorForCell(SDL_Renderer *r, Cell *c)
{
	switch (c->type)
	{
	case cell_null:
		printf("Attempt to set color for null cell!\n");
		exit(1);
		break;

	default:
		const ImVec4 &thisColor = cellColors[c->type];
		SDL_SetRenderDrawColor(r, thisColor.x, thisColor.y, thisColor.z, thisColor.w);
	}
}

void DrawCell(SDL_Renderer *r, Cell *c, int x, int y)
{
	SetColorForCell(r, c);
	SDL_RenderDrawPoint(r, x, y);
	if (c->type == cell_eye)
	{
		Cell_Eye *thisEye = static_cast<Cell_Eye *>(c);
		SDL_RenderSetScale(r, 1.0, 1.0);
		int *eyeDirection = directions[thisEye->Direction()];
		SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
		SDL_RenderDrawPoint(r, (3 * x) + 1, (3 * y) + 1);
		SDL_RenderDrawPoint(r, (3 * x) + eyeDirection[0] + 1, (3 * y) + eyeDirection[1] + 1);
		SDL_RenderSetScale(r, 3.0, 3.0);
	}
}
