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
#include "worldsettings.h"
#include "organismview.h"
#include "detailedstats.h"
#include "rng.h"
#include "datatracker.h"
#include "util.h"

#define MIN_EXTRA_MICROS 100
static volatile int running = 1;
Board *board = nullptr;
WorldSettings Settings;
std::map<OrganismIdentifier, std::unique_ptr<OrganismView>> activeOrganismViews;

boost::mutex renderMutex;
bool doneRendering = false;
boost::condition_variable renderCondition;
void intHandler(int dummy)
{
	running = 0;
}

float scaleFactor = (1.0 / 3.0);
float x_off = 0.0;
float y_off = 0.0;
int winSizeX, winSizeY;
float targetTickrate = 10;
long int leftoverMicros = 0;
bool autoplay = false;
bool maxSpeed = false;
int ticksThisSecond = 0;

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

int main(int argc, char *argv[])
{
	int board_x = 0;
	int board_y = 0;
	int thisOpt;
	while ((thisOpt = getopt(argc, argv, "w:h:")) != -1)
	{
		printf("%c\n", thisOpt);
		switch (thisOpt)
		{
		case 'w':
			board_x = atoi(optarg);
			break;

		case 'h':
			board_y = atoi(optarg);
			break;

		default:
			printf("use -w and -h to set board dimensions");
			exit(1);
		}
	}
	if (board_x == 0 || board_y == 0)
	{
		printf("Please specify board width and height with -w and -h flags!\n");
		exit(1);
	}
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

	board = new Board(board_x, board_y);
	printf("created board with dimension %d %d\n", board->dim_x, board->dim_y);

	Organism *firstOrganism = board->CreateOrganism(board->dim_x / 2, board->dim_y / 2);
	firstOrganism->direction = 3;
	firstOrganism->AddCell(0, 0, new Cell_Leaf(0));
	// firstOrganism->AddCell(0, 1, new Cell_Leaf(0));
	// firstOrganism->AddCell(0, -1, new Cell_Leaf(0));
	/*
	firstOrganism->AddCell(-1, 0, new Cell_Leaf(0));
	firstOrganism->AddCell(1, 0, new Cell_Leaf(0));
	firstOrganism->AddCell(0, 1, new Cell_Leaf(0));
	*/
	firstOrganism->RecalculateStats();
	firstOrganism->lifespan = 10000;

	firstOrganism->mutability = 10;
	firstOrganism->age = 0;
	// firstOrganism->Reproduce();
	firstOrganism->AddEnergy(static_cast<float>(firstOrganism->MaxEnergy()));
	firstOrganism->Heal(100);
	// firstOrganism->identifier_ = OrganismIdentifier(board->GetNextSpecies());
	// firstOrganism->reproductionCooldown = 10;
	// organism will be species 0 instance 0 by default
	board->AddSpeciesMember(firstOrganism);
	board->GetNextSpecies();

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
						if (scaleFactor < (1.0 / 27.0))
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
							board->GetMutex();

							float cell_x = ((static_cast<float>(event.button.x) / 3) + (x_off / 3)) / (scaleFactor * 3.0);
							float cell_y = ((static_cast<float>(event.button.y) / 3) + (y_off / 3)) / (scaleFactor * 3.0);
							int cell_x_int = cell_x;
							int cell_y_int = cell_y;
							if (!board->boundCheckPos(cell_x_int, cell_y_int))
							{
								Organism *clickedOrganism = board->cells[cell_y_int][cell_x_int]->myOrganism;
								if (clickedOrganism != nullptr && (activeOrganismViews.count(clickedOrganism->Identifier()) == 0))
								{
									activeOrganismViews[clickedOrganism->Identifier()] = (std::make_unique<OrganismView>(clickedOrganism, renderer));
								}
							}
							board->ReleaseMutex();
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
			ImGui::Text("Viewport offset: %.0f,%.0f", x_off, y_off);
			ImGui::Text("Window dimensions: %d,%d", winSizeX, winSizeY);
			ImGui::Checkbox("Detailed Organism Makeup Stats", &showDetailedStats);
			ImGui::SameLine();
			ImGui::Checkbox("World Settings", &showWorldSettingsView);
			ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);

			/*
			ImPlot::SetNextAxesToFit();
			if (ImPlot::BeginPlot("Renderer Stats"))
			{
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
				// ImPlot::PlotLine("Frame Rate", frameRateData.rawData(), frameRateData.size());
				// ImPlot::PlotLine("Proportion of cells modified per render call", cellsModifiedData.rawData(), cellsModifiedData.size());
				ImPlot::EndPlot();
			}
			// ImGui::PlotLines("Frame Times", frameRateData.rawData(), frameRateData.size());
			// ImGui::PlotLines("Proportion of cells modified per render call", cellsModifiedData.rawData(), cellsModifiedData.size());
			*/
			ImGui::PlotLines("Frame Rate", frameRateData.rawData(), frameRateData.size());
			ImGui::Checkbox("Autoplay (SPACE):", &autoplay);

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
			board->stats.Display();

			if (showDetailedStats)
			{
				DetailedStats();
			}

			if (showWorldSettingsView)
			{
				WorldSettingsView();
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

		if (activeOrganismViews.size() > 0)
		{
			doneRendering = false;
			board->GetMutex();
			for (auto organismViewi = activeOrganismViews.begin(); organismViewi != activeOrganismViews.end();)
			{
				auto next = organismViewi;
				++next;
				if (!(organismViewi->second->isOpen()))
				{
					activeOrganismViews.erase(organismViewi->first);
				}
				else
				{
					organismViewi->second->OnFrame(renderer);
				}
				organismViewi = next;
			}
			board->ReleaseMutex();
			doneRendering = true;
			renderCondition.notify_all();
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);

		float cellsModified;
		// only make the tick thread wait every time if not running at max speed, otherwise just once a second is good
		if (!maxSpeed || (frameCount % 60 == 0))
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
