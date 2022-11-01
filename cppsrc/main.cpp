#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <vector>
#include <chrono>
#include <boost/thread.hpp>

#include <SDL2/SDL.h>
#include "imgui-1.88/imgui.h"
#include "imgui-1.88/backends/imgui_impl_sdl.h"
#include "imgui-1.88/backends/imgui_impl_sdlrenderer.h"

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

static volatile int running = 1;
Board *board = nullptr;
void intHandler(int dummy)
{
	running = 0;
}

int scaleFactor = 4;
int x_off = 0;
int y_off = 0;
int winX, winY;

class FrameratePID
{
	float target = 55.0;
	float previous_error = 0.0;
	float integral = 0.0;

	float Kp = 0.01;
	float Ki = 0.00001;
	float Kd = 10.0;

public:
	float Tick(float instantaneousFramerate)
	{
		if (instantaneousFramerate == 0.0)
		{
			instantaneousFramerate = 0.000001;
		}
		printf("Current instanteneous framerate: %f\n", instantaneousFramerate);
		float error = instantaneousFramerate - target;
		float dt = 1000.0 / instantaneousFramerate;
		printf("DT is %f, error is %f\n", dt, error);
		integral = integral + (error * dt);
		printf("Error * dt is %f\n", error * dt);
		float derivative = (error - previous_error) / dt;
		float delta = Kp * error + Ki * integral + Kd * derivative;
		printf("P:%f I:%f D:%f\n", error * Kp, integral * Ki, derivative * Kd);
		printf("PID Delta returned: %f\n\n", delta);
		previous_error = error;
		return delta;
	}
};
/*
previous_error = 0
integral = 0
Start:
error = setpoint – input
integral = integral + error*dt
derivative = (error – previous error)/dt
output = Kp*error + Ki*integral + Kd*derivative
previous_error = error
wait (dt)
Goto Start
*/

bool forceRedraw = false;
void RenderBoard(SDL_Renderer *r)
{
	// we'll use this texture as our own backbuffer
	static SDL_Texture *boardBuf = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB888,
													 SDL_TEXTUREACCESS_TARGET, winX, winY);

	// draw to board buffer instead of backbuffer
	SDL_SetRenderTarget(r, boardBuf);
	SDL_RenderSetScale(r, scaleFactor, scaleFactor);

	if (forceRedraw)
	{
		SDL_RenderClear(r);
	}

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
			if (forceRedraw || board->DeltaCells[y][x])
			{
				board->DeltaCells[y][x] = false;
				Cell *thisCell = board->cells[y][x];
				switch (thisCell->type)
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
				SDL_RenderDrawPoint(r, x + (x_off / scaleFactor), y + (y_off / scaleFactor));
			}
		}
	}
	SDL_RenderSetScale(r, 1.0, 1.0);
	forceRedraw = false;
	SDL_SetRenderTarget(r, NULL);
	SDL_RenderCopy(r, boardBuf, NULL, NULL);
}

bool autoplay = false;
int targetTickrate = 10;
int ticksThisSecond = 0;
void RenderMain()
{
	if (board == nullptr)
	{
		printf("RenderMain called with null board!\nBoard must be instantiated first!\n");
		return;
	}

	auto lastFrame = std::chrono::high_resolution_clock::now();
	
	long int leftoverMicros = 0;
	while (running)
	{
		if (autoplay)
		{
			board->Tick();
			ticksThisSecond++;
			auto frameEnd = std::chrono::high_resolution_clock::now();
			auto diff = frameEnd - lastFrame;
			size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
			if (targetTickrate < 1000)
			{
				leftoverMicros += static_cast<int>((1000000.0 / targetTickrate) - micros);
				if (leftoverMicros > 1000)
				{
					printf("Need additional delay\n");
					long int delayDuration = (leftoverMicros / 1000) * 1000;
					boost::this_thread::sleep(boost::posix_time::milliseconds(delayDuration / 1000));
					// SDL_Delay(delayDuration / 1000);
					// SDL_Delay(delayMillis);
					leftoverMicros %= 1000;
					auto delayEnd = std::chrono::high_resolution_clock::now();
					auto delayDiff = delayEnd - frameEnd;
					size_t actualDelayMicros = std::chrono::duration_cast<std::chrono::microseconds>(delayDiff).count();
					// printf("Additional delay leftover is %ld\n", static_cast<long int>(actualDelayMicros) - delayDuration);
					leftoverMicros -= delayDuration - static_cast<int>(actualDelayMicros);
					lastFrame = delayEnd;
				}
				else
				{
					lastFrame = frameEnd;
				}
			}
			else
			{
				lastFrame = frameEnd;
			}
			// printf("leftover: %ld\n", leftoverMicros);
		}
		else
		{
			// SDL_Delay(100);
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
}

#define BOARD_X 192
#define BOARD_Y 192
#define FRAMERATE_TRACKING_INTERVAL 1000

int main(int argc, char *argv[])
{
	int frameP = 0;
	float frameRates[FRAMERATE_TRACKING_INTERVAL] = {60.0};
	SDL_Init(SDL_INIT_VIDEO);
	signal(SIGINT, intHandler);
	FrameratePID frameratePid;
	// Setup SDL
	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	board = new Board(512, 512);
	// boardWindow->SetBoard(board);
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
	// Setup window
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

	// Setup SDL_Renderer instance
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return 0;
	}
	// SDL_RendererInfo info;
	// SDL_GetRendererInfo(renderer, &info);
	// SDL_Log("Current SDL_Renderer: %s", info.name);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// io.Fonts->AddFontDefault();
	// io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	// ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	// IM_ASSERT(font != NULL);

	// Our state
	// bool show_demo_window = true;
	// bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool testWinShown = true;

	int mouse_x = 0;
	int mouse_y = 0;

	boost::thread renderThread{RenderMain};
	SDL_GetWindowSize(window, &winX, &winY);
	// Main loop
	bool done = false;
	static size_t frameCount = 0;
	static size_t lastSecondFrameCount = frameCount;
	while (!done)
	{
		frameRates[frameP] = ImGui::GetIO().Framerate;
		++frameP %= FRAMERATE_TRACKING_INTERVAL;
		// if (autoplay && maxSpeed)
		// {
			// targetTPSDelta = frameratePid.Tick(ImGui::GetIO().Framerate);
		// }

		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			static bool mouse1Held = false;
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
				// scroll up
				if (event.wheel.y > 0)
				{
					forceRedraw = true;
					scaleFactor++;
				}
				// scroll down
				else if (event.wheel.y < 0)
				{
					if (scaleFactor > 1)
					{
						forceRedraw = true;
						scaleFactor--;
					}
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (!io.WantCaptureMouse)
				{
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

			if (mouse1Held)
			{
				forceRedraw = true;
				int delta_x = mouse_x - event.button.x;
				int delta_y = mouse_y - event.button.y;
				x_off -= delta_x;
				y_off -= delta_y;
				if ((x_off + (board->dim_x * scaleFactor)) < scaleFactor)
				{
					x_off = -1 * ((board->dim_x - 1) * scaleFactor);
				}
				else if (x_off > winX - scaleFactor)
				{
					x_off = winX - scaleFactor;
				}
				if ((y_off + (board->dim_y * scaleFactor)) < scaleFactor)
				{
					y_off = -1 * ((board->dim_y - 1) * scaleFactor);
				}
				else if (y_off > winY - scaleFactor)
				{
					y_off = winY - scaleFactor;
				}
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
			ImGui::Begin("Hello, world!");				   // Create a window called "Hello, world!" and append into it.
			ImGui::Text("This is some useful text.");	   // Display some text (you can use a format strings too)
			ImGui::Checkbox("Test Window", &testWinShown); // Edit bools storing our window open/close state
														   // ImGui::Checkbox("Another Window", &show_another_window);
														   // renderer and other code before this point
			ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
			static float realFrameRates[FRAMERATE_TRACKING_INTERVAL];
			for (int i = 0; i < FRAMERATE_TRACKING_INTERVAL; i++)
			{
				realFrameRates[i] = frameRates[(frameP + i) % FRAMERATE_TRACKING_INTERVAL];
			}
			ImGui::PlotLines("Frame Times", realFrameRates, FRAMERATE_TRACKING_INTERVAL);
			ImGui::Checkbox("Autoplay (ENTER):", &autoplay);
			ImGui::SliderInt("Target tick count per frame", &targetTickrate, 1, 1000);
			static int lastSecondTickrate = 0;
			if(frameCount >= (lastSecondFrameCount + ceil(ImGui::GetIO().Framerate)))
			{
				lastSecondTickrate = ticksThisSecond;
				ticksThisSecond = 0;
				lastSecondFrameCount = frameCount;
			}
			ImGui::Text("%d ticks per second", lastSecondTickrate);
			ImGui::End();
		}

		/*
		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}*/
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

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		RenderBoard(renderer);
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
		if (board->peakSpeciesCounts[i] > largestSpeciesPopulation)
		{
			largestSpecies = i;
			largestSpeciesPopulation = board->peakSpeciesCounts[i];
		}
		totalMaxSpeciesSize += board->peakSpeciesCounts[i];
		int index = 0;
		int count = board->peakSpeciesCounts[i];
		while (count > 1)
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
	for (int i = 0; i < 16; i++)
	{
		int len = printf("\t%d..%d", (int)pow(2, i), (int)(pow(2, i + 1) - 1));
		while (len++ < 15)
		{
			printf(" ");
		}
		printf(":%2.2f%% (%d)\n", 100.0 * (speciesSizeCounts[i] / (double)finalSpeciesCount), speciesSizeCounts[i]);
	}

	// Cleanup
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
