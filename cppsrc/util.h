class SDL_Renderer;
class Cell;
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

// void TickMain();

void AddImPlotColorMap();

void SetColorForCell(SDL_Renderer *r, Cell *c);
