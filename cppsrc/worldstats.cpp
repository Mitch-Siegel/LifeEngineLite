#include "worldstats.h"

#include "imgui.h"

#include "util.h"
#include "board.h"

WorldStats::WorldStats()
{
	for (int i = 0; i < class_null + 1; i++)
	{
		this->classCountData[i] = new DataTracker<int>(2500);
	}

	for (int i = 0; i < class_null; i++)
	{
		this->classEnergyProportionData[i] = new DataTracker<double>(2500);
	}
	this->whichGraph = 0;
}

WorldStats::~WorldStats()
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

void WorldStats::DisplayGeneralInfoTable()
{
	if (ImGui::BeginTable("OrganismStats", class_null + 1))
	{
		const char *rowNames[count_null + 2] = {"Class:", "Count", "Cells", "Energy%", "Vitality", "Max Energy/Cell", "Max Energy", "Age", "Lifespan", "Reproduction cooldown", "Mutability", "Inner Neurons", "Synapses"};
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
			ImGui::Text("%.2f", organismStats[i][count_cells]);
		}
		row++;

		// energy %
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.1f", 100.0 * (organismStats[i][count_energy] / organismStats[i][count_maxenergy]));
		}
		row++;

		// vitality
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.2lf", 100.0 * (organismStats[i][count_vitality]));
		}
		row++;

		// energy / cell
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.2f", (organismStats[i][count_maxenergy] / organismStats[i][count_cells]));
		}
		row++;

		// max energy
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.1f", organismStats[i][count_maxenergy]);
		}
		row++;

		// age
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.0f", (organismStats[i][count_age]));
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

		// reproduction cooldown
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", rowNames[row]);
		for (int i = 0; i < class_null; i++)
		{
			ImGui::TableSetColumnIndex(i + 1);
			ImGui::Text("%.1f", organismStats[i][count_reproductioncooldown]);
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
}

void WorldStats::DisplayHistoryGraphs()
{
	ImGui::RadioButton("Organism Counts", &this->whichGraph, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Energy info by class", &this->whichGraph, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Number of species", &this->whichGraph, 2);

	switch (this->whichGraph)
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
	}
	break;

	case 2:
	{
		std::map<uint32_t, uint32_t> mapCopy = nSpeciesBySize;
		unsigned int maxRows = 25;
		if(maxRows > mapCopy.size())
		{
			maxRows = mapCopy.size();
		}

		auto mapIterator = mapCopy.rbegin();
		for(unsigned int i = 0; i < maxRows; i++)
		{
			if(mapIterator->second > 1)
			{
			ImGui::Text("%3d species have >=%5d members", mapIterator->second, static_cast<int>(pow(2, mapIterator->first)));
			}
			else
			{
				ImGui::Text("%3d species has  >=%5d members", mapIterator->second, static_cast<int>(pow(2, mapIterator->first)));
			}
			++mapIterator;
		}
		/*
		ImPlot::SetNextAxesToFit();
		if (ImPlot::BeginPlot("Sizes of currently active species", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit))
		{
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

			std::map<uint32_t, uint32_t> mapCopy = nSpeciesBySize;
			std::vector<int> x(mapCopy.size());
			std::vector<int> y(mapCopy.size());
			auto mapIterator = mapCopy.begin();

			int maxY = 0;
			for (size_t i = 0; i < mapCopy.size(); i++)
			{
				int thisY = mapIterator->first + 1;
				x[i] = mapIterator->second;
				y[i] = thisY;
				if(thisY > maxY)
				{
					maxY = thisY;
				}

				printf("%d:%d\n", x[i], y[i]);

				++mapIterator;
			}
			// ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, maxY);

			printf("\n");
			ImPlot::PlotBars("", x.data(), y.data(), x.size(), 1.0);
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

void WorldStats::Display()
{
	this->DisplayGeneralInfoTable();
	this->DisplayHistoryGraphs();
}

void WorldStats::Update(Board *board)
{
	// zero out counts
	for (int i = 0; i < class_null; i++)
	{
		for (int j = 0; j < count_null; j++)
		{
			this->organismStats[i][j] = 0.0;
		}
		classCounts[i] = 0;
		for (int j = 0; j < class_null; j++)
		{
			this->organismCellCounts[i][j] = 0.0;
		}

		this->totalClassEnergies[i] = 0;
	}

	// iterate all organisms, add their info to the stats
	for (Organism *o : board->Organisms)
	{
		enum OrganismClassifications thisClass = board->GetSpeciesInfo(o->Identifier().Species()).classification;
		this->classCounts[thisClass]++;

		this->organismStats[thisClass][count_cells] += o->nCells();

		size_t thisEnergy = o->Energy();
		this->organismStats[thisClass][count_energy] += thisEnergy;
		this->totalClassEnergies[thisClass] += thisEnergy;

		this->organismStats[thisClass][count_maxenergy] += o->MaxEnergy();

		this->organismStats[thisClass][count_vitality] += o->Vitality();

		this->organismStats[thisClass][count_age] += o->age;
		this->organismStats[thisClass][count_lifespan] += o->lifespan;
		this->organismStats[thisClass][count_reproductioncooldown] += o->reproductionCooldown;
		this->organismStats[thisClass][count_mutability] += o->mutability;
		this->organismStats[thisClass][count_neurons] += o->brain->NHidden();
		this->organismStats[thisClass][count_synapses] += o->brain->SynapseCount();
		this->organismStats[thisClass][count_raw]++;
		for (int i = 0; i < cell_null; i++)
		{
			organismCellCounts[thisClass][i] += o->cellCounts[i];
		}
		if (o->cellCounts[cell_touch] > 0)
		{
			touchSensorHaverCounts[thisClass]++;
		}
	}

	for (int i = 0; i < class_null; i++)
	{
		int thisClassSize = classCounts[i];
		for (int j = 0; j < count_null; j++)
		{
			this->organismStats[i][j] /= thisClassSize;
		}
	}

	this->tickData.Add(board->tickCount);
	this->tickDataDouble.Add(static_cast<double>(board->tickCount));
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
		this->classEnergyProportionData[i]->Add(totalClassEnergies[i] / static_cast<double>(totalEnergy));
	}

	this->activeSpeciesData.Add(board->activeSpecies().size());

	nSpeciesBySize.clear();
	for (uint32_t species : board->activeSpecies())
	{
		const SpeciesInfo &info = board->GetSpeciesInfo(species);
		nSpeciesBySize[static_cast<unsigned int>(log2(info.count))]++;
	}
}

void WorldStats::Reset()
{
	for (int i = 0; i < class_null + 1; i++)
	{
		this->classCountData[i]->Reset();
	}

	for (int i = 0; i < class_null; i++)
	{
		this->classEnergyProportionData[i]->Reset();
	}
	this->whichGraph = 0;


	// x-axis for tick values
	tickData.Reset();
	tickDataDouble.Reset();


	// history of number of active species
	activeSpeciesData.Reset();

	// mapping from number of species members to count of species with this many members
	nSpeciesBySize.clear();
}