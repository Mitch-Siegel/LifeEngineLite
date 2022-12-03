#include <math.h>
#include <stdio.h>
#include <algorithm>

#include "nnet.h"
namespace SimpleNets
{
    // Neural Net base
    NeuralNet::NeuralNet()
    {
    }

    NeuralNet::NeuralNet(const NeuralNet &n)
    {

        for (auto u : n.units_)
        {
            this->GenerateUnitFromType(u.second->type(), u.first);
        }

        for (Layer l : n.layers)
        {
            Layer newL = Layer(this, false);
            for (auto u = l.begin(); u != l.end(); ++u)
            {
                newL.AddUnit(this->units_[(*u)->Id()]);
            }
            this->layers.push_back(newL);
        }

        for (auto c : n.connections_)
        {
            Unit *from = this->units_[c.first.first];
            Unit *to = this->units_[c.first.second];
            Connection *newC = new Connection(from, to, c.second->weight);
            this->connections_[{from->Id(), to->Id()}] = newC;
            from->AddConnection(newC);
            to->AddConnection(newC);
        }
    }

    NeuralNet::~NeuralNet()
    {
        for (auto c : this->connections_)
        {
            delete c.second;
        }

        for (auto u : this->units_)
        {
            delete u.second;
        }
    }

    size_t NeuralNet::AcquireNewUnitID()
    {
        if (this->units_.size() == 0 || (this->units_.size() == (this->units_.rbegin()->second->Id() + 1)))
        {
            return this->units_.size();
        }
        else
        {
            size_t s = this->units_.size();
            for (size_t i = 0; i < s; i++)
            {
                if (this->units_.count(i) == 0)
                {
                    return i;
                }
            }
            printf("Error - expecteed gap in units map but couldn't find one!\n");
            exit(1);
        }
    }

    Unit *NeuralNet::GenerateUnitFromType(neuronTypes t, size_t id)
    {
        if (this->units_.count(id))
        {
            printf("Request to add unit with id %lu to net already containing that ID!\n", id);
            exit(1);
        }
        Unit *u = nullptr;
        switch (t)
        {
        case input:
            u = new Units::Input(id);
            break;

        case bias:
            u = new Units::BiasNeuron(id);
            break;

        case logistic:
            u = new Units::Logistic(id);
            break;

        case perceptron:
            u = new Units::Perceptron(id);
            break;

        case linear:
            u = new Units::Linear(id);
            break;
        }
        this->units_[id] = u;
        return u;
    }

    Unit *NeuralNet::GenerateUnitFromType(neuronTypes t)
    {
        return this->GenerateUnitFromType(t, this->AcquireNewUnitID());
    }

    const std::map<size_t, Unit *> &NeuralNet::units()
    {
        return this->units_;
    }

    const std::map<std::pair<size_t, size_t>, Connection *> &NeuralNet::connections()
    {
        return this->connections_;
    }

    Layer &NeuralNet::operator[](int index)
    {
        return this->layers[index];
    }

    size_t NeuralNet::size()
    {
        return this->layers.size();
    }

    size_t NeuralNet::size(int index)
    {
        return (*this)[index].size();
    }

    void NeuralNet::Dump()
    {
        printf("Neural Net with %lu layers\n", this->layers.size());
        for (size_t i = 0; i < this->layers.size(); i++)
        {
            Layer &l = (*this)[i];
            printf("Layer %lu (index %lu) - %lu units\n", i, l.Index(), l.size());
            for (size_t j = 0; j < l.size(); j++)
            {
                Unit &u = l[j];
                printf("Neuron %2lu (type %10s): raw: %f, delta %f, error %f\n\tactivation: %f\n\tweights:", u.Id(), GetNeuronTypeName(u.type()), u.Raw(), u.delta, u.error, u.Activation());
                for (auto connection : u.InboundConnections())
                {
                    printf("%lu->this % 0.07f, ", connection->from->Id(), connection->weight);
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("Output value: % f\n", this->Output());
    }

    void NeuralNet::SetInput(const std::vector<nn_num_t> &values)
    {
        // - 1 to account for bias neuron
        if (values.size() != this->layers[0].size() - 1)
        {
            printf("Error setting input for neural network!\n"
                   "Expected %lu input values, received vector of size %lu\n",
                   this->layers[0].size(), values.size());
        }
        for (size_t i = 0; i < values.size(); i++)
        {
            // offset by 1 to skip over bias neuron at index 0
            Units::Input *input = static_cast<Units::Input *>(*(this->layers[0].begin() + 1 + i));
            input->SetValue(values[i]);
        }
    }

    void NeuralNet::SetInput(size_t index, const std::vector<nn_num_t> &values)
    {
        // - 1 to account for bias neuron
        if (values.size() + index > this->layers[0].size() - 1)
        {
            printf("Error setting input for neural network!\n"
                   "Expected %lu input values, received vector of size %lu\n",
                   this->layers[0].size() - 1, values.size());
        }
        for (size_t i = 0; i < values.size(); i++)
        {
            // offset by 1 to skip over bias neuron at index 0
            Units::Input *input = static_cast<Units::Input *>(*(this->layers[0].begin() + 1 + i + index));
            input->SetValue(values[i]);
        }
    }

    // return false if no error, true if valid request but error, or bail the program if from and/or to are nonexistent
    // error condition determined by specific net implementations in the OnConnectionAdded function
    bool NeuralNet::AddConnection(Unit *from, Unit *to, nn_num_t w)
    {
        Connection *c = new Connection(from, to, w);
        this->connections_[{from->Id(), to->Id()}] = c;
        from->AddConnection(c);
        to->AddConnection(c);
        return this->OnConnectionAdded(c);
    }

    // return false if no error, true if valid request but error, or bail the program if from and/or to are nonexistent
    // error condition determined by specific net implementations in the OnConnectionAdded function
    bool NeuralNet::AddConnection(size_t fromId, size_t toId, nn_num_t w)
    {
        if (this->units_.count(fromId) == 0)
        {
            printf("Error generating connection from %lu->%lu - source unit doesn't exist!\n", fromId, toId);
            exit(1);
        }

        if (this->units_.count(toId) == 0)
        {
            printf("Error generating connection from %lu->%lu - destination unit doesn't exist!\n", fromId, toId);
            exit(1);
        }
        return this->AddConnection(this->units_[fromId], this->units_[toId], w);
    }

    // return false if no error, true if valid request but error, or bail the program if connection doesn't exist
    // error condition determined by specific net implementations in the OnConnectionRemoved function
    bool NeuralNet::RemoveConnection(Unit *from, Unit *to)
    {
        if (this->connections_.count({from->Id(), to->Id()}) == 0)
        {
            printf("Error removing connection from %lu->%lu - no such connection!\n", from->Id(), to->Id());
            exit(1);
        }
        Connection *c = this->connections_[{from->Id(), to->Id()}];
        this->connections_.erase({from->Id(), to->Id()});
        from->RemoveConnection(c);
        to->RemoveConnection(c);
        bool retval = this->OnConnectionRemoved(c);
        if (!retval)
        {
            delete c;
        }
        return retval;
    }

    // return false if no error, true if valid request but error, or bail the program if from and/or to are nonexistent
    // error condition determined by specific net implementations in the OnConnectionRemoved function
    bool NeuralNet::RemoveConnection(size_t fromId, size_t toId)
    {
        if (this->units_.count(fromId) == 0)
        {
            printf("Error removing connection from %lu->%lu - source unit doesn't exist!\n", fromId, toId);
            exit(1);
        }

        if (this->units_.count(toId) == 0)
        {
            printf("Error removing connection from %lu->%lu - destination unit doesn't exist!\n", fromId, toId);
            exit(1);
        }
        return this->RemoveConnection(this->units_[fromId], this->units_[toId]);
    }

    void NeuralNet::RemoveConnection(Connection *c)
    {
        this->connections_.erase({c->from->Id(), c->to->Id()});
        c->from->RemoveConnection(c);
        c->to->RemoveConnection(c);
        delete c;
    }

    void NeuralNet::RemoveUnit(size_t id)
    {
        Unit *u = this->units_[id];
        for (Layer &l : this->layers)
        {
            std::vector<Unit *>::iterator toErase;
            if ((toErase = std::find(l.begin(), l.end(), u)) != l.end())
            {
                l.erase(toErase);
                break;
            }
        }
        auto outbound = u->OutboundConnections();
        for (auto oc : outbound)
        {
            this->RemoveConnection(oc);
        }

        auto inbound = u->InboundConnections();
        for (auto ic : inbound)
        {
            this->RemoveConnection(ic);
        }
        this->units_.erase(id);
        delete u;
    }

    const nn_num_t NeuralNet::GetWeight(size_t fromId, size_t toId)
    {
        if (this->connections_.count({fromId, toId}) == 0)
        {
            printf("Error getting connection weight from %lu->%lu - connection doesn't exist!\n", fromId, toId);
            exit(1);
        }
        // Unit *from = this->units_[fromId];
        // return from->GetConnections().find(Connection(toId))->weight;
        return this->connections_[{fromId, toId}]->weight;
    }

    void NeuralNet::ChangeWeight(size_t fromId, size_t toId, nn_num_t delta)
    {
        if (this->connections_.count({fromId, toId}) == 0)
        {
            printf("Error changing connection weight from %lu->%lu - connection doesn't exist!\n", fromId, toId);
            exit(1);
        }
        this->connections_[{fromId, toId}]->weight += delta;
    }

    /*
    void NeuralNet::ChangeWeight(std::pair<size_t, size_t> from, std::pair<size_t, size_t> to, nn_num_t delta)
    {
        if ((from.first + 1 > this->size()) ||
            (from.second + 1 > this->size(from.first)) ||
            (from.first + 1 != to.first) ||
            (to.first + 1 > this->size()) ||
            (to.second + 1 > this->size(to.first)))
        {
            printf("Invalid request to get weight from layer %lu:%lu to layer %lu:%lu\n",
                   from.first, from.second, to.first, to.second);
            exit(1);
        }
        Layer &tl = *(*this)[to.first];
        tl[to.second].ChangeConnectionWeight(from.second, delta);
        // return tl[to.second].GetConnectionWeights()[from.second];
    }

    void NeuralNet::SetWeight(std::pair<size_t, size_t> from, std::pair<size_t, size_t> to, nn_num_t w)
    {
        if ((from.first + 1 > this->size()) ||
            (from.second + 1 > this->size(from.first)) ||
            (from.first + 1 != to.first) ||
            (to.first + 1 > this->size()) ||
            (to.second + 1 > this->size(to.first)))
        {
            printf("Invalid request to get weight from layer %lu:%lu to layer %lu:%lu\n",
                   from.first, from.second, to.first, to.second);
            exit(1);
        }
        Layer &tl = *(*this)[to.first];
        tl[to.second].SetConnectionWeight(from.second, w);
        // return tl[to.second].GetConnectionWeights()[from.second];
    }

    void NeuralNet::AddNeuron(size_t layer, neuronTypes t)
    {
        if (layer == 0)
        {
            printf("Use AddInput() to add input (neuron on layer 0)\n");
            exit(1);
        }
        else if (layer == this->size() - 1)
        {
            printf("Can't add output to existing network\n");
            exit(1);
        }
        this->layers[layer]->AddUnit(GenerateUnitFromType(t, this->layers[layer - 1]));
    };
    */
} // namespace SimpleNets
