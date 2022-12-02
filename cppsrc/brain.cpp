#include "brain.h"
#include "rng.h"

Brain::Brain() : SimpleNets::DAGNetwork(2, {{SimpleNets::logistic, 4}}, {7, SimpleNets::logistic})
{
    this->nextSensorIndex = 0;
}

Brain::Brain(const Brain &b) : SimpleNets::DAGNetwork(b)
{
}

Brain::~Brain()
{
}

void Brain::TryAddRandomInputConnection()
{
    this->AddConnection(this->layers[0][randInt(0, this->layers[0].size())].Id(),
                        this->layers[1][randInt(0, this->layers[1].size())].Id(),
                        randFloat(-1.0, 1.0));
}

void Brain::TryAddRandomHiddenConnection()
{
    if (this->layers[1].size() > 1)
    {
        size_t firstIndex = randInt(0, this->layers[1].size());
        size_t secondIndex = randInt(0, this->layers[1].size() - 1);
        if (secondIndex == firstIndex)
        {
            if (secondIndex < this->layers[1].size() - 1)
            {
                if (secondIndex > 0)
                {
                    secondIndex += randPercent(50) ? -1 : 1;
                }
                else
                {
                    secondIndex++;
                }
            }
            else
            {
                secondIndex--;
            }
        }
        this->AddConnection(this->layers[1][firstIndex].Id(),
                            this->layers[1][secondIndex].Id(),
                            randFloat(-1.0, 1.0));
    }
}

void Brain::TryAddRandomOutputConnection()
{
    this->AddConnection(this->layers[1][randInt(0, this->layers[1].size())].Id(),
                        this->layers[2][randInt(0, this->layers[2].size())].Id(),
                        randFloat(-1.0, 1.0));
}

void Brain::SetBaselineInput(nn_num_t energyProportion, nn_num_t healthProportion)
{
    this->SetInput(0, {energyProportion, healthProportion});
}

void Brain::SetSensoryInput(unsigned int senseCellIndex, nn_num_t values[cell_null])
{
    std::vector<nn_num_t> valuesVector(cell_null);
    for (int i = 0; i < cell_null; i++)
    {
        valuesVector[i] = values[i];
    }
    this->SetInput(2 + (senseCellIndex * cell_null), valuesVector);
}

void Brain::Mutate()
{
}

unsigned int Brain::GetNewSensorIndex()
{
    for (int i = 0; i < cell_null; i++)
    {
        this->AddInput();
    }
    return this->nextSensorIndex++;
}

enum Intent Brain::Decide()
{
    // ok to cast to enum because the net chooses between discrete outputs and returns the index of the chosen one
    return static_cast<enum Intent>(this->Output());
}

