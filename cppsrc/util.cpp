#include "util.h"
#include <boost/thread.hpp>
#include <chrono>
#include <SDL2/SDL.h>
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

float WorldSettings::GetBase(WorldSettings::SettingNames s)
{
	return SettingsBase[s];
}

float WorldSettings::Get(WorldSettings::SettingNames s)
{
	switch (s)
	{
	case default_mutability:
	case lifespan_multiplier:
	case reproduction_energy_proportion:
		return this->values[s];

	case reproduction_cooldown_multiplier:
		return this->values[s] * this->values[lifespan_multiplier];

	case max_health_multiplier:
	case energy_density_multiplier:
		return this->values[s];

	case move_cost_multiplier:
	case food_multiplier:
		return this->values[s] * this->values[energy_density_multiplier];

	case leaf_food_energy:
	case flower_food_energy:
	case fruit_food_energy:
	case plantmass_food_energy:
	case biomass_food_energy:
		return this->values[s] * this->values[food_multiplier];

	// leaf
	case photosynthesis_energy_multiplier:
		return this->values[s];

	case leaf_flowering_cost:
		return this->values[s] * this->values[energy_density_multiplier];

	case leaf_flowering_cooldown:
	case leaf_flowering_ability_percent:
		return this->values[s];

	// flower
	case flower_bloom_cost:
		return this->values[s] * this->values[energy_density_multiplier];

	case flower_bloom_cooldown:
	case flower_wilt_chance:
	case flower_expand_percent:
	case spoil_time_base:

	case plantmass_spoil_time:
	case biomass_spoil_time:
	case fruit_spoil_time:
		return this->values[s] * this->values[spoil_time_base];

	case fruit_grow_percent:
	case bark_grow_cooldown:
	case bark_plant_vs_thorn:
		return this->values[s];

	case bark_grow_cost:
		return this->values[s] * this->values[energy_density_multiplier];

	case bark_max_integrity:
		return this->values[s];

	// killer
	case killer_tick_cost:
	case killer_damage_cost:
		return this->values[s] * this->values[energy_density_multiplier];

	// armor
	case armor_health_bonus:
	case eye_max_seeing_distance:
		return this->values[s];

	case null:
	default:
		printf("null/unexpected setting passed to WorldSettings::Get()\n");
		exit(1);
	}
}

void WorldSettings::Set(WorldSettings::SettingNames s, float value)
{
	this->values[s] = value;
}

const ImVec4 cellColors[cell_null] =
	{
		{0, 0, 0, 0},		  // empty
		{10, 40, 10, 255},	  // plantmass
		{150, 60, 60, 255},	  // biomass
		{30, 120, 30, 255},	  // leaf
		{75, 25, 25, 255},	  // bark
		{50, 250, 150, 255},  // flower
		{200, 200, 0, 255},	  // fruit
		{255, 150, 0, 255},	  // herb
		{255, 100, 150, 255}, // carn
		{50, 120, 255, 255},  // mover
		{255, 0, 0, 255},	  // killer
		{175, 0, 255, 255},	  // armor
		{150, 150, 150, 255}, // touch
		{255, 255, 255, 255}  // eye

};

const ImVec4 classColors[class_null] =
	{
		{30.0 / 255, 120.0 / 255, 30.0 / 255, 255.0 / 255},	  // plant
		{255.0 / 255, 150.0 / 255, 0.0 / 255, 255.0 / 255},	  // herb
		{255.0 / 255, 100.0 / 255, 150.0 / 255, 255.0 / 255}, // carn
		{255.0 / 255, 0.0 / 255, 255.0 / 255, 255.0 / 255}	  // omni
};

const char *cellNames[cell_null] =
	{"empty",
	 "plantmass",
	 "biomass",
	 "leaf",
	 "bark",
	 "flower",
	 "fruit",
	 "herbivore",
	 "carnivore",
	 "mover",
	 "killer",
	 "armor",
	 "touch",
	 "eye"};

const char *classNames[class_null] =
	{"plant",
	 "herbivore",
	 "carnivore",
	 "omnivore"};

const uint32_t cellXs[cell_null] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const double cellXs_double[cell_null] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0};

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
