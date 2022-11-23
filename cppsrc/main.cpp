#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>
#include <chrono>
#include <boost/thread.hpp>

#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include "implot.h"

#include "lifeforms.h"
#include "board.h"
#include "organismview.h"
#include "detailedstats.h"
#include "rng.h"
#include "util.h"

#define MIN_EXTRA_MICROS 100
static volatile int running = 1;
Board *board = nullptr;
void intHandler(int dummy)
{
	running = 0;
}

float scaleFactor = 4.0;
int x_off = 0;
int y_off = 0;
int winX, winY;
float targetTickrate = 10;
long int leftoverMicros = 0;
bool autoplay = false;
bool maxSpeed = false;
int ticksThisSecond = 0;

class TickratePID
{
	float previous_error = 0.0;
	float Kp = 0.000024;
	float Ki = 0.0002;
	float Kd = 0.00000001;

public:
	float Tick(float instanteneousMeasurement)
	{
		if (instanteneousMeasurement == 0.0)
		{
			instanteneousMeasurement = 1.0;
		}
		// printf("Current instanteneous framerate: %f\n", instanteneousMeasurement);
		float error = ((1000000.0 / targetTickrate) - instanteneousMeasurement);
		float dt = 1.0 / targetTickrate;
		// printf("DT is % .8f, error is % .8f\n", dt, error);
		// printf("Error * dt is %f\n", error * dt);
		float derivative = (error - previous_error) / dt;
		float delta = Kp * error + Ki * (leftoverMicros + 100) + Kd * derivative;
		// printf("P:% .8f I:% .8f D:% .8f\n", error * Kp, leftoverMicros * Ki, derivative * Kd);
		// printf("PID Delta returned: % .8f\n\n", delta);
		previous_error = error;
		return delta;
	}
};

TickratePID pidController;

template <class T>
class DataTracker
{
private:
	int maxSamples = 0;
	T *data;
	int dataP = 0;

public:
	DataTracker(int maxSamples)
	{
		this->maxSamples = maxSamples;
		this->data = new T[maxSamples * 2];
	}

	~DataTracker()
	{
		delete[] this->data;
	}

	void Add(T value)
	{
		this->data[this->dataP] = value;
		if (++this->dataP >= (maxSamples * 2))
		{
			for (int i = 0; i < maxSamples; i++)
			{
				this->data[i] = this->data[i + this->maxSamples];
			}
			this->dataP = maxSamples;
		}
	}

	T const *rawData()
	{
		int dataStartP = dataP - maxSamples;
		if (dataStartP < 0)
		{
			dataStartP = 0;
		}
		return data + dataStartP;
	}

	size_t size()
	{
		if (dataP > maxSamples)
		{
			return maxSamples;
		}
		else
		{
			return dataP;
		}
	}
};

inline void DrawCell(SDL_Renderer *r, Cell *c, int x, int y)
{
	SetColorForCell(r, c);
	SDL_RenderDrawPoint(r, x, y);
	// SDL_RenderDrawPoint(r, x + (x_off / scaleFactor), y + (y_off / scaleFactor));
}

class Stats
{
private:
	enum Counts
	{
		count_cells,
		count_energy,
		count_maxenergy,
		count_age,
		count_lifespan,
		count_mutability,
		count_maxconviction,
		count_rotatevschange,
		count_turnwhenrotate,
		count_raw,
		count_null
	};
	double organismStats[class_null][count_null] = {{0.0}};
	int classCounts[class_null] = {0};
	double organismCellCounts[class_null][cell_null] = {{0.0}};
	double touchSensorHaverCounts[class_null] = {0.0};
	// static double touchSensorIntervals[class_null] = {0.0};
	double cellSentiments[class_null][cell_null] = {{0.0}};

	DataTracker<int> tickData = DataTracker<int>(10000);
	DataTracker<int> organismCountData = DataTracker<int>(10000);
	DataTracker<int> plantCountData = DataTracker<int>(10000);
	DataTracker<int> herbCountData = DataTracker<int>(10000);
	DataTracker<int> carnCountData = DataTracker<int>(10000);
	DataTracker<int> omniCountData = DataTracker<int>(10000);

public:
	void Display()
	{
		const char *classNames[class_null] = {"Plant", "Herbivore", "Carnivore", "Omnivore"};
		const char *rowNames[count_null] = {"Class:", "Count", "Cells", "Energy%", "Max Energy", "Age", "Lifespan", "Mutability", "Max Conviction", "Rotate vs. change"};
		if (ImGui::BeginTable("OrganismStats", class_null + 1))
		{
			int row = 0;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%s", classNames[i]);
			}
			row++;

			// organism counts
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%d", classCounts[i]);
			}
			row++;

			// cell counts
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.1f", organismStats[i][count_cells]);
			}
			row++;

			// energy %
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.0f%%", 100.0 * (organismStats[i][count_energy] / organismStats[i][count_maxenergy]));
			}
			row++;

			// max energy
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.0f", organismStats[i][count_maxenergy]);
			}
			row++;

			// age
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.0f", 100.0 * (organismStats[i][count_age] / organismStats[i][count_lifespan]));
			}
			row++;

			// lifespan
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.0f", organismStats[i][count_lifespan]);
			}
			row++;

			// mutability
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.1f", organismStats[i][count_mutability]);
			}
			row++;

			ImGui::EndTable();
		}

		// ImPlot::SetNextAxesLimits(0, organismCountData.size(), 0, static_cast<double>(maxOrganisms));
		ImPlot::SetNextAxesToFit();
		// ImPlot::BeginPlot("Bar Graph##Line", "Day", NULL, ImVec2(-1, 0), ImPlotFlags_NoLegend | ImPlotFlags_NoBoxSelect | ImPlotFlags_AntiAliased, ImPlotAxisFlags_Time, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit)
		if (ImPlot::BeginPlot("Organism Counts by Classification", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit))
		{
			ImPlot::PushColormap(ClassColormap);
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
			ImPlot::PlotLine("Plants", tickData.rawData(), plantCountData.rawData(), static_cast<int>(organismCountData.size()));
			ImPlot::PlotLine("Herbivores", tickData.rawData(), herbCountData.rawData(), static_cast<int>(organismCountData.size()));
			ImPlot::PlotLine("Carnivores", tickData.rawData(), carnCountData.rawData(), static_cast<int>(organismCountData.size()));
			ImPlot::PlotLine("Omnivores", tickData.rawData(), omniCountData.rawData(), static_cast<int>(organismCountData.size()));
			ImPlot::PopColormap();
			ImPlot::PlotLine("Total Organisms", tickData.rawData(), organismCountData.rawData(), static_cast<int>(organismCountData.size()));
			ImPlot::EndPlot();
		}
	}

	void Update()
	{
		board->GetMutex();
		for (int i = 0; i < class_null; i++)
		{
			for (int j = 0; j < count_null; j++)
			{
				organismStats[i][j] = 0.0;
			}
			classCounts[i] = 0;
			for (int j = 0; j < class_null; j++)
			{
				organismCellCounts[i][j] = 0.0;
			}

			// touchSensorIntervals[i] = 0.0;

			for (int j = 0; j < cell_null; j++)
			{
				cellSentiments[i][j] = 0.0;
			}
		}
		for (Organism *o : board->Organisms)
		{
			enum OrganismClassifications thisClass = board->GetSpeciesInfo(o->species).classification;
			classCounts[thisClass]++;

			organismStats[thisClass][count_cells] += o->nCells();
			organismStats[thisClass][count_energy] += o->GetEnergy();
			organismStats[thisClass][count_maxenergy] += o->GetMaxEnergy();

			organismStats[thisClass][count_age] += o->age;
			organismStats[thisClass][count_lifespan] += o->lifespan;
			organismStats[thisClass][count_mutability] += o->mutability;
			organismStats[thisClass][count_maxconviction] += o->brain.maxConviction;
			organismStats[thisClass][count_rotatevschange] += o->brain.rotatevschange;
			organismStats[thisClass][count_turnwhenrotate] += o->brain.turnwhenrotate;
			organismStats[thisClass][count_raw]++;
			for (int i = 0; i < cell_null; i++)
			{
				organismCellCounts[thisClass][i] += o->cellCounts[i];
			}
			if (o->cellCounts[cell_touch] > 0)
			{
				touchSensorHaverCounts[thisClass]++;
				for (int i = 0; i < cell_null; i++)
				{
					cellSentiments[thisClass][i] += o->brain.cellSentiments[i];
				}
			}
		}
		board->ReleaseMutex();
		for (int i = 0; i < class_null; i++)
		{
			int thisClassSize = classCounts[i];
			for (int j = 0; j < count_null; j++)
			{
				organismStats[i][j] /= thisClassSize;
			}
		}

		tickData.Add(board->tickCount);
		organismCountData.Add(board->Organisms.size());
		plantCountData.Add(classCounts[class_plant]);
		herbCountData.Add(classCounts[class_herbivore]);
		carnCountData.Add(classCounts[class_carnivore]);
		omniCountData.Add(classCounts[class_omnivore]);
	}
};
Stats stats;

bool forceRedraw = false;
float RenderBoard(SDL_Renderer *r, size_t frameNum)
{
	// we'll use this texture as a seperate backbuffer
	static SDL_Texture *boardBuf = nullptr;

	if (boardBuf == nullptr)
	{
		boardBuf = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB888,
									 SDL_TEXTUREACCESS_TARGET, board->dim_x, board->dim_y);
	}

	int x_src, y_src, w_src, h_src;
	int x_dst, y_dst, w_dst, h_dst;

	x_src = x_off / scaleFactor;
	y_src = y_off / scaleFactor;

	x_dst = 0;
	y_dst = 0;
	w_dst = winX;
	h_dst = winY;

	if (x_src < 0)
	{
		// w_src += (x_src / scaleFactor);
		x_src = 0;
	}

	if (y_src < 0)
	{
		// h_src += (y_src / scaleFactor);
		y_src = 0;
	}
	if (board->dim_x * scaleFactor <= winX)
	{
		w_src = (winX - x_src) / scaleFactor;
	}
	else
	{
		w_src = (winX / scaleFactor);
	}
	if(w_src > board->dim_x)
	{
		w_src = board->dim_x;
	}

	if (board->dim_y * scaleFactor <= winY)
	{
		h_src = (winY - y_src) / scaleFactor;
	}
	else
	{
		h_src = (winY / scaleFactor);
	}
	if(h_src > board->dim_y)
	{
		h_src = board->dim_y;
	}

	w_dst = w_src * scaleFactor;
	h_dst = h_src * scaleFactor;

	// forceRedraw = true;
	SDL_Rect srcRect = {x_src, y_src, w_src, h_src};
	SDL_Rect dstRect = {x_dst, y_dst, w_dst, h_dst};

	printf("Mapping  % 3d,% 3d, % 3dx% 3d\n@(%2.0f)x to % 3d,% 3d, % 3dx% 3d\n\n", x_src, y_src, w_src, h_src, scaleFactor, x_dst, y_dst, w_dst, h_dst);

	if ((board->DeltaCells.size() == 0 && !forceRedraw) ||
		(leftoverMicros < 0))
	{
		SDL_RenderCopy(r, boardBuf, &srcRect, &dstRect);
		return -1.0;
	}

	/*
	 * don't bother waiting around for the mutex if trying to run at max speed
	 * waiting for the mutex will decrease the frame rate and this kick down the tick speed from the PID controller
	 */
	if (!maxSpeed)
	{
		board->GetMutex();
	}
	else
	{
		if (!board->TryGetMutex())
		{
			SDL_RenderCopy(r, boardBuf, &srcRect, &dstRect);
			return -1.0;
		}
	}

	// draw to board buffer instead of backbuffer
	SDL_SetRenderTarget(r, boardBuf);
	size_t cellsModified = 0;

	if (forceRedraw)
	{
		// SDL_RenderClear(r);
		for (int y = 0; y < board->dim_y; y++)
		{
			if (y + (y_off / scaleFactor) < 0 || ((y - board->dim_y) * scaleFactor) + y_off > winY)
			{
				continue;
			}
			for (int x = 0; x < board->dim_x; x++)
			{
				if (x + (x_off / scaleFactor) < 0 || ((x - board->dim_x) * scaleFactor) + x_off > winX)
				{
					continue;
				}
				Cell *thisCell = board->cells[y][x];
				DrawCell(r, thisCell, x, y);
			}
		}
		cellsModified = -1;
	}
	else
	{
		cellsModified = board->DeltaCells.size();
		for (std::pair<int, int> coordinate : board->DeltaCells)
		{
			int x = coordinate.first;
			int y = coordinate.second;
			DrawCell(r, board->cells[y][x], x, y);
		}
		board->DeltaCells.clear();
	}

	board->ReleaseMutex();
	forceRedraw = false;
	SDL_SetRenderTarget(r, NULL);
	SDL_RenderCopy(r, boardBuf, &srcRect, &dstRect);
	return static_cast<float>(cellsModified) / (board->dim_x * board->dim_y);
}

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
			if (board->Tick())
			{
				continue;
			}
			ticksThisSecond++;
			if (board->tickCount % 10 == 0)
			{
				stats.Update();
			}
			auto frameEnd = std::chrono::high_resolution_clock::now();
			auto diff = frameEnd - lastFrame;
			size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
			int leftoverThisStep = static_cast<int>((1000000.0 / targetTickrate) - micros);
			leftoverMicros += leftoverThisStep;
			// lastFrame = frameEnd;
			if (leftoverMicros > 1000)
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

#define BOARD_X 192
#define BOARD_Y 192
int main(int argc, char *argv[])
{
	DataTracker<float> frameRateData(500);
	DataTracker<float> cellsModifiedData(250);
	SDL_Init(SDL_INIT_VIDEO);
	signal(SIGINT, intHandler);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	board = new Board(480, 270);
	printf("created board with dimension %d %d\n", board->dim_x, board->dim_y);

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
	board->AddSpeciesMember(firstOrganism);

	// SDL setup
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *window = SDL_CreateWindow("Mitch's Life Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return 0;
	}
	// SDL_RendererInfo info;
	// SDL_GetRendererInfo(renderer, &info);
	// SDL_Log("Current SDL_Renderer: %s", info.name);

	// Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	AddImPlotColorMap();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.IniFilename = NULL;

	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool testWinShown = false;

	int mouse_x = 0;
	int mouse_y = 0;

	boost::thread renderThread{TickMain};
	SDL_GetWindowSize(window, &winX, &winY);
	// Main loop
	bool done = false;
	static size_t frameCount = 0;
	static size_t lastSecondFrameCount = frameCount;
	std::vector<std::unique_ptr<OrganismView>> activeOrganismViews;
	while (!done)
	{
		frameRateData.Add(ImGui::GetIO().Framerate);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			static bool mouse1Held = false;
			static int totalDrag_x, totalDrag_y;
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
			{
				done = true;
				running = false;
			}
			else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
			{
				done = true;
				running = false;
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				if (!io.WantCaptureMouse)
				{
					SDL_GetMouseState(&mouse_x, &mouse_y);
					// scroll up
					if (event.wheel.y > 0)
					{
						forceRedraw = true;
						// x_off -= mouse_x / scaleFactor;
						// y_off -= mouse_y / scaleFactor;
						scaleFactor++;
					}
					// scroll down
					else if (event.wheel.y < 0)
					{
						if (scaleFactor > 1)
						{
							forceRedraw = true;
							// x_off += mouse_x / scaleFactor;
							// y_off += mouse_y / scaleFactor;
							scaleFactor--;
						}
					}
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (!io.WantCaptureMouse)
				{
					totalDrag_x = 0;
					totalDrag_y = 0;
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						mouse1Held = true;
						mouse_x = event.button.x;
						mouse_y = event.button.y;
					}
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				if (!io.WantCaptureMouse)
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						if (abs(totalDrag_x) < scaleFactor && abs(totalDrag_y) < scaleFactor)
						{
							int cell_x = (event.button.x - x_off) / scaleFactor;
							int cell_y = (event.button.y - y_off) / scaleFactor;
							if (!board->boundCheckPos(cell_x, cell_y))
							{
								Organism *clickedOrganism = board->cells[cell_y][cell_x]->myOrganism;
								if (clickedOrganism != nullptr)
								{
									board->GetMutex();
									activeOrganismViews.push_back(std::make_unique<OrganismView>(clickedOrganism, renderer));
									board->ReleaseMutex();
								}
							}
						}
						mouse1Held = false;
					}
				}
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (!io.WantCaptureMouse)
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_SPACE:
						autoplay = true;
						break;

					case SDLK_RETURN:
						if (!autoplay)
						{
							board->Tick();
						}
					}
				}
			}
			else if (event.type == SDL_WINDOWEVENT_SIZE_CHANGED || event.type == SDL_WINDOWEVENT)
			{
				SDL_GetWindowSize(window, &winX, &winY);
			}
			if (mouse1Held)
			{
				forceRedraw = true;
				int delta_x = event.button.x - mouse_x;
				int delta_y = event.button.y - mouse_y;
				totalDrag_x += delta_x;
				totalDrag_y += delta_y;

				x_off += delta_x;
				y_off += delta_y;

				/*
				if (x_off < 0)
				{
					x_off = 0;
				}
				else if (board->dim_x - (x_off / scaleFactor) < 0)
				{
					x_off = (board->dim_x - 1) * scaleFactor;
				}

				if ((y_off + (board->dim_y * scaleFactor)) < scaleFactor)
				{
					y_off = -1 * ((board->dim_y - 1) * scaleFactor);
				}
				else if (y_off > winY - scaleFactor)
				{
					y_off = winY - scaleFactor;
				}
				*/

				// printf("Delta on mouse drag: %d, %d\n", delta_x, delta_y);
				mouse_x = event.button.x;
				mouse_y = event.button.y;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Hello, world!");
			ImGui::Text("This is some useful text.");
			ImGui::Text("Viewport offset: %d,%d", x_off, y_off);
			ImGui::Text("Window dimensions: %d,%d", winX, winY);
			ImGui::Checkbox("Detailed Stats", &showDetailedStats);
			ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
			ImGui::PlotLines("Frame Times", frameRateData.rawData(), frameRateData.size());
			ImGui::PlotLines("Proportion of cells modified per render call", cellsModifiedData.rawData(), cellsModifiedData.size());
			ImGui::Checkbox("Autoplay (ENTER):", &autoplay);
			ImGui::SliderFloat("Target tick count per frame", &targetTickrate, 1.0, 100.0, "%.0f");
			ImGui::Checkbox("Max speed (reduces render rate)", &maxSpeed);
			static int lastSecondTickrate = 0;
			if (frameCount >= (lastSecondFrameCount + ceil(ImGui::GetIO().Framerate)))
			{
				lastSecondTickrate = ticksThisSecond;
				ticksThisSecond = 0;
				lastSecondFrameCount = frameCount;
			}
			ImGui::Text("%d ticks per second", lastSecondTickrate);
			ImGui::Text("%ld leftover microseconds", leftoverMicros);

			ImGui::Text("%lu organisms in %lu species", board->Organisms.size(), board->activeSpecies().size());
			stats.Display();

			if (showDetailedStats)
			{
				DetailedStats();
			}
			ImGui::End();
		}

		{
			if (testWinShown)
			{
				ImGui::Begin("TestWindow", &testWinShown); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from the test window!");
				if (ImGui::Button("Close Me"))
				{
					testWinShown = false;
				}
				ImGui::End();
			}
		}

		for (auto ovi = activeOrganismViews.begin(); ovi != activeOrganismViews.end(); ++ovi)
		{
			if (!(*ovi)->isOpen())
			{
				activeOrganismViews.erase(ovi);
				ovi--;
			}
			else
			{
				(*ovi)->OnFrame();
			}
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);

		// only force mutex acquisition if not running at max speed
		float cellsModified = RenderBoard(renderer, frameCount);
		if (cellsModified >= 0.0)
		{
			cellsModifiedData.Add(cellsModified);
		}
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

		SDL_RenderPresent(renderer);

		frameCount++;
	}

	renderThread.join();
	unsigned int finalSpeciesCount = board->GetNextSpecies();
	printf("\n");
	printf("%u species have ever lived\n", finalSpeciesCount);

	unsigned int largestSpecies = 0, largestSpeciesPopulation = 0;
	double totalMaxSpeciesSize = 0;
	int speciesSizeCounts[16] = {0};
	for (unsigned int i = 1; i < finalSpeciesCount; i++)
	{
		const SpeciesInfo &s = board->GetSpeciesInfo(i);
		if (s.peakCount > largestSpeciesPopulation)
		{
			largestSpecies = i;
			largestSpeciesPopulation = s.peakCount;
		}
		totalMaxSpeciesSize += s.peakCount;
		int index = 0;
		int count = s.peakCount;
		while (count > 1)
		{
			index++;
			count /= 2;
		}
		speciesSizeCounts[index]++;
	}
	totalMaxSpeciesSize /= finalSpeciesCount;
	printf("The largest species ever (%u) had %u concurrent living members at its peak\n", largestSpecies, largestSpeciesPopulation);
	printf("The average species peaked at %.2f members\n", totalMaxSpeciesSize);
	printf("Species size breakdown:\n");
	for (int i = 0; i < 16; i++)
	{
		int len = printf("\t%d..%d", (int)pow(2, i), (int)(pow(2, i + 1) - 1));
		while (len++ < 15)
		{
			printf(" ");
		}
		printf(":%2.4f%% or (%d)\n", 100.0 * (speciesSizeCounts[i] / (double)finalSpeciesCount), speciesSizeCounts[i]);
	}

	// Cleanup
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
