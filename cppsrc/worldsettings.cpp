#include "worldsettings.h"
#include "board.h"

#include "imgui.h"

#include <iostream>
#include <fstream>
#include <string>

WorldSettings::WorldSettings()
{
	this->Initialize();
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
				this->settings[nLinesRead].value = stof(token);

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
		std::cout << "Using default settings" << std::endl;
	}
	else
	{
		std::cout << "Loaded settings from ./savedsettings.txt" << std::endl;
		this->usingDefaultSettings = false;
	}
}

WorldSettings::Setting::Setting(std::string name, std::string description, bool isInt, double baseValue, std::pair<double, double> range)
{
	this->name = name;
	this->description = description;
	this->isInt = isInt;
	this->value = baseValue;
	this->range = range;
}
WorldSettings::Setting::Setting()
{
	this->name = "Uninitialized Setting";
	this->description = "Uninitialized Setting";
	this->isInt = false;
	this->value = -9999.9999;
	this->range = {-9999.9999, -9999.9999};
}

void WorldSettings::Initialize()
{
	this->settings[WorldSettings::default_mutability] = Setting("Default Mutability (%)",
																"Base percent chance for an organism to mutate upon reproduction",
																true,
																15,
																{0, 100});

	this->settings[WorldSettings::lifespan_multiplier] = Setting("Lifespan Multiplier",
																 "Multiplier on the formula of max energy and cell count which determines lifespan",
																 true,
																 75,
																 {0, 9999});

	this->settings[WorldSettings::reproduction_energy_proportion] = Setting("Reproduction Energy Proportion (%)",
																			"Percent of maximum energy expended during reproduction (must have 1.1x this amount to reproduce)",
																			false,
																			75,
																			{0, 90.9});

	this->settings[WorldSettings::reproduction_cooldown_multiplier] = Setting("Reproduction Cooldown Multiplier",
																			  "Multiplier on the formula of max energy and cell count which determines how frequently an organism can reproduce",
																			  false,
																			  0.0,
																			  {0, 9999});

	this->settings[WorldSettings::max_health_multiplier] = Setting("Max Health Multiplier",
																   "Multiplier on the formula of max health (1 per cell + extra for armor)",
																   true,
																   1,
																   {1, 9999});

	this->settings[WorldSettings::energy_density_multiplier] = Setting("Energy Density Multiplier",
																	   "Multiplier on organism max energy (determined based on energy content of constituent cells)",
																	   true,
																	   4,
																	   {1, 9999});

	this->settings[WorldSettings::move_cost_multiplier] = Setting("Move Cost Multiplier",
																  "Multiplier on formula of cell count which termines cost for an organism to move once",
																  false,
																  1,
																  {0, 9999});

	this->settings[WorldSettings::food_multiplier] = Setting("Food Multiplier",
															 "Multiplier on energy gained for all food types",
															 true,
															 4,
															 {0, 9999});

	this->settings[WorldSettings::leaf_food_energy] = Setting("Leaf Food Energy",
															  "Energy gained by herbivores when eating one leaf",
															  true,
															  1,
															  {0, 9999});

	this->settings[WorldSettings::flower_food_energy] = Setting("Flower Food Energy",
																"Energy gained by herbivores when eating one flower",
																true,
																2,
																{0, 9999});

	this->settings[WorldSettings::fruit_food_energy] = Setting("Fruit Food Energy",
															   "Energy gained by herbivores when eating one fruit",
															   true,
															   4,
															   {0, 9999});

	this->settings[WorldSettings::plantmass_food_energy] = Setting("Plantmass Food Energy",
																   "Energy gained by herbivores when eating one plantmass",
																   true,
																   2,
																   {0, 9999});

	this->settings[WorldSettings::biomass_food_energy] = Setting("Biomass Food Energy",
																 "Energy gained by carnivores when eating one biomass",
																 true,
																 16,
																 {0, 9999});

	this->settings[WorldSettings::photosynthesis_interval] = Setting("Photosynthesis Interval",
																	 "Interval each leaf cell waits between generating 1 energy",
																	 true,
																	 10,
																	 {0, 9999});

	this->settings[WorldSettings::leaf_flowering_cost] = Setting("Leaf Flowering Cost",
																 "Cost for a leaf to flower",
																 true,
																 3,
																 {0, 9999});

	this->settings[WorldSettings::leaf_flowering_cooldown] = Setting("Leaf Flowering Cooldown",
																	 "Interval each leaf waits before trying to flower again",
																	 true,
																	 200,
																	 {0, 9999});

	this->settings[WorldSettings::leaf_flowering_ability_percent] = Setting("Leaf Flowering Ability (%)",
																			"What percent of leaf cells will be able to flower",
																			true,
																			30,
																			{0, 100});

	this->settings[WorldSettings::flower_bloom_cost] = Setting("Flower Bloom Cost",
															   "Energy expended for a flower to bloom and generate a fruit",
															   true,
															   16,
															   {0, 9999});

	this->settings[WorldSettings::flower_bloom_cooldown] = Setting("Flower Bloom Cooldown",
																   "Interval each flower waits before trying to bloom again",
																   true,
																   70,
																   {0, 9999});

	this->settings[WorldSettings::flower_wilt_chance] = Setting("Flower Wilt Chance (%)",
																"Percent each flower will wilt after blooming",
																true,
																33,
																{0, 100});

	this->settings[WorldSettings::flower_expand_percent] = Setting("Flower Expand Chance (%)",
																   "Percent a flower will become a new leaf when it wilts vs. simply decaying",
																   true,
																   30,
																   {0, 100});

	this->settings[WorldSettings::spoil_time_base] = Setting("Spoil Time Multiplier",
															 "Multiplier on time it takes plantmass/biomass/fruit to spoil",
															 true,
															 10,
															 {0, 9999});

	this->settings[WorldSettings::plantmass_spoil_time] = Setting("Plantmass Spoil Time",
																  "Base time it takes plantmass to decay (dependent on organism size which became plantmass)",
																  true,
																  50,
																  {0, 9999});

	this->settings[WorldSettings::biomass_spoil_time] = Setting("Biomass Spoil Time",
																"Base time it takes biomass to decay (dependent on organism size which became plantmass)",
																true,
																5,
																{0, 9999});

	this->settings[WorldSettings::fruit_spoil_time] = Setting("Fruit Spoil Time",
															  "Base time it takes fruit to decay",
															  true,
															  7,
															  {0, 9999});

	this->settings[WorldSettings::fruit_grow_percent] = Setting("Fruit Grow Chance (%)",
																"Chance for a fruit to spontaneously generate a new organism rather than spoiling - must roll twice in a row",
																true,
																7,
																{0, 100});

	this->settings[WorldSettings::bark_grow_cooldown] = Setting("Bark Grow Cooldown",
																"Interval each bark waits before trying to grow again if possible",
																true,
																25,
																{0, 9999});

	this->settings[WorldSettings::bark_plant_vs_thorn] = Setting("Bark Grow Leaf vs Thorn (%)",
																"Percent for a bark to grow a new leaf vs a new thorn",
																true,
																95,
																{0, 100});

	this->settings[WorldSettings::bark_grow_cost] = Setting("Bark Grow Cost",
															"Energy cost for a bark cell to grow",
															true,
															24,
															{0, 9999});

	this->settings[WorldSettings::bark_max_integrity] = Setting("Bark Max Integrity",
																"Number of \"chomps\" from an herbivore a bark can withstand before breaking",
																true,
																3,
																{1, 9999});

	this->settings[WorldSettings::killer_cost_interval] = Setting("Killer Cost Interval",
																  "Each killer will cost its organism 1 energy this frequently",
																  true,
																  10,
																  {0, 9999});

	this->settings[WorldSettings::killer_damage_cost] = Setting("Killer Damage Cost",
																"Energy cost for a killer to do 1 damage",
																true,
																4,
																{0, 9999});

	this->settings[WorldSettings::armor_health_bonus] = Setting("Armor Health Bonus",
																"Each armor cell will contribute this much extra health",
																true,
																4,
																{0, 9999});

	this->settings[WorldSettings::eye_max_seeing_distance] = Setting("Eye Max Seeing Distance",
																	 "How far an eye cell can see",
																	 true,
																	 20,
																	 {0, 9999});
}

double WorldSettings::Get(WorldSettings::SettingNames s)
{
	switch (s)
	{
	case default_mutability:
	case lifespan_multiplier:
	case reproduction_energy_proportion:
	case reproduction_cooldown_multiplier:
	case max_health_multiplier:
	case energy_density_multiplier:
	case move_cost_multiplier:
	case food_multiplier:
		return this->settings[s].value;

	case leaf_food_energy:
	case flower_food_energy:
	case fruit_food_energy:
	case plantmass_food_energy:
	case biomass_food_energy:
		return this->settings[s].value * this->settings[food_multiplier].value;

	// leaf
	case photosynthesis_interval:
	case leaf_flowering_cost:
	case leaf_flowering_cooldown:
	case leaf_flowering_ability_percent:
	case flower_bloom_cost:
	case flower_bloom_cooldown:
	case flower_wilt_chance:
	case flower_expand_percent:
	case spoil_time_base:
		return this->settings[s].value;

	case plantmass_spoil_time:
	case biomass_spoil_time:
		return this->settings[s].value * this->settings[spoil_time_base].value;

	case fruit_spoil_time:
	case fruit_grow_percent:
	case bark_grow_cooldown:
	case bark_plant_vs_thorn:
	case bark_grow_cost:
	case bark_max_integrity:
	case killer_cost_interval:
	case killer_damage_cost:
	case armor_health_bonus:
	case eye_max_seeing_distance:
		return this->settings[s].value;

	case null:
	default:
		printf("null/unexpected setting passed to WorldSettings::Get()\n");
		exit(1);
	}
}

int WorldSettings::GetInt(SettingNames s)
{
	if(!this->settings[s].isInt)
	{
		printf("Call to WorldSettings::GetInt for non-int setting!");
		exit(1);
	}
	return static_cast<int>(this->Get(s));
}

double WorldSettings::GetRaw(WorldSettings::SettingNames s)
{
	return this->settings[s].value;
}

std::string WorldSettings::GetName(WorldSettings::SettingNames s)
{
	return this->settings[s].name;
}

void WorldSettings::Set(WorldSettings::SettingNames s, double value)
{
	const std::pair<double, double> &range = this->settings[s].range;
	if (value < range.first)
	{
		value = range.first;
	}
	else if (value > range.second)
	{
		value = range.second;
	}

	if (this->settings[s].isInt)
	{
		value = round(value);
	}
	this->settings[s].value = value;
}

const WorldSettings::Setting &WorldSettings::operator[](int index)
{
	return this->settings[index];
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
		ImGui::Text("%s", Settings[i].name.c_str());
		bool isInt = Settings[i].isInt;
		float fraction = isInt ? 1.0 : 0.1;
		for (int j = 0; j < 3; j++)
		{
			char label[128];
			sprintf(label, isInt ? "-%2.0f" : "-%2.1f", fraction);

			ImGui::PushID(buttonId++);
			if (ImGui::Button(label))
			{
				Settings.Set(i, Settings.GetRaw(i) - fraction);
			}
			ImGui::PopID();

			ImGui::SameLine();
			fraction *= 10;
		}

		ImGui::Text("%f", Settings.Get(static_cast<WorldSettings::SettingNames>(i)));
		ImGui::SameLine();

		fraction = isInt ? 100.0 : 10.0;
		for (int j = 0; j < 3; j++)
		{
			char label[128];
			sprintf(label, isInt ? "+%2.0f" : "+%2.1f", fraction);

			ImGui::PushID(buttonId++);
			if (ImGui::Button(label))
			{
				Settings.Set(i, Settings.GetRaw(i) + fraction);
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