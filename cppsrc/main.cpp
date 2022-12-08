#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>
#include <chrono>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/mutex.hpp>

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
#include "datatracker.h"
#include "util.h"

#define MIN_EXTRA_MICROS 100
static volatile int running = 1;
Board *board = nullptr;

boost::mutex renderMutex;
bool doneRendering = false;
boost::condition_variable renderCondition;
// boost::interprocess::interprocess_semaphore renderSemaphore(2);
void intHandler(int dummy)
{
	running = 0;
}

float scaleFactor = (1.0 / 3.0);
float x_off = 0.0;
float y_off = 0.0;
int winSizeX, winSizeY;
float targetTickrate = 10;
// double PIDTickrate = 1.0;
long int leftoverMicros = 0;
bool autoplay = false;
// bool justEnabledAutoplay = false;
bool maxSpeed = false;
int ticksThisSecond = 0;
/*
class TickratePID
{
	double previous_error = 0.0;
	double integral = 0.0;
	// float Kp = 0.000024;
	// float Ki = 0.0002;
	// float Kd = 0.00000001;
	// float Kp = 0.0001;
	double Kp = 0.00001;
	double Ki = 0.00000001;
	// double Ki = 0.0001;
	double Kd = -0.0000001;

public:
	void Tick(double instanteneousMeasurement, size_t dtMicroseconds, bool ignoreIntegral)
	{

		// printf("Current instanteneous framerate: %f\n", instanteneousMeasurement);
		double error = instanteneousMeasurement - (1000000.0 / targetTickrate);
		double dt = dtMicroseconds / 1000.0;
		if (dt == 0.0)
		{
			dt = 0.00001;
		}

		if (!ignoreIntegral)
		{
			integral += (error * dt);
			integral /= 2.0;
		}
		// integral += instanteneousMeasurement;
		// integral /= 2;
		double derivative = (error - previous_error) / dt;
		printf("input: %f \tE:% 7f I:% 7f D:% 7f - DT:% 8f\n", instanteneousMeasurement, (Kp * error), (Ki * integral), (Kd * derivative), dt);
		double delta = (Kp * error) + (Ki * integral) + (Kd * derivative);
		if (!ignoreIntegral)
		{
			previous_error = error;
		}
		else
		{
			previous_error = 0;
		}
		PIDTickrate += delta;
		printf("PID Delta: %f\n", delta);
		if (PIDTickrate < 1.0 || isnan(PIDTickrate))
		{
			PIDTickrate = 1.0;
		}
	}
};

TickratePID pidController;
*/

inline void DrawCell(SDL_Renderer *r, Cell *c, int x, int y)
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
		count_neurons,
		count_synapses,
		count_raw,
		count_null
	};
	double organismStats[class_null][count_null] = {{0.0}};
	size_t totalClassEnergies[class_null] = {0};

	int classCounts[class_null] = {0};
	double organismCellCounts[class_null][cell_null] = {{0.0}};
	double touchSensorHaverCounts[class_null] = {0.0};
	// static double touchSensorIntervals[class_null] = {0.0};
	double cellSentiments[class_null][cell_null] = {{0.0}};

	DataTracker<int> tickData = DataTracker<int>(2500);
	DataTracker<double> tickDataDouble = DataTracker<double>(2500);

	DataTracker<int> *classCountData[class_null + 1];
	DataTracker<double> *classEnergyProportionData[class_null];

public:
	Stats()
	{
		for (int i = 0; i < class_null + 1; i++)
		{
			this->classCountData[i] = new DataTracker<int>(2500);
		}

		for (int i = 0; i < class_null; i++)
		{
			this->classEnergyProportionData[i] = new DataTracker<double>(2500);
		}
	}

	~Stats()
	{
		for (int i = 0; i < class_null + 1; i++)
		{
			delete this->classCountData[i];
		}

		for (int i = 0; i < class_null; i++)
		{
			delete this->classEnergyProportionData[i];
		}
	}

	void Display()
	{
		if (ImGui::BeginTable("OrganismStats", class_null + 1))
		{
			const char *rowNames[count_null + 1] = {"Class:", "Count", "Cells", "Energy%", "Max Energy", "Age%", "Lifespan", "Mutability", "Neurons", "Synapses"};
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

			// neurons
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.2f", organismStats[i][count_neurons]);
			}
			row++;

			// synapses
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", rowNames[row]);
			for (int i = 0; i < class_null; i++)
			{
				ImGui::TableSetColumnIndex(i + 1);
				ImGui::Text("%.2f", organismStats[i][count_synapses]);
			}

			ImGui::EndTable();
		}

		static int whichGraph = 0;
		ImGui::RadioButton("Organism Counts", &whichGraph, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Energy info by class", &whichGraph, 1);
		// ImGui::SameLine();
		// ImGui::RadioButton("radio c", &whichGraph, 2);

		switch (whichGraph)
		{
		case 0:
		{
			// ImPlot::SetNextAxesLimits(0, organismCountData.size(), 0, static_cast<double>(maxOrganisms));
			ImPlot::SetNextAxesToFit();
			// ImPlot::BeginPlot("Bar Graph##Line", "Day", NULL, ImVec2(-1, 0), ImPlotFlags_NoLegend | ImPlotFlags_NoBoxSelect | ImPlotFlags_AntiAliased, ImPlotAxisFlags_Time, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit)
			if (ImPlot::BeginPlot("Organism Counts by Classification", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit))
			{
				ImPlot::PushColormap(ClassColormap);
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
				ImPlot::PlotLine("Plants", tickData.rawData(), classCountData[class_plant]->rawData(), static_cast<int>(classCountData[class_plant]->size()));
				ImPlot::PlotLine("Herbivores", tickData.rawData(), classCountData[class_herbivore]->rawData(), static_cast<int>(classCountData[class_herbivore]->size()));
				ImPlot::PlotLine("Carnivores", tickData.rawData(), classCountData[class_carnivore]->rawData(), static_cast<int>(classCountData[class_carnivore]->size()));
				ImPlot::PlotLine("Omnivores", tickData.rawData(), classCountData[class_omnivore]->rawData(), static_cast<int>(classCountData[class_omnivore]->size()));
				ImPlot::PopColormap();
				ImPlot::PlotLine("Total Organisms", tickData.rawData(), classCountData[class_null]->rawData(), static_cast<int>(classCountData[class_null]->size()));
				ImPlot::EndPlot();
			}
		}
		break;

		case 1:
		{
			// ImPlot::SetNextAxesToFit();
			ImPlot::SetNextAxesToFit();
			if (ImPlot::BeginPlot("Proportion of total energy by Classification", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect))
			{
				ImPlot::PushColormap(ClassColormap);
				static double rawProportionData[2500 * class_null];
				// const double *rawProportionData[class_null] = {nullptr};
				for (int i = 0; i < class_null; i++)
				{
					size_t size = classEnergyProportionData[i]->size();
					for (size_t j = 0; j < size; j++)
					{
						rawProportionData[(i * size) + j] = classEnergyProportionData[i]->rawData()[j];
					}
					// rawProportionData[i] = classEnergyProportionData[i]->rawData();
				}
				ImPlot::PlotBarGroups(classNames, rawProportionData, class_null, classEnergyProportionData[0]->size(), 0, 0, ImPlotBarGroupsFlags_Stacked);

				// ImPlot::PlotLine(classNames[i], tickDataDouble.rawData(), classEnergyProportionData[i]->rawData(), static_cast<int>(classEnergyProportionData[i]->size()));
				ImPlot::EndPlot();
			}
			/*
			size_t totalEnergy = 0;

			if (ImPlot::BeginPlot("Proportion of total energy by Classification", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect))
			{
				ImPlot::PushColormap(ClassColormap);
				double energies[class_null];
				for (int i = 0; i < class_null; i++)
				{
					totalEnergy += totalClassEnergies[i];
				}
				for (int i = 0; i < class_null; i++)
				{
					energies[i] = totalClassEnergies[i] / static_cast<double>(totalEnergy);
				}

				ImPlot::PlotBarGroups(classNames, energies, class_null, 1, 1.0, 0, ImPlotBarGroupsFlags_Stacked);
				ImPlot::EndPlot();
			}
			*/
		}
		break;
		default:
			printf("Impossible value for radio button!\n");
			exit(1);
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
			totalClassEnergies[i] = 0;
		}
		for (Organism *o : board->Organisms)
		{
			enum OrganismClassifications thisClass = board->GetSpeciesInfo(o->species).classification;
			classCounts[thisClass]++;

			organismStats[thisClass][count_cells] += o->nCells();
			organismStats[thisClass][count_energy] += o->GetEnergy();

			totalClassEnergies[thisClass] += o->GetEnergy();
			organismStats[thisClass][count_maxenergy] += o->GetMaxEnergy();

			organismStats[thisClass][count_age] += o->age;
			organismStats[thisClass][count_lifespan] += o->lifespan;
			organismStats[thisClass][count_mutability] += o->mutability;
			organismStats[thisClass][count_neurons] += o->brain->NeuronCount();
			organismStats[thisClass][count_synapses] += o->brain->SynapseCount();
			organismStats[thisClass][count_raw]++;
			for (int i = 0; i < cell_null; i++)
			{
				organismCellCounts[thisClass][i] += o->cellCounts[i];
			}
			if (o->cellCounts[cell_touch] > 0)
			{
				touchSensorHaverCounts[thisClass]++;
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
		tickDataDouble.Add(static_cast<double>(board->tickCount));
		for (int i = 0; i < class_null; i++)
		{
			classCountData[i]->Add(classCounts[i]);
		}
		classCountData[class_null]->Add(board->Organisms.size());

		size_t totalEnergy = 0;
		for (int i = 0; i < class_null; i++)
		{
			totalEnergy += totalClassEnergies[i];
		}
		for (int i = 0; i < class_null; i++)
		{
			classEnergyProportionData[i]->Add(totalClassEnergies[i] / static_cast<double>(totalEnergy));
		}
	}
};
Stats stats;

bool forceRedraw = false;
float RenderBoard(SDL_Renderer *r, size_t frameNum, bool forceMutex)
{
	// we'll use this texture as a seperate backbuffer
	static SDL_Texture *boardBuf = nullptr;

	if (boardBuf == nullptr)
	{
		boardBuf = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB888,
									 SDL_TEXTUREACCESS_TARGET, board->dim_x * 3, board->dim_y * 3);
	}

	float trueScaleFactor = (scaleFactor * 3);

	int boardBufDim_x = board->dim_x * 3;
	int boardBufDim_y = board->dim_y * 3;

	float winSizeXF = winSizeX;
	float winSizeYF = winSizeY;

	if ((x_off + winSizeXF) / trueScaleFactor >= (boardBufDim_x))
	{
		x_off = (boardBufDim_x * trueScaleFactor) - winSizeXF;
	}

	if (x_off < 0)
	{
		x_off = 0;
	}

	if ((y_off + winSizeYF) / trueScaleFactor >= (boardBufDim_y))
	{
		y_off = (boardBufDim_y * trueScaleFactor) - winSizeYF;
	}

	if (y_off < 0)
	{
		y_off = 0;
	}

	float x_src, y_src, w_src, h_src;
	float x_dst, y_dst, w_dst, h_dst;

	x_src = x_off / trueScaleFactor;
	y_src = y_off / trueScaleFactor;

	x_dst = 0.0;
	y_dst = 0.0;
	w_dst = winSizeX;
	h_dst = winSizeY;

	if (x_src < 0.0)
	{
		// w_src += (x_src / scaleFactor);
		x_src = 0.0;
	}

	if (y_src < 0.0)
	{
		// h_src += (y_src / scaleFactor);
		y_src = 0.0;
	}

	w_src = (winSizeXF / trueScaleFactor);
	if (w_src > boardBufDim_x)
	{
		w_src = boardBufDim_x;
	}

	h_src = (winSizeYF / trueScaleFactor);
	if (h_src > boardBufDim_y)
	{
		h_src = boardBufDim_y;
	}

	w_dst = w_src * (trueScaleFactor);
	h_dst = h_src * (trueScaleFactor);

	SDL_Rect srcRect = {static_cast<int>(x_src), static_cast<int>(y_src), static_cast<int>(w_src), static_cast<int>(h_src)};
	SDL_Rect dstRect = {static_cast<int>(x_dst), static_cast<int>(y_dst), static_cast<int>(w_dst), static_cast<int>(h_dst)};

	// printf("scalefactor = %f\n", trueScaleFactor);
	// printf("Src rect: %.1f:%.1f %.1fx%.1f\n", x_src, y_src, w_src, h_src);
	// printf("Dst rect: %.1f:%.1f %.1fx%.1f\n\n", x_dst, y_dst, w_dst, h_dst);

	if ((board->DeltaCells.size() == 0 && !forceRedraw) ||
		(leftoverMicros < 0))
	{
		SDL_RenderCopy(r, boardBuf, &srcRect, &dstRect);
		return -1.0;
	}

	/*
	 * don't bother waiting around for the mutex if trying to run at max speed
	 * waiting for the mutex will decrease the frame rate, we will use the condvar to guarantee an update every so often
	 */
	if (forceMutex)
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
	SDL_RenderSetScale(r, 3.0, 3.0);
	size_t cellsModified = 0;

	if (forceRedraw)
	{
		// SDL_RenderClear(r);
		for (int y = 0; y < board->dim_y; y++)
		{
			if (y + (y_off / scaleFactor) < 0 || ((y - board->dim_y) * scaleFactor) + y_off > winSizeY)
			{
				continue;
			}
			for (int x = 0; x < board->dim_x; x++)
			{
				if (x + (x_off / scaleFactor) < 0 || ((x - board->dim_x) * scaleFactor) + x_off > winSizeX)
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
	SDL_RenderSetScale(r, 1.0, 1.0);

	board->ReleaseMutex();
	forceRedraw = false;
	SDL_SetRenderTarget(r, NULL);
	SDL_RenderCopy(r, boardBuf, &srcRect, &dstRect);
	return static_cast<float>(cellsModified) / (board->dim_x * board->dim_y);
}

void TickMain()
{
	auto lastFrame = std::chrono::high_resolution_clock::now();
	boost::mutex::scoped_lock lock(renderMutex);
	while (running)
	{
		if (autoplay)
		{
			while (!doneRendering)
				renderCondition.wait(lock);

			if (board->Tick())
			{
				continue;
			}
			ticksThisSecond++;
			if (board->tickCount % 10 == 0)
			{
				stats.Update();
			}
			auto tickEnd = std::chrono::high_resolution_clock::now();
			auto diff = tickEnd - lastFrame;
			size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
			if (!maxSpeed)
			{
				int leftoverThisStep = static_cast<int>((1000000.0 / targetTickrate) - micros);
				leftoverMicros += leftoverThisStep;

				// pidController.Tick(leftoverMicros, micros, justEnabledAutoplay);

				if (leftoverMicros > 1000)
				{
					int delayDuration = leftoverMicros / 1000;
					boost::this_thread::sleep(boost::posix_time::milliseconds(delayDuration));
					leftoverMicros -= delayDuration * 1000;
					auto delayEnd = std::chrono::high_resolution_clock::now();
					auto delayDiff = delayEnd - tickEnd;
					size_t actualDelayMicros = std::chrono::duration_cast<std::chrono::microseconds>(delayDiff).count();
					leftoverMicros -= (delayDuration * 1000) - static_cast<int>(actualDelayMicros);
					lastFrame = delayEnd;
				}
				else
				{
					if (leftoverMicros < -10000)
					{
						leftoverMicros = -10000;
					}
					lastFrame = tickEnd;
				}
			}
			else
			{
				lastFrame = tickEnd;
			}
		}
		else
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
}

void testOrganism(Organism *o)
{
	static int counter = 0;
	// while (true)
	// {
	// RenderBoard(renderer, 0, true);
	// firstOrganism->Rotate(true);
	// boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	// }
	if (++counter % 2 == 0)
	{
		o->Rotate(true);
	}
	else
	{
		o->Move(3);
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

	// SDL setup
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *window = SDL_CreateWindow("Mitch's Life Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return 0;
	}

	board = new Board(384, 216);
	printf("created board with dimension %d %d\n", board->dim_x, board->dim_y);

	Organism *firstOrganism = board->createOrganism(board->dim_x / 2, board->dim_y / 2);
	firstOrganism->direction = 3;
	firstOrganism->AddCell(0, 0, new Cell_Leaf(0));
	firstOrganism->RecalculateStats();
	firstOrganism->lifespan = LIFESPAN_MULTIPLIER * firstOrganism->GetMaxEnergy();
	firstOrganism->mutability = 10;
	firstOrganism->Reproduce();
	firstOrganism->AddEnergy(firstOrganism->GetMaxEnergy());
	firstOrganism->Heal(100);
	// firstOrganism->reproductionCooldown = 10;
	firstOrganism->species = board->GetNextSpecies();
	board->AddSpeciesMember(firstOrganism);

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
	SDL_GetWindowSize(window, &winSizeX, &winSizeY);
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
						scaleFactor += (1.0 / 27.0);
					}
					// scroll down
					else if (event.wheel.y < 0)
					{
							forceRedraw = true;
							// x_off += mouse_x / scaleFactor;
							// y_off += mouse_y / scaleFactor;
							scaleFactor -= (1.0 / 27.0);
						if(scaleFactor < (1.0 / 27.0))
						{
							scaleFactor = (1.0 / 27.0);
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
							float cell_x = ((static_cast<float>(event.button.x) / 3) + (x_off / 3)) / (scaleFactor * 3.0);
							float cell_y = ((static_cast<float>(event.button.y) / 3) + (y_off / 3)) / (scaleFactor * 3.0);
							int cell_x_int = cell_x;
							int cell_y_int = cell_y;

							if (!board->boundCheckPos(cell_x_int, cell_y_int))
							{
								Organism *clickedOrganism = board->cells[cell_y_int][cell_x_int]->myOrganism;
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
				SDL_GetWindowSize(window, &winSizeX, &winSizeY);
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
			ImGui::Text("Viewport offset: %.0f,%.0f", x_off, y_off);
			ImGui::Text("Window dimensions: %d,%d", winSizeX, winSizeY);
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

		float cellsModified;
		// only make the tick thread wait if not running at max speed
		if (!maxSpeed || (frameCount % 30 == 0))
		{
			doneRendering = false;
			cellsModified = RenderBoard(renderer, frameCount, true);
			doneRendering = true;
			renderCondition.notify_all();
		}
		else
		{
			cellsModified = RenderBoard(renderer, frameCount, false);
		}

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
