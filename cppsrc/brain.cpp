#include "brain.h"
#include "rng.h"

#include <map>
#include <math.h>

/*
 * Default brain inputs:
 * random
 * energy proportion
 * health proportion
 *
 *
 */
#define BRAIN_DEFAULT_INPUTS 4

nn_num_t RandomBrainConnectionWeight()
{
    return randFloat(-0.1, 0.1) * 10;
}

Brain::Brain() : SimpleNets::DAGNetwork(BRAIN_DEFAULT_INPUTS, {}, {7, SimpleNets::logistic})
{
    this->nextSensorIndex = 0;
    this->freeWill = randFloat(0.0, 1.0);
    this->nTicksSameAction = 0;

    int startingNeurons = randInt(3, 15);
    std::vector<size_t> addedIds;
    for (int i = 0; i < startingNeurons; i++)
    {
        addedIds.push_back(this->AddNeuron(SimpleNets::logistic));
    }

    for (int i = 0; i < startingNeurons; i++)
    {
        // add an input to a random neuron
        if (randPercent(50))
        {
            while (!this->TryAddRandomHiddenConnectionByDst(addedIds[randInt(0, startingNeurons - 1)]))
                ;
        }
        else
        {
            while (!this->TryAddRandomInputConnectionByDst(addedIds[randInt(0, startingNeurons - 1)]))
                ;
        }

        // add an output to a random neuron
        if (randPercent(50))
        {
            while (!this->TryAddRandomHiddenConnectionBySrc(addedIds[randInt(0, startingNeurons - 1)]))
                ;
        }
        else
        {
            while (!this->TryAddRandomOutputConnectionBySrc(addedIds[randInt(0, startingNeurons - 1)]))
                ;
        }
    }

    /*for (int i = 0; i < 1; i++)
    {
        size_t newId = this->AddNeuron(static_cast<SimpleNets::neuronTypes>(SimpleNets::logistic));

        int tries = randInt(1, 2);
        for (int i = 0; i < tries; i++)
        {
            if (randPercent(50))
            {
                while (!this->TryAddRandomHiddenConnectionByDst(newId))
                    ;
            }
            else
            {
                while (!this->TryAddRandomInputConnectionByDst(newId))
                    ;
            }
        }

        if (randPercent(50))
        {
            while (!this->TryAddRandomHiddenConnectionBySrc(newId))
                ;
        }
        else
        {
            while (!this->TryAddRandomOutputConnectionBySrc(newId))
                ;
        }
    }
    */
}

Brain::Brain(const Brain &b) : SimpleNets::DAGNetwork(b)
{
    this->nextSensorIndex = b.nextSensorIndex;
    this->nTicksSameAction = 0;
}

bool Brain::TryAddRandomInputConnectionBySrc(size_t srcId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    return this->AddConnection(srcId,
                               this->layers[1][randInt(0, this->size(1) - 1)].Id(),
                               RandomBrainConnectionWeight());
}

bool Brain::TryAddRandomInputConnectionByDst(size_t dstId)
{
    return this->AddConnection(this->layers[0][randInt(0, this->size(0) - 1)].Id(),
                               dstId,
                               RandomBrainConnectionWeight());
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
                               RandomBrainConnectionWeight());
}

bool Brain::TryAddRandomInputOutputConnectionByDst(size_t dstId)
{
    return this->AddConnection(this->layers[0][randInt(0, this->size(0) - 1)].Id(),
                               dstId,
                               RandomBrainConnectionWeight());
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
                                   RandomBrainConnectionWeight());
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
                                   RandomBrainConnectionWeight());
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
                               RandomBrainConnectionWeight());
}

bool Brain::TryAddRandomOutputConnectionByDst(size_t dstId)
{
    if (this->size(1) == 0)
    {
        return true;
    }
    return this->AddConnection(this->layers[1][randInt(0, this->size(1) - 1)].Id(),
                               dstId,
                               RandomBrainConnectionWeight());
}

bool Brain::TryAddRandomOutputConnection()
{
    return this->TryAddRandomOutputConnectionByDst(this->layers[2][randInt(0, this->size(2) - 1)].Id());
}

void Brain::SetBaselineInput(nn_num_t energyProportion, nn_num_t healthProportion)
{
    this->freeWill += randFloat(-0.1, 0.1);
    if (this->freeWill < 0.0)
    {
        this->freeWill = 0.0;
    }
    else if (this->freeWill > 1.0)
    {
        this->freeWill = 1.0;
    }

    this->SetInput(0, {this->freeWill, (static_cast<nn_num_t>(this->nTicksSameAction) / 20.0f), energyProportion, healthProportion});
}

void Brain::SetSensoryInput(unsigned int senseCellIndex, const std::vector<nn_num_t> &sense)
{
    this->SetInput((BRAIN_DEFAULT_INPUTS - 1) + senseCellIndex, sense);
}

void Brain::AddRandomHiddenNeuron()
{
    size_t newNeuronId = this->AddNeuron(SimpleNets::perceptron);
    this->AddConnection(this->layers[0][0].Id(), newNeuronId, RandomBrainConnectionWeight()); // bias connection

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
            this->AddConnection(this->layers[0][0].Id(), id, RandomBrainConnectionWeight()); // bias connection
            this->AddConnection(this->layers[0][0].Id(), id, RandomBrainConnectionWeight());
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

            this->ChangeWeight(toModify->first.first, toModify->first.second, randFloat(-0.05, 0.05) * 20);
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
    size_t thisInputNeuron = this->AddNeuron(SimpleNets::perceptron);
    if (randPercent(50) && (this->size(1) > 1))
    {
        while (!this->TryAddRandomOutputConnectionBySrc(thisInputNeuron))
            ;
    }
    else
    {
        while (!this->TryAddRandomHiddenConnectionBySrc(thisInputNeuron))
            ;
    }

    for (int i = 0; i < cell_null; i++)
    {
        size_t inputId = this->AddInput();

        if (randPercent(20))
        {
            while (!this->TryAddRandomHiddenConnectionBySrc(inputId))
                ;
        }
    }

    int returnedIndex = this->nextSensorIndex;
    this->nextSensorIndex += cell_null;
    return returnedIndex;
}

enum Intent Brain::Decide()
{
    std::vector<std::pair<int, nn_num_t>> strengthsByOutput;

    for (size_t i = 0; i < this->size(2); i++)
    {
        strengthsByOutput.push_back({i, this->layers[2][i].Activation()});
        // strengthsByOutput[i] = this->layers[2][i].Activation();
    }

    std::sort(strengthsByOutput.begin(), strengthsByOutput.end(),
              [](const std::pair<int, nn_num_t> a, const std::pair<int, nn_num_t> b)
              { return a.second > b.second; });

    // int strictIntent = this->Output();
    enum Intent chosenAction;
    bool notChosen = true;
    while (notChosen)
    {
        for (auto i = strengthsByOutput.begin(); i != strengthsByOutput.end(); ++i)
        {
            // printf("%f\n", i->second);
            if (randFloat(0.0, 1.0) >= (1 - tanh(M_PI * pow(i->second, (1.0 + (0.1 * (this->nTicksSameAction)))))))
            {
                // printf("Strict Intent: %d - Randomized Intent: %d - Same?:%s @ %f\n", strictIntent, i->first, (strictIntent == i->first) ? "YES" : "NO", i->second);
                chosenAction = static_cast<enum Intent>(i->first);
                notChosen = false;
                break;
            }
        }
    }

    if (chosenAction == this->lastAction)
    {
        this->nTicksSameAction++;
    }
    else
    {
        this->nTicksSameAction = 0;
    }
    this->lastAction = chosenAction;
    return chosenAction;
}
