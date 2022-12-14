#include "brain.h"
#include "rng.h"

/*
 * Default brain inputs:
 * random
 * energy proportion
 * health proportion
 *
 *
 */
#define BRAIN_DEFAULT_INPUTS 4

Brain::Brain() : SimpleNets::DAGNetwork(BRAIN_DEFAULT_INPUTS, {}, {7, SimpleNets::logistic})
{
    this->nextSensorIndex = 0;
    this->freeWill[0] = randFloat(0.0, 1.0);
    this->freeWill[1] = randFloat(0.0, 1.0);

    for (int i = 0; i < 2; i++)
    {

        size_t newId = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(randInt(SimpleNets::logistic, SimpleNets::perceptron)));
        while (!this->TryAddRandomInputConnectionByDst(newId))
            ;

        while (!this->TryAddRandomOutputConnectionBySrc(newId))
            ;
    }

    int nIO = randInt(0, 2);
    while (nIO -= this->TryAddRandomInputOutputConnection())
        ;
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
    this->freeWill[0] += randFloat(-0.25, 0.25);
    if (this->freeWill[0] > 1.0)
    {
        this->freeWill[0] = 1.0;
    }
    else if (this->freeWill[0] < 0.0)
    {
        this->freeWill[0] = 0.0;
    }
    this->freeWill[1] = randFloat(0.0, 1.0);

    this->SetInput(0, {this->freeWill[0], this->freeWill[1], energyProportion, healthProportion});
}

void Brain::SetSensoryInput(unsigned int senseCellIndex, nn_num_t values[cell_null])
{
    std::vector<nn_num_t> valuesVector(cell_null);
    for (int i = 0; i < cell_null; i++)
    {
        valuesVector[i] = values[i];
    }
    this->SetInput((BRAIN_DEFAULT_INPUTS - 1) + (senseCellIndex * cell_null), valuesVector);
}

void Brain::AddRandomHiddenNeuron()
{
    size_t newNeuronId = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(randInt(SimpleNets::logistic, SimpleNets::perceptron)));

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
            this->AddNeuron(static_cast<SimpleNets::neuronTypes>(randInt(SimpleNets::logistic, SimpleNets::perceptron)));
            // this->AddRandomHiddenNeuron();
        }
    }
    // add/remove/modify connection with 80% probability
    else
    {
        // modify existing connection with 75% probability
        if (randPercent(75) && this->connections().size() > 0)
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
                    switch (randInt(0, 4))
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

                    case 4:
                        couldAdd = !this->TryAddRandomInputOutputConnection();
                        break;
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

    size_t newInternalID = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(randInt(SimpleNets::logistic, SimpleNets::perceptron)));
    // determine the new internal neuron's outputs (input layer vs output layer)
    if (randPercent(50))
    {
        while (!this->TryAddRandomHiddenConnectionBySrc(newInternalID))
            ;
    }
    else
    {
        while (!this->TryAddRandomOutputConnectionBySrc(newInternalID))
            ;
    }

    for (int i = 0; i < cell_null; i++)
    {
        // size_t inputId = this->AddInput();
        this->AddInput();
        /*
        if (randPercent(40))
        {
            if (randPercent(60))
            {
                if (this->AddConnection(inputId, newInternalID, randFloat(-1.0, 1.0)))
                {
                    printf("that shouldn't have happened\n");
                    exit(1);
                }
            }
            else
            {
                if (randPercent(90))
                {
                    if (this->AddConnection(inputId, newInternalID, randFloat(-1.0, 1.0)))
                    {
                        printf("that shouldn't have happened\n");
                        exit(1);
                    }
                }
                else
                {
                    if (this->TryAddRandomInputOutputConnectionBySrc(inputId))
                    {
                        printf("that shouldn't have happened\n");
                        exit(1);
                    }
                }
            }
        }
        */
    }
    return this->nextSensorIndex++;
}

enum Intent Brain::Decide()
{
    // ok to cast to enum because the net chooses between discrete outputs and returns the index of the chosen one
    return static_cast<enum Intent>(this->Output());
}
