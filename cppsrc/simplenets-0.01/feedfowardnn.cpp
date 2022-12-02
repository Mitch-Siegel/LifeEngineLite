#include "feedforwardnn.h"
namespace SimpleNets
{
    FeedForwardNeuralNet::FeedForwardNeuralNet(size_t nInputs, std::vector<std::pair<size_t, neuronTypes>> hiddenLayers, std::pair<size_t, neuronTypes> outputFormat)
    {
        this->finalized = false;
        this->layers.push_back(Layer(this, true));
        size_t i;
        for (i = 0; i < nInputs; i++)
        {
            this->layers.back().AddUnit(this->GenerateUnitFromType(input));
        }

        for (i = 0; i < hiddenLayers.size(); i++)
        {
            Layer l = Layer(this, false);
            for (size_t j = 0; j < hiddenLayers[i].first; j++)
            {
                Unit *u = this->GenerateUnitFromType(hiddenLayers[i].second);
                for (auto k = this->layers.back().begin(); k != this->layers.back().end(); ++k)
                {
                    this->AddConnection(*k, u, 0.1);
                }
                l.AddUnit(u);
            }
            this->layers.push_back(l);
        }

        Layer ol = Layer(this, false);
        for (size_t j = 0; j < outputFormat.first; j++)
        {
            Unit *u = this->GenerateUnitFromType(outputFormat.second);
            for (auto k = this->layers.back().begin(); k != this->layers.back().end(); ++k)
            {
                this->AddConnection(*k, u, 0.1);
            }
            ol.AddUnit(u);
        }
        this->layers.push_back(ol);
        this->finalized = true;
    }

    void FeedForwardNeuralNet::Learn(const std::vector<nn_num_t> &expectedOutput, nn_num_t learningRate)
    {
        if (expectedOutput.size() != this->layers.back().size())
        {
            printf("Provided expected output array of length %lu, expected size %lu\n",
                   expectedOutput.size(), this->layers.back().size());
            exit(1);
        }
        this->BackPropagate(expectedOutput);
        this->UpdateWeights(learningRate);
    }

    nn_num_t FeedForwardNeuralNet::Output()
    {
        this->ForwardPropagate();
        switch (this->layers.back().size())
        {
        case 1:
            return (this->layers.back()[0].Activation());
            break;

        default:
            int maxIndex = 0;
            nn_num_t maxValue = -1.0 * MAXFLOAT;
            Layer &ol = this->layers.back();
            for (size_t i = 0; i < ol.size(); i++)
            {
                nn_num_t thisOutput = ol[i].Activation();
                if (thisOutput > maxValue)
                {
                    maxValue = thisOutput;
                    maxIndex = i;
                }
            }
            return (nn_num_t)maxIndex;
        }
        return -999.999;
    }

    void FeedForwardNeuralNet::BackPropagate(const std::vector<nn_num_t> &expectedOutput)
    {
        // delta of each output j = activation derivative(j) * (expected(j) - actual(j))
        for (auto u : this->units())
        {
            u.second->delta = 0.0;
        }
        Layer &ol = this->layers.back();
        for (size_t j = 0; j < ol.size(); j++)
        {
            ol[j].delta = (expectedOutput[j] - ol[j].Activation());
        }

        // for all other layers, delta of a node i in the layer is:
        // activation derivative(i) * sum for all j(weight of connection from i to j * delta(j))
        for (size_t li = this->layers.size() - 1; li > 0; --li)
        {
            Layer &l = this->layers[li];
            for (auto u = l.begin(); u != l.end(); ++u)
            {
                Unit *j = *u;
                for (auto c : j->InboundConnections())
                {
                    Unit *i = c->from;
                    i->delta += (c->weight * j->delta);
                }
            }
        }
        for (auto ui : this->units())
        {
            Unit *u = ui.second;
            u->delta *= u->ActivationDeriv();
        }
    }

    void FeedForwardNeuralNet::UpdateWeights(nn_num_t learningRate)
    {
        for (size_t li = this->size() - 1; li > 0; li--)
        {
            Layer &l = (*this)[li];
            for (auto u = l.begin(); u != l.end(); ++u)
            {
                Unit *j = *u;
                for (auto c : j->InboundConnections())
                {
                    Unit *i = c->from;
                    j->ChangeConnectionWeight(i, nullptr, learningRate * i->Activation() * j->delta);
                }
            }
        }
    }

    void FeedForwardNeuralNet::ForwardPropagate()
    {
        for (size_t i = 1; i < this->size(); i++)
        {
            Layer &l = (*this)[i];
            for (auto u = l.begin(); u != l.end(); ++u)
            {
                (*u)->CalculateValue();
            }
        }
    }

    bool FeedForwardNeuralNet::OnConnectionAdded(Connection *c)
    {
        if (this->finalized)
        {
            printf("Adding connections to existing FeedForwardNeuralNet not allowed!\n");
            exit(1);
        }
        return false;
    }

    bool FeedForwardNeuralNet::OnConnectionRemoved(Connection *c)
    {
        if (this->finalized)
        {
            printf("Removing connections from existing FeedForwardNeuralNet not allowed!\n");
            exit(1);
        }
        return false;
    }

} // namespace SimpleNets
