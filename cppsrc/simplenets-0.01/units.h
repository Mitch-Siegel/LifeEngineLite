#ifndef __UNITS_H_
#define __UNITS_H_

#include <set>

#include "connection.h"

namespace SimpleNets
{

    // enum for the different types of neurons
    enum neuronTypes
    {
        input,
        bias,
        logistic,
        perceptron,
        linear,
    };

    // return the cstr for the name of a given type in the enum
    const char *GetNeuronTypeName(neuronTypes t);

    class Layer;

    /*
     * A unit represents a node in the network
     * connections flow from left to right (lower to higher layer indices)
     */
    class Unit
    {
        // baseline read-only identifying information about the unit
    private:
        size_t id_;
        neuronTypes type_;

    protected:
        // list of all connections starting from a different unit and going to this unit
        std::set<Connection *> inboundConnections_;
        std::set<Connection *> outboundConnections_;

        // internal value of the unit, calculated based on what type it is
        nn_num_t value_ = 0.0;

    public:
        // parameters for learning
        nn_num_t delta = 0.0;
        nn_num_t error = 0.0;

        Unit(size_t id, neuronTypes type);
        virtual ~Unit() = 0;

        // getters for read-only info
        const size_t Id();
        const neuronTypes type();

        // returns the raw value of the unit
        nn_num_t Raw();
        // returns the output of the unit based on its activation function
        virtual nn_num_t Activation() = 0;

        // returns the derivative of the unit's activation function at its current value
        virtual nn_num_t ActivationDeriv() = 0;

        // updates the value of the unit based on the current values of its connections
        virtual void CalculateValue() = 0;

        // alter the weight of a connection - either "from" or "to" must be nullptr to refer to "this"
        void ChangeConnectionWeight(Unit *from, Unit *to, nn_num_t delta);
        void SetConnectionWeight(Unit *from, Unit *to, nn_num_t w);

        // get all outbound connections from this unit
        const std::set<Connection *> &OutboundConnections();

        // get all inbound connections to this unit
        const std::set<Connection *> &InboundConnections();

        // add a connection to this unit's list of references, return true if error
        void AddConnection(Connection *c);

        // remove a connection from this unit's list of references, return true if error
        void RemoveConnection(Connection *c);

    };

    namespace Units
    {
        class Input : public Unit
        {
        public:
            explicit Input(size_t id);
            ~Input(){};

            nn_num_t Activation() override;
            nn_num_t ActivationDeriv() override;
            void CalculateValue() override;
            void SetValue(nn_num_t newValue);
        };

        class Neuron : public Unit
        {
            friend class Layer;
            friend class NeuronLayer;
            friend class OutputLayer;

        public:
            explicit Neuron(size_t id, neuronTypes type);
            ~Neuron();

            void CalculateValue() override;
        };

        class Logistic : public Neuron
        {
        public:
            explicit Logistic(size_t id);
            nn_num_t Activation() override;
            nn_num_t ActivationDeriv() override;
        };

        class Perceptron : public Neuron
        {
        public:
            explicit Perceptron(size_t id);
            nn_num_t Activation() override;
            nn_num_t ActivationDeriv() override;
        };

        class Linear : public Neuron
        {
        public:
            explicit Linear(size_t id);
            nn_num_t Activation() override;
            nn_num_t ActivationDeriv() override;
        };

        class BiasNeuron : public Unit
        {
        public:
            explicit BiasNeuron(size_t id);
            nn_num_t Activation() override;
            nn_num_t ActivationDeriv() override;
            void CalculateValue() override;
        };
    } // namespace Units

} // namespace SimpleNets

#endif
