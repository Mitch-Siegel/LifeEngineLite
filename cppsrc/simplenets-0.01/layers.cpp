#include "layers.h"
#include "nnet.h"

namespace SimpleNets
{
    Unit &Layer::operator[](size_t index)
    {
        return *this->units.at(index);
    };

    Layer::Layer(NeuralNet *myNet_, bool addBias)
    {
        this->myNet = myNet_;
        this->index_ = myNet_->size();

        if (addBias)
        {
            this->AddUnit(myNet_->GenerateUnitFromType(bias));
        }
    }

    Layer::~Layer()
    {
    }

    void Layer::AddUnit(Unit *u)
    {
        this->units.push_back(u);
    }

    size_t Layer::size()
    {
        return this->units.size();
    }

    size_t Layer::Index()
    {
        return this->index_;
    }

    std::vector<Unit *>::iterator Layer::begin()
    {
        return this->units.begin();
    }

    std::vector<Unit *>::iterator Layer::end()
    {
        return this->units.end();
    }
} // namespace SimpleNets
