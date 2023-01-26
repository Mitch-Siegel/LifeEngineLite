
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
        move_cost_multiplier,             // * energy density multiplier
        food_multiplier,                  // * energy density multiplier
        leaf_food_energy,                 // * food multiplier
        flower_food_energy,               // * food multiplier
        fruit_food_energy,                // * food multiplier
        plantmass_food_energy,            // * food multiplier
        biomass_food_energy,              // * food multiplier
                                          // leaf
        photosynthesis_energy_multiplier, // each leaf generates this much energy per tick
        leaf_flowering_cost,              // * energy density multiplier
        leaf_flowering_cooldown,          // number of ticks between leaf flowering
        leaf_flowering_ability_percent,   // % chance a new leaf cell will be able to flower
                                          // flower
        flower_bloom_cost,                // * energy density multiplier
        flower_bloom_cooldown,            // * lifespan multiplier
        flower_wilt_chance,               // each time the flower blooms, it has this % chance to wilt
        flower_expand_percent,            // % for wilting flower to become leaf
                                          // spoil times
        spoil_time_base,                  // in ticks
        plantmass_spoil_time,             // * spoiltime base
        biomass_spoil_time,               // * spoiltime base
        fruit_spoil_time,                 // * spoiltime base
        fruit_grow_percent,               // % for fruit to spontaneously grow a new organism (must roll 2x in a row)

        // bark
        bark_grow_cooldown,  // * lifespan multiplier
        bark_plant_vs_thorn, // % to grow a plant cell (leaf/bark) vs a killer
        bark_grow_cost,      // * energy density multiplier
        bark_max_integrity,  // how many times bark can be "eaten" before it is broken through
        bark_tick_cost,      // * energy density multiplier

        // killer
        killer_tick_cost,   // * energy density multiplier
        killer_damage_cost, // * energy density multiplier

        // armor
        armor_health_bonus,

        // eye
        eye_max_seeing_distance,

        null

    };

    const char *names[SettingNames::null + 1] =
        {
            "default mutability",
            "lifespan multiplier",
            // energy proportions (of max) required for certain actions
            "reproduction energy proportion",
            "reproduction cooldown multiplier (* lifespan multiplier)", // * lifespan multiplier
            "max health multiplier",
            // base energy stats
            "energy density multiplier",
            "move cost multiplier (* energy density multiplier)",                                          // * energy density multiplier
            "food multiplier (* energy density multiplier)",                                               // * energy density multiplier
            "leaf food energy (* food multiplier)",                                                        // * food multiplier
            "flower food energy (* food multiplier)",                                                      // * food multiplier
            "fruit food energy (* food multiplier)",                                                       // * food multiplier
            "plantmass food energy (* food multiplier)",                                                   // * food multiplier
            "biomass food energy (* food multiplier)",                                                     // * food multiplier
                                                                                                           // leaf
            "photosynthesis energy multiplier",                                                            // each leaf generates this much energy per tick
            "leaf flowering cost (* energy density multiplier)",                                           // * energy density multiplier
            "leaf flowering cooldown",                                                                     // number of ticks between leaf flowering
            "leaf flowering ability percent",                                                              // % chance a new leaf cell will be able to flower
                                                                                                           // flower
            "flower bloom cost (* energy density multiplier)",                                             // * energy density multiplier
            "flower bloom cooldown (* lifespan multiplier)",                                               // * lifespan multiplier
            "flower wilt chance",                                                                          // each time the flower blooms, it has this % chance to wilt
            "flower expand percent",                                                                       // % for wilting flower to become leaf
                                                                                                           // spoil times
            "spoiltime base",                                                                              // in ticks
            "plantmass spoil time (* spoiltime base)",                                                     // * spoiltime base
            "biomass spoil time (* spoiltime base)",                                                       // * spoiltime base
            "fruit spoil time (* spoiltime base)",                                                         // * spoiltime base
            "fruit grow percent (to spontaneously grow new organism upon spoil)\n(must roll 2x in a row)", // % for fruit to spontaneously grow a new organism

            // bark
            "bark grow cooldown (* lifespan multiplier)",   // * lifespan multiplier
            "bark plant vs thorn",                          // % to grow a plant cell (leaf/bark) vs a killer
            "bark grow cost (* energy density multiplier)", // * energy density multiplier
            "bark max integrity",                           // how many times bark can be "eaten" before it is broken through
            "bark tick cost",                               // * energy density multipler
            // killer
            "killer tick cost (* energy density multiplier)",   // * energy density multiplier
            "killer damage cost (* energy density multiplier)", // * energy density multiplier

            // armor
            "armor health bonus",

            // eye
            "eye max seeing distance",

            "null"};

private:
    bool usingDefaultSettings;

    constexpr static float Default_SettingsBase[SettingNames::null] =
        {
            // base settings
            15,    // default_mutability,
            15,    // lifespan_multiplier,
            0.7,   // reproduction_energy_proportion,
            0,     // reproduction_cooldown_multiplier,
            1,     // max_health_multiplier,
            4.0,   // energy_density_multiplier,
            0.075, // move_cost_multiplier,
            2.0,   // food_multiplier,
            1,     // leaf_food_energy,
            3,     // flower_food_energy,
            4,     // fruit_food_energy,
            2,     // plantmass_food_energy,
            8,     // biomass_food_energy,
            0.3,   // photosynthesis_energy_multiplier,
            3,     // leaf_flowering_cost,
            45,    // leaf_flowering_cooldown, // * lifespan multiplier
            45,    // leaf_flowering_ability_percent,

            4,  // flower_bloom_cost,
            7,  // flower_bloom_cooldown, // * lifespan multiplier
            33, // flower_wilt_chance,
            30, // flower_expand_percent,

            10,  // spoil_time_base,
            50, // plantmass_spoil_time,
            15,  // biomass_spoil_time,
            5,   // fruit_spoil_time,
            7,   // fruit_grow_percent,

            2,   // bark_grow_cooldown,	 // * lifespan multiplier
            95,  // bark_plant_vs_thorn, // % to grow a plant cell (leaf/bark) vs a killer
            6,   // bark_grow_cost,		 // * energy density multiplier
            3,   // bark_max_integrity,	 // how many times bark can be "eaten" before it is broken through
            0.3, // bark_tick_cost      // * energy density multiplier

            0.4, // killer_tick_cost,	// * energy density multiplier
            1,     // killer_damage_cost, // * energy density multiplier

            4, // armor_health_bonus,

            20, // eye_max_seeing_distance,

    };

    float SettingsBase[SettingNames::null];

    float values[SettingNames::null];

public:
    WorldSettings();

    float GetBase(SettingNames s);

    float Get(SettingNames s);

    float GetRaw(SettingNames s);

    const char *GetName(WorldSettings::SettingNames s);

    void Set(SettingNames s, float value);

    /*

float SETTING_LIFESPAN(maxEnergy, nCells) LIFESPAN_MULTIPLIER * (maxEnergy / ENERGY_DENSITY_MULTIPLIER) * sqrt(nCells)
float SETTING_REPRODUCTION_COOLDOWN(maxEnergy, nCells, nLeaves) (REPRODUCTION_COOLDOWN_MULTIPLIER * (static_cast<double>(nCells) / sqrt(maxEnergy)))


*/
};

void WorldSettingsView();

#ifndef Settings
extern WorldSettings Settings;
extern bool showWorldSettingsView;
#endif
