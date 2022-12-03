#ifndef __LAYERS_H_
#define __LAYERS_H_
#include <stdio.h>
#include <vector>

#include "units.h"

namespace SimpleNets
{
    class NeuralNet;

    class Layer
    {
    private:
        size_t index_;
        std::vector<Unit *> units;
        NeuralNet *myNet;

    public:
        Unit &operator[](size_t index);

        Layer(NeuralNet *myNet_, bool addBias);
        ~Layer();

        void AddUnit(Unit *u);

        size_t size();
        size_t Index();

        std::vector<Unit *>::iterator begin();
        std::vector<Unit *>::iterator end();
        std::vector<Unit *>::iterator erase(std::vector<Unit *>::iterator i);
    };
} // namespace SimpleNets
#endif
