class SDL_Renderer;
class Cell;

#include "implot.h"
#include "config.h"
#include <cstdint>
/*
class TickratePID
{
	float previous_error = 0.0;
	float Kp = 0.00024;
	float Ki = 0.00002;
	float Kd = 0.0000001;

public:
	float Tick(float instanteneousMeasurement);
};*/

#ifndef CellColormap
extern const ImVec4 cellColors[cell_null];
extern ImPlotColormap CellColormap;
extern ImPlotColormap ClassColormap;
extern const char *cellNames[cell_null];
extern const char *classNames[class_null];
extern const uint32_t cellXs[cell_null];
extern const double cellXs_double[cell_null];
#endif

// void TickMain();

void AddImPlotColorMap();

void SetColorForCell(SDL_Renderer *r, Cell *c);

void DrawCell(SDL_Renderer *r, Cell *c, int x, int y);
