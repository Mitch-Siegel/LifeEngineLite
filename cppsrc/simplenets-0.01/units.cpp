#include <algorithm>

#include "units.h"
#include "layers.h"

namespace SimpleNets
{
    // Unit

    const char *GetNeuronTypeName(neuronTypes t)
    {
        switch (t)
        {
        case input:
            return "Input";
            break;

        case bias:
            return "Bias";
            break;

        case logistic:
            return "Logistic";
            break;

        case perceptron:
            return "Perceptron";
            break;

        case linear:
            return "Linear";
            break;

        default:
            printf("Unexpected neuron type!\n");
            exit(1);
        }

        return nullptr;
    }

    Unit::Unit(size_t id, neuronTypes type)
    {
        this->id_ = id;
        this->type_ = type;
    }

    Unit::~Unit()
    {
    }

    const size_t Unit::Id()
    {
        return this->id_;
    }

    const neuronTypes Unit::type()
    {
        return this->type_;
    }

    nn_num_t Unit::Raw()
    {
        return this->value_;
    }

    void Unit::ChangeConnectionWeight(Unit *from, Unit *to, nn_num_t delta)
    {
        std::set<Connection *>::iterator c;
        if (from == nullptr)
        {
            from = this;
            c = std::find_if(this->outboundConnections_.begin(), this->outboundConnections_.end(), Connection(from, to, 0.0));
        }
        else if (to == nullptr)
        {
            to = this;
            c = std::find_if(this->inboundConnections_.begin(), this->inboundConnections_.end(), Connection(from, to, 0.0));
        }
        else
        {
            printf("Erroneous call to Unit::ChangeCOnnectionWeight - either 'to' or 'from' must be nullptr to refer to this unit!\n");
            exit(1);
        }

        (*c)->weight += delta;
    };

    void Unit::SetConnectionWeight(Unit *from, Unit *to, nn_num_t w)
    {
        std::set<Connection *>::iterator c;
        if (from == nullptr)
        {
            from = this;
            c = std::find_if(outboundConnections_.begin(), outboundConnections_.end(), Connection(from, to, 0.0));
        }
        else if (to == nullptr)
        {
            to = this;
            c = std::find_if(inboundConnections_.begin(), inboundConnections_.end(), Connection(from, to, 0.0));
        }
        else
        {
            printf("Erroneous call to Unit::SetConnectionWeight - either 'to' or 'from' must be nullptr to refer to this unit!\n");
            exit(1);
        }

        (*c)->weight = w;
    };

    const std::set<Connection *> &Unit::OutboundConnections()
    {
        return this->outboundConnections_;
    };

    const std::set<Connection *> &Unit::InboundConnections()
    {
        return this->inboundConnections_;
    };

    void Unit::AddConnection(Connection *c)
    {
        if (c->from == this)
        {
            this->outboundConnections_.insert(c);
        }
        else
        {
            this->inboundConnections_.insert(c);
        }
    };

    void Unit::RemoveConnection(Connection *c)
    {
        if (c->from == this)
        {
            this->outboundConnections_.erase(c);
        }
        else
        {
            this->inboundConnections_.erase(c);
        }
    };

    namespace Units
    {
        // Neuron

        Neuron::Neuron(size_t id, neuronTypes type) : Unit(id, type)
        {
        }

        Neuron::~Neuron()
        {
        }

        void Neuron::CalculateValue()
        {
            this->value_ = 0.0;
            for (auto c : this->inboundConnections_)
            {
                this->value_ += c->weight * c->from->Activation();
            }
        }

        // Input
        Input::Input(size_t id) : Unit(id, input)
        {
            this->value_ = 0.0;
        }

        nn_num_t Input::Activation()
        {
            return this->value_;
        }

        nn_num_t Input::ActivationDeriv()
        {
            return 0.0;
        }

        void Input::CalculateValue()
        {
        }

        void Input::SetValue(nn_num_t newValue)
        {
            this->value_ = newValue;
        }

        // Logistic
        Logistic::Logistic(size_t id) : Neuron(id, logistic)
        {
        }

        nn_num_t Logistic::Activation()
        {
            return 1.0 / (1.0 + exp(-1.0 * this->value_));
        }

        nn_num_t Logistic::ActivationDeriv()
        {

            nn_num_t a = this->Activation();
            return a * (1.0 - a);
        }

        // Perceptron
        Perceptron::Perceptron(size_t id) : Neuron(id, perceptron)
        {
        }

        nn_num_t Perceptron::Activation()
        {
            return (this->value_ > 0) ? 1.0 : 0.0;
        }

        nn_num_t Perceptron::ActivationDeriv()
        {
            return 1.0001 - pow(abs(this->value_), 0.1);
        }

        // Linear

        Linear::Linear(size_t id) : Neuron(id, linear)
        {
        }

        nn_num_t Linear::Activation()
        {
            return this->value_;
        }

        nn_num_t Linear::ActivationDeriv()
        {
            return 1.0;
        }

        // bias
        BiasNeuron::BiasNeuron(size_t id) : Unit(id, bias)
        {
            this->value_ = 1.0;
        }

        nn_num_t BiasNeuron::Activation()
        {
            return this->value_;
        }

        nn_num_t BiasNeuron::ActivationDeriv()
        {
            return 0.0;
        }

        void BiasNeuron::CalculateValue()
        {
        }
    } // namespace Units

} // namespace SimpleNets
