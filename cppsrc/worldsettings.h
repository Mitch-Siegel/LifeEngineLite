#include <string>

class WorldSettings
{

    class Setting
    {
    public:
        std::string name;
        std::string description;
        bool isInt;
        double value;
        std::pair<double, double> range;
        Setting();
        Setting(std::string name, std::string description, bool isInt, double baseValue, std::pair<double, double> range);

    };

public:
    enum SettingNames
    {
        // base settings
        day_length,
        do_day_night_cycle,
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
        photosynthesis_interval, // each leaf generates this much energy per tick
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

        // killer
        killer_cost_interval,   // * energy density multiplier
        killer_damage_cost, // * energy density multiplier

        // armor
        armor_health_bonus,

        // eye
        eye_max_seeing_distance,

        null

    };

private:
    void Initialize(); // set up the array of settings with defaults
    bool usingDefaultSettings;

    Setting settings[SettingNames::null];

public:
    WorldSettings();

    double Get(SettingNames s);
    
    int GetInt(SettingNames s);

    double GetRaw(SettingNames s);

    std::string GetName(WorldSettings::SettingNames s);

    std::string GetDescription(WorldSettings::SettingNames s);

    void Set(SettingNames s, double value);

    const WorldSettings::Setting &operator[](int index);

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
