#include "brain.h"
#include "rng.h"

CellSenseActivation::CellSenseActivation()
{
    for (int i = 0; i < cell_null; i++)
    {
        this->values[i] = randFloat(-1.0, 1.0);
    }
}

nn_num_t &CellSenseActivation::operator[](unsigned int index)
{
    return this->values[index];
}

/*
 * Default brain inputs:
 * random
 * energy proportion
 * health proportion
 *
 *
 */
#define BRAIN_DEFAULT_INPUTS 1

Brain::Brain() : SimpleNets::DAGNetwork(BRAIN_DEFAULT_INPUTS, {}, {7, SimpleNets::logistic})
{
    this->nextSensorIndex = 0;
    this->freeWill = randFloat(-1.0, 1.0);

    this->AddNeuron(SimpleNets::logistic);
    for (int i = 0; i < 10; i++)
    {

        size_t newId = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(SimpleNets::logistic));
        this->AddConnection(this->layers[0][0].Id(), newId, 0.0); // bias connection

        while (!this->TryAddRandomHiddenConnectionByDst(newId))
            ;

        while (!this->TryAddRandomHiddenConnectionBySrc(newId))
            ;

        while (!this->TryAddRandomInputConnectionByDst(newId))
            ;

        while (!this->TryAddRandomOutputConnectionBySrc(newId))
            ;
    }

    // for (auto output = this->layers[2].begin(); output != this->layers[2].end(); ++output)
    // {
    // this->AddConnection(this->layers[0][0].Id(), (*output)->Id(), randFloat(-1.0, 1.0));
    // }
}

Brain::Brain(const Brain &b) : SimpleNets::DAGNetwork(b)
{
    this->nextSensorIndex = b.nextSensorIndex;
}

bool Brain::TryAddRandomInputConnectionBySrc(size_t srcId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    return this->AddConnection(srcId,
                               this->layers[1][randInt(0, this->size(1) - 1)].Id(),
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomInputConnectionByDst(size_t dstId)
{
    return this->AddConnection(this->layers[0][randInt(0, this->size(0) - 1)].Id(),
                               dstId,
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomInputConnection()
{
    if (this->size(1) == 0)
    {
        return true;
    }
    return this->TryAddRandomInputConnectionByDst(this->layers[1][randInt(0, this->size(1) - 1)].Id());
}

bool Brain::TryAddRandomInputOutputConnectionBySrc(size_t srcId)
{
    return this->AddConnection(srcId,
                               this->layers[2][randInt(0, this->size(2) - 1)].Id(),
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomInputOutputConnectionByDst(size_t dstId)
{
    return this->AddConnection(this->layers[0][randInt(0, this->size(0) - 1)].Id(),
                               dstId,
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomInputOutputConnection()
{
    return this->TryAddRandomInputOutputConnectionByDst(this->layers[2][randInt(0, this->size(2) - 1)].Id());
}

bool Brain::TryAddRandomHiddenConnectionBySrc(size_t srcId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    size_t dstId;
    if (this->size(1) > 1 ||
        ((this->size(1) > 0) && this->layers[1][0].Id() != srcId))
    {
        size_t dstIndex = randInt(0, this->size(1) - 1);
        // this is a bit janky because it just rerolls until it doesn't collide
        while ((dstId = this->layers[1][dstIndex].Id()) == srcId)
        {
            dstIndex = randInt(0, this->size(1) - 1);
        }

        return this->AddConnection(srcId,
                                   dstId,
                                   randFloat(-1.0, 1.0));
    }
    return false;
}

bool Brain::TryAddRandomHiddenConnectionByDst(size_t dstId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    size_t srcId;
    if (this->size(1) > 1 ||
        ((this->size(1) > 0) && this->layers[1][0].Id() != dstId))
    {
        size_t srcIndex = randInt(0, this->size(1) - 1);
        // this is a bit janky because it just rerolls until it doesn't collide
        while ((srcId = this->layers[1][srcIndex].Id()) == dstId)
        {
            srcIndex = randInt(0, this->size(1) - 1);
        }

        return this->AddConnection(srcId,
                                   dstId,
                                   randFloat(-1.0, 1.0));
    }
    return false;
}

bool Brain::TryAddRandomHiddenConnection()
{
    if (this->size(1) == 0)
    {
        return true;
    }

    return this->TryAddRandomHiddenConnectionByDst(this->layers[1][randInt(0, this->size(1) - 1)].Id());
}

bool Brain::TryAddRandomOutputConnectionBySrc(size_t srcId)
{
    return this->AddConnection(srcId,
                               this->layers[2][randInt(0, this->size(2) - 1)].Id(),
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomOutputConnectionByDst(size_t dstId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    return this->AddConnection(this->layers[1][randInt(0, this->size(1) - 1)].Id(),
                               dstId,
                               randFloat(-1.0, 1.0));
}

bool Brain::TryAddRandomOutputConnection()
{
    return this->TryAddRandomOutputConnectionByDst(this->layers[2][randInt(0, this->size(2) - 1)].Id());
}

void Brain::SetBaselineInput(nn_num_t energyProportion, nn_num_t healthProportion)
{
    this->freeWill = randFloat(0.0, 1.0);

    this->SetInput(0, {this->freeWill});
}

void Brain::SetSensoryInput(unsigned int senseCellIndex, nn_num_t value)
{
    std::vector<nn_num_t> valueVec;
    valueVec.push_back(value);
    this->SetInput((BRAIN_DEFAULT_INPUTS - 1) + senseCellIndex, valueVec);
}

void Brain::AddRandomHiddenNeuron()
{
    size_t newNeuronId = this->AddNeuron(SimpleNets::perceptron);
    this->AddConnection(this->layers[0][0].Id(), newNeuronId, randFloat(-1.0, 1.0)); // bias connection

    bool usedHidden = false;
    // determine this neuron's inputs (input layer vs hidden layer)
    if (randPercent(50))
    {
        int nInputs = randInt(1, floor(sqrt(this->size(0))));
        while (nInputs > 0)
        {
            nInputs -= !this->TryAddRandomInputConnectionByDst(newNeuronId);
        }
    }
    else
    {
        int nInputs = randInt(1, floor(sqrt(this->size(1))));
        while (nInputs > 0)
        {
            nInputs -= !this->TryAddRandomHiddenConnectionByDst(newNeuronId);
        }
        usedHidden = true;
    }

    // determine this neuron's outputs (input layer vs output layer)
    if (randPercent(50) && !usedHidden)
    {
        int nOutputs = randInt(1, floor(sqrt(this->size(1))));
        while (nOutputs > 0)
        {
            nOutputs -= !this->TryAddRandomHiddenConnectionBySrc(newNeuronId);
        }
    }
    else
    {
        int nOutputs = randInt(1, floor(sqrt(this->size(2))));
        while (nOutputs > 0)
        {
            nOutputs -= !this->TryAddRandomOutputConnectionBySrc(newNeuronId);
        }
    }
}

void Brain::Mutate()
{
    // add/remove a neuron with 20% probability
    if (randPercent(20))
    {
        // 49% to remove a hidden neuron, 51% to add one (generally trend towards more complex brains)
        if (this->size(1) > 1 && randPercent(49))
        {
            this->RemoveUnit(this->layers[1][randInt(0, this->size(1) - 1)].Id());
        }
        else
        {
            // generate a completely unconnected neuron, rely on further mutations to connect to it
            size_t id = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(SimpleNets::perceptron));
            this->AddConnection(this->layers[0][0].Id(), id, randFloat(-1.0, 1.0)); // bias connection
            this->AddConnection(this->layers[0][0].Id(), id, randFloat(-1.0, 1.0));
            // this->AddRandomHiddenNeuron();
        }
    }
    // add/remove/modify connection with 80% probability
    else
    {
        // modify existing connection with 50% probability
        if (randPercent(50) && this->connections().size() > 0)
        {
            auto toModify = this->connections().begin();
            int i = 0;
            if (this->connections().size() > 2)
            {
                i = randInt(0, this->connections().size() - 2);
            }
            while (i-- > 0)
            {
                ++toModify;
            }

            this->ChangeWeight(toModify->first.first, toModify->first.second, randFloat(-0.25, 0.25));
        }
        else // add/remove connection
        {
            // 51% to add a connection, 49% to remove one (generally trend towards more complex brains)
            if (randPercent(51) || this->connections().size() < 3) // add connection
            {
                int nTries = 0;
                bool couldAdd = false;
                while (!couldAdd && (nTries++ < 20))
                {
                    switch (randInt(0, 3))
                    {
                    case 0:
                        couldAdd = !this->TryAddRandomInputConnection();
                        break;

                    case 1:
                    case 2:
                        couldAdd = !this->TryAddRandomHiddenConnection();
                        break;

                    case 3:
                        couldAdd = !this->TryAddRandomOutputConnection();
                        break;

                        // case 4:
                        // couldAdd = !this->TryAddRandomInputOutputConnection();
                        // break;
                    }
                }
            }
            else // remove connection
            {
                auto toRemove = this->connections().begin();
                int i = 0;
                if (this->connections().size() > 2)
                {
                    i = randInt(0, this->connections().size() - 2);
                }
                while (i-- > 0)
                {
                    ++toRemove;
                }
                this->RemoveConnection((*toRemove).second);
            }
        }
    }
}

unsigned int Brain::GetNewSensorIndex()
{

    size_t inputId = this->AddInput();
    this->AddNeuron(SimpleNets::perceptron);
    int nConnections = randInt(1, (this->size(1)));
    while (nConnections > 0)
    {
        if (randPercent(50))
        {
            nConnections -= (!this->TryAddRandomHiddenConnectionBySrc(inputId));
        }
    }
    return this->nextSensorIndex++;
}

enum Intent Brain::Decide()
{
    enum Intent thisAction = static_cast<enum Intent>(this->Output());
    if(thisAction == this->lastAction)
    {
        this->nTicksSameAction++;
        if(this->nTicksSameAction > 20)
        {
            enum Intent newAction = randPercent(50) ? intent_rotate_clockwise : intent_rotate_counterclockwise;
            this->lastAction = newAction;
            return newAction;
        }
    }
    // ok to cast to enum because the net chooses between discrete outputs and returns the index of the chosen one
    return static_cast<enum Intent>(thisAction);
}
