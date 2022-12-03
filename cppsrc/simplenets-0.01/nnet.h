#ifndef __NNET_H_
#define __NNET_H_

#include <math.h>
#include <vector>
#include <map>

#include "layers.h"

namespace SimpleNets
{
    class Unit;
    class Layer;

    class NeuralNet
    {
        friend class Layer;

    private:
        std::map<size_t, Unit *> units_;
        std::map<std::pair<size_t, size_t>, Connection *> connections_;

    protected:
        // generate a unique ID for a new unit
        size_t AcquireNewUnitID();

        // generate a unit from the given type, ensuring it is handled correctly in the map of units
        // this is the only correct way to make new units for the net
        Unit *GenerateUnitFromType(neuronTypes t, size_t unitID);
        Unit *GenerateUnitFromType(neuronTypes t);

        const std::map<size_t, Unit *> &units();
        const std::map<std::pair<size_t, size_t>, Connection *> &connections();
        std::vector<Layer> layers;

        virtual bool OnConnectionAdded(Connection *c) = 0;
        virtual bool OnConnectionRemoved(Connection *c) = 0;

        // unchecked removeconnection method for internal use
        void RemoveConnection(Connection *c);

    public:
        NeuralNet();
        NeuralNet(const NeuralNet &n);
        virtual ~NeuralNet();

        // return the pointer to a given layer by index
        Layer &operator[](int index);

        // return the number of layers in the network
        size_t size();

        // return the size of a given layer by index
        size_t size(int index);

        // return the output of the network given the current input state
        // to be implemented by specific net types
        virtual nn_num_t Output() = 0;

        // print the network layers, units, and connections
        void Dump();

        // sets all input neuron values by index to the values provided
        void SetInput(const std::vector<nn_num_t> &values);

        // sets a subset of the inputs starting at the provided index, with the number of values provided going to subsequent indices
        void SetInput(size_t index, const std::vector<nn_num_t> &values);

        // add a connection from a given ID to a given ID with the given weight
        bool AddConnection(Unit *from, Unit *to, nn_num_t w);

        // add a connection from a given ID to a given ID with the given weight
        bool AddConnection(size_t fromId, size_t toId, nn_num_t w);

        // remove the connection from a given ID to a given ID
        bool RemoveConnection(Unit *from, Unit *to);

        // remove the connection from a given ID to a given ID
        bool RemoveConnection(size_t fromId, size_t toId);

        // remove the unit with the given ID (and all its connections)
        void RemoveUnit(size_t id);

        // get the weight of a connection from a given ID to a given ID
        const nn_num_t GetWeight(size_t fromId, size_t toId);

        // change the weight of a connection from a given ID to a given ID by a delta
        void ChangeWeight(size_t fromId, size_t toId, nn_num_t delta);

        // set the weight of a connection from a given ID to a given ID to a value
        void SetWeight(size_t fromId, size_t toId, nn_num_t w);
    };
} // namespace SimpleNets

#endif
