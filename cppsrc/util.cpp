#include "util.h"
#include <boost/thread.hpp>
#include <chrono>
#include <SDL2/SDL.h>

#include <imgui.h>
#include "board.h"

extern Board *board;
// extern TickratePID pidController;
extern volatile int running;
extern bool autoplay;
extern bool maxSpeed;
extern int ticksThisSecond;
extern int targetTickrate;
extern long int leftoverMicros;
/*
float TickratePID::Tick(float instanteneousMeasurement)
{
	if (instanteneousMeasurement == 0.0)
	{
		instanteneousMeasurement = 1.0;
	}
	// printf("Current instanteneous framerate: %f\n", instanteneousMeasurement);
	float framerate = ImGui::GetIO().Framerate;
	float error = ((1000000.0 / targetTickrate) - instanteneousMeasurement);
	if (maxSpeed)
	{
		error -= (10000.0 * (59.99 - framerate));
	}
	float dt = 1.0 / targetTickrate;
	// printf("DT is % .8f, error is % .8f\n", dt, error);
	// printf("Error * dt is %f\n", error * dt);
	float derivative = (error - previous_error) / dt;
	float delta = Kp * error + Ki * leftoverMicros + Kd * derivative;
	// printf("P:% .8f I:% .8f D:% .8f\n", error * Kp, leftoverMicros * Ki, derivative * Kd);
	// printf("PID Delta returned: % .8f\n\n", delta);
	previous_error = error;
	return delta;
}
*/

/*
void TickMain()
{
	if (board == nullptr)
	{
		printf("TickMain called with null board!\nBoard must be instantiated first!\n");
		return;
	}

	auto lastFrame = std::chrono::high_resolution_clock::now();
	while (running)
	{
		if (autoplay)
		{
			board->Tick();
			ticksThisSecond++;
			auto frameEnd = std::chrono::high_resolution_clock::now();
			auto diff = frameEnd - lastFrame;
			size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
			int leftoverThisStep = static_cast<int>((1000000.0 / targetTickrate) - micros);
			leftoverMicros += leftoverThisStep;
			// lastFrame = frameEnd;
			if (leftoverMicros > 5000)
			{
				if (maxSpeed)
				{
					targetTickrate += pidController.Tick(micros);
				}
				int delayDuration = leftoverMicros / 1000;
				boost::this_thread::sleep(boost::posix_time::milliseconds(delayDuration));
				leftoverMicros -= delayDuration * 1000;
				auto delayEnd = std::chrono::high_resolution_clock::now();
				auto delayDiff = delayEnd - frameEnd;
				size_t actualDelayMicros = std::chrono::duration_cast<std::chrono::microseconds>(delayDiff).count();
				leftoverMicros -= (delayDuration * 1000) - static_cast<int>(actualDelayMicros);
				lastFrame = delayEnd;
			}
			else
			{
				if ((leftoverMicros < -10 * (1000000.0 / targetTickrate)) || maxSpeed)
				{
					targetTickrate += pidController.Tick(micros);
					if (targetTickrate < 1.0)
					{
						targetTickrate = 1.0;
					}
				}
				lastFrame = frameEnd;
			}
		}
		else
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
}
*/
void SetColorForCell(SDL_Renderer *r, Cell *c)
{
	switch (c->type)
	{
	case cell_empty:
		SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
		break;

	case cell_plantmass:
		SDL_SetRenderDrawColor(r, 10, 40, 10, 255);
		break;

	case cell_biomass:
		SDL_SetRenderDrawColor(r, 150, 60, 60, 255);
		break;

	case cell_leaf:
		SDL_SetRenderDrawColor(r, 30, 120, 30, 255);
		break;

	case cell_bark:
		SDL_SetRenderDrawColor(r, 75, 25, 25, 255);
		break;

	case cell_mover:
		SDL_SetRenderDrawColor(r, 50, 120, 255, 255);
		break;

	case cell_herbivore_mouth:
		SDL_SetRenderDrawColor(r, 255, 150, 0, 255);
		break;

	case cell_carnivore_mouth:
		SDL_SetRenderDrawColor(r, 255, 100, 150, 255);
		break;

	case cell_flower:
		SDL_SetRenderDrawColor(r, 50, 250, 150, 255);
		break;

	case cell_fruit:
		SDL_SetRenderDrawColor(r, 200, 200, 0, 255);
		break;

	case cell_killer:
		SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
		break;

	case cell_armor:
		SDL_SetRenderDrawColor(r, 175, 0, 255, 255);
		break;

	case cell_touch:
		SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
		break;

	case cell_null:
		break;
	}
}