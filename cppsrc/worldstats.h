#include <stddef.h>
#include <stdint.h>
#include <map>

#include "config.h"
#include "datatracker.h"

#pragma once
class Board;
class WorldStats
{
	friend class Board;
private:
	enum Counts
	{
		count_cells,
		count_energy,
		count_maxenergy,
		count_vitality,
		count_age,
		count_lifespan,
		count_reproductioncooldown,
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

	// x-axis for tick values
	DataTracker<int> tickData = DataTracker<int>(2500);
	DataTracker<double> tickDataDouble = DataTracker<double>(2500);

	// history of number of class members by class
	DataTracker<int> *classCountData[class_null + 1];

	// history of proportion of energy total for each class
	DataTracker<double> *classEnergyProportionData[class_null];

	// history of number of active species
	DataTracker<int> activeSpeciesData = DataTracker<int>(2500);

	// mapping from number of species members to count of species with this many members
	std::map<uint32_t, uint32_t> nSpeciesBySize;

	void DisplayGeneralInfoTable();

	int whichGraph;
	void DisplayHistoryGraphs();

protected:
	// assumed to be called during Board::Tick()
	// this implies the board's mutex is locked so data can be read consistently
	void Update(Board *b);

public:
	WorldStats();

	~WorldStats();

	void Display();
};
