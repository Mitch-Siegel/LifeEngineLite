class SDL_Renderer;
class Cell;

#include "implot.h"
#include "config.h"
#include <cstdint>

#ifndef CellColormap
extern const ImVec4 cellColors[cell_null];
extern ImPlotColormap CellColormap;
extern ImPlotColormap ClassColormap;
extern const char *cellNames[cell_null];
extern const char *classNames[class_null];
extern const uint32_t cellXs[cell_null];
extern const double cellXs_double[cell_null];
#endif

class WorldSettings
{
public:
	enum SettingNames
	{
		// base settings
		default_mutability,
		lifespan_multiplier,
		// energy proportions (of max) required for certain actions
		reproduction_energy_proportion,
		reproduction_cooldown_multiplier, // * lifespan multiplier
		max_health_multiplier,
		// base energy stats
		energy_density_multiplier,
		move_cost_multiplier,			  // * energy density multiplier
		food_multiplier,				  // * energy density multiplier
		leaf_food_energy,				  // * food multiplier
		flower_food_energy,				  // * food multiplier
		fruit_food_energy,				  // * food multiplier
		plantmass_food_energy,			  // * food multiplier
		biomass_food_energy,			  // * food multiplier
										  // leaf
		photosynthesis_energy_multiplier, // each leaf generates this much energy per tick
		leaf_flowering_cost,			  // * energy density multiplier
		leaf_flowering_cooldown,		  // number of ticks between leaf flowering
		leaf_flowering_ability_percent,	  // % chance a new leaf cell will be able to flower
										  // flower
		flower_bloom_cost,				  // * energy density multiplier
		flower_bloom_cooldown,			  // * lifespan multiplier
		flower_wilt_chance,				  // each time the flower blooms, it has this % chance to wilt
		flower_expand_percent,			  // % for wilting flower to become leaf
										  // spoil times
		spoil_time_base,				  // in ticks
		plantmass_spoil_time,			  // * spoiltime base
		biomass_spoil_time,				  // * spoiltime base
		fruit_spoil_time,				  // * spoiltime base
		fruit_grow_percent,				  // % for fruit to spontaneously grow a new organism

		// bark
		bark_grow_cooldown,	 // * lifespan multiplier
		bark_plant_vs_thorn, // % to grow a plant cell (leaf/bark) vs a killer
		bark_grow_cost,		 // * energy density multiplier
		bark_max_integrity,	 // how many times bark can be "eaten" before it is broken through

		// killer
		killer_tick_cost,	// * energy density multiplier
		killer_damage_cost, // * energy density multiplier

		// armor
		armor_health_bonus,

		// eye
		eye_max_seeing_distance,

		null

	};

private:
	constexpr static float SettingsBase[SettingNames::null] =
		{
			// base settings
			15,	  // default_mutability,
			30,	  // lifespan_multiplier,
			0.7,  // reproduction_energy_proportion,
			15,	  // reproduction_cooldown_multiplier,
			1,	  // max_health_multiplier,
			4.0,  // energy_density_multiplier,
			1.2,  // move_cost_multiplier,
			8.0,  // food_multiplier,
			1,	  // leaf_food_energy,
			3,	  // flower_food_energy,
			8,	  // fruit_food_energy,
			2,	  // plantmass_food_energy,
			32,	  // biomass_food_energy,
			.084, // photosynthesis_energy_multiplier,
			1,	  // leaf_flowering_cost,
			3,	  // leaf_flowering_cooldown,
			45,	  // leaf_flowering_ability_percent,

			1,	 // flower_bloom_cost,
			1.5, // flower_bloom_cooldown,
			33,	 // flower_wilt_chance,
			30,	 // flower_expand_percent,

			10, // spoil_time_base,
			50, // plantmass_spoil_time,
			20, // biomass_spoil_time,
			5,	// fruit_spoil_time,
			2,	// fruit_grow_percent,

			2,	// bark_grow_cooldown,	 // * lifespan multiplier
			95, // bark_plant_vs_thorn, // % to grow a plant cell (leaf/bark) vs a killer
			2,	// bark_grow_cost,		 // * energy density multiplier
			5,	// bark_max_integrity,	 // how many times bark can be "eaten" before it is broken through

			0.0625, // killer_tick_cost,	// * energy density multiplier
			1,		// killer_damage_cost, // * energy density multiplier

			4, // armor_health_bonus,

			20, // eye_max_seeing_distance,

	};

	float values[SettingNames::null];

public:
	WorldSettings()
	{
		memcpy(this->values, this->SettingsBase, SettingNames::null * sizeof(float));
	}

	float GetBase(SettingNames s);

	float Get(SettingNames s);

	void Set(SettingNames s, float value);

	/*

float SETTING_LIFESPAN(maxEnergy, nCells) LIFESPAN_MULTIPLIER * (maxEnergy / ENERGY_DENSITY_MULTIPLIER) * sqrt(nCells)
float SETTING_REPRODUCTION_COOLDOWN(maxEnergy, nCells, nLeaves) (REPRODUCTION_COOLDOWN_MULTIPLIER * (static_cast<double>(nCells) / sqrt(maxEnergy)))


*/
};

#ifndef Settings
extern WorldSettings Settings;
#endif

// void TickMain();

void AddImPlotColorMap();

void SetColorForCell(SDL_Renderer *r, Cell *c);

void DrawCell(SDL_Renderer *r, Cell *c, int x, int y);
