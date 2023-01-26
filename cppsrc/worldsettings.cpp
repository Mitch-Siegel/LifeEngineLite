#include "worldsettings.h"
#include "board.h"

#include "imgui.h"

#include <iostream>
#include <fstream>
#include <string>

WorldSettings::WorldSettings()
{
    std::ifstream loadFile;
    bool revertDefault = true;

    try
    {
        loadFile.open("./savedsettings.txt");

        // std::ifstream loadFile;
        if (loadFile.good())
        {
            int nLinesRead = 0;
            while (loadFile.good() && (nLinesRead < SettingNames::null))
            {
                std::string line;
                std::getline(loadFile, line);

                size_t pos = line.find('\t');
                std::string token = line.substr(0, pos);
                this->SettingsBase[nLinesRead] = stof(token);

                nLinesRead++;
            }

            if (nLinesRead >= SettingNames::null - 1)
            {
                revertDefault = false;
            }
        }
        else
        {
            std::cout << "No world settings file found" << std::endl;
        }
    }
    catch (const std::exception &exc)
    {
        std::cerr << exc.what();
        std::cerr << "Error parsing ./savedsettings.txt - expect first element on each line to be the value in float format\nYou can always delete ./savedsettings.txt and re-dump settings to get the file back to a good state" << std::endl;
    }

    if (revertDefault)
    {
        std::cout << "Loading default settings" << std::endl;
        this->usingDefaultSettings = true;
        memcpy(this->values, this->Default_SettingsBase, SettingNames::null * sizeof(float));
    }
    else
    {
        std::cout << "Loaded settings from ./savedsettings.txt" << std::endl;
        this->usingDefaultSettings = false;
        memcpy(this->values, this->SettingsBase, SettingNames::null * sizeof(float));
    }
}

float WorldSettings::GetBase(WorldSettings::SettingNames s)
{
    if (usingDefaultSettings)
    {
        return Default_SettingsBase[s];
    }
    else
    {
        return SettingsBase[s];
    }
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
        return this->values[s] * this->values[lifespan_multiplier];

    case leaf_flowering_ability_percent:
        return this->values[s];

    // flower
    case flower_bloom_cost:
        return this->values[s] * this->values[energy_density_multiplier];

    case flower_bloom_cooldown:
        return this->values[s] * this->values[lifespan_multiplier];
        
    case flower_wilt_chance:
    case flower_expand_percent:
    case spoil_time_base:
        return this->values[s];

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
    case bark_photosynthesis_bonus:
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

float WorldSettings::GetRaw(WorldSettings::SettingNames s)
{
    return this->values[s];
}

const char *WorldSettings::GetName(WorldSettings::SettingNames s)
{
    return this->names[s];
}

void WorldSettings::Set(WorldSettings::SettingNames s, float value)
{
    if (value < 0.0)
    {
        value = 0.0;
    }
    this->values[s] = value;
}

bool showWorldSettingsView = false;
void WorldSettingsView()
{
    ImGui::Begin("World Settings", &showWorldSettingsView, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Dump Settings"))
    {
        std::ofstream saveFile;
        saveFile.open("./savedsettings.txt");

        // std::ifstream loadFile;
        if (saveFile.good())
        {
            for (WorldSettings::SettingNames i = static_cast<WorldSettings::SettingNames>(0); i < WorldSettings::SettingNames::null; i = static_cast<WorldSettings::SettingNames>(static_cast<int>(i) + 1))
            {
                saveFile << Settings.GetRaw(i) << '\t' << Settings.GetName(i) << std::endl;
            }
        }
        else
        {
            std::cerr << "Unable to open ./savedsettings.txt to save settings" << std::endl;
        }
        saveFile.close();
    }

    int buttonId = 0;
    // this is horrible and i hate programming
    for (WorldSettings::SettingNames i = static_cast<WorldSettings::SettingNames>(0); i < WorldSettings::SettingNames::null; i = static_cast<WorldSettings::SettingNames>(static_cast<int>(i) + 1))
    {
        ImGui::Text("%s", Settings.GetName(i));
        float fraction = 0.001;
        for (int j = 0; j < 3; j++)
        {
            char label[128];
            sprintf(label, "-%2.1f%%", fraction * 100);

            ImGui::PushID(buttonId++);
            if (ImGui::Button(label))
            {
                Settings.Set(i, Settings.GetRaw(i) - (fraction * Settings.GetBase(i)));
            }
            ImGui::PopID();

            ImGui::SameLine();
            fraction *= 10;
        }

        ImGui::Text("%f", Settings.Get(static_cast<WorldSettings::SettingNames>(i)));
        ImGui::SameLine();

        fraction = .1;
        for (int j = 0; j < 3; j++)
        {
            char label[128];
            sprintf(label, "+%2.1f%%", fraction * 100);

            ImGui::PushID(buttonId++);
            if (ImGui::Button(label))
            {
                Settings.Set(i, Settings.GetRaw(i) + (fraction * Settings.GetBase(i)));
            }
            ImGui::PopID();

            ImGui::SameLine();
            fraction /= 10;
        }
        ImGui::NewLine();
        ImGui::NewLine();

        // ImGui::SameLine();
    }
    ImGui::End();
}