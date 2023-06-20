#pragma once

enum CellTypes
{
	cell_empty,
	cell_plantmass,
	cell_biomass,
	cell_leaf,
	cell_bark,
	cell_flower,
	cell_fruit,
	cell_mouth,
	cell_mover,
	cell_killer,
	cell_armor,
	cell_touch,
	cell_eye,
	cell_null,
};

enum OrganismClassifications
{
	class_plant,
	class_mover,
	class_null,
};

extern int CellEnergyDensities[cell_null];
