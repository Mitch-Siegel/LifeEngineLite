#ifndef __CONNECTION_H_
#define __CONNECTION_H_
#include <math.h>

typedef float nn_num_t;

namespace SimpleNets
{

    class Unit;

    class Connection
    {
    public:
        Unit *from;
        Unit *to;

        nn_num_t weight;

        Connection(Unit *from, Unit *to, nn_num_t weight);
        // explicit Connection(Unit *u);
        // explicit Connection(size_t id);

        bool operator==(const Connection &b);

        // bool operator<(const Connection &b) const;

        bool operator()(const Connection *b);
    };

} // namespace SimpleNets

#endif
