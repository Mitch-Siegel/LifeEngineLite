#include "connection.h"
#include "units.h"

namespace SimpleNets
{
    Connection::Connection(Unit *from, Unit *to, nn_num_t weight)
    {
        this->from = from;
        this->to = to;
        this->weight = weight;
    }

    bool Connection::operator==(const Connection &b)
    {
        return (this->from == b.from) && (this->to = b.to);
    }

    // bool Connection::operator<(const Connection &b) const
    // {
        // return (this->to->Id() + this->from->Id()) < (b.to->Id() + b.from->Id());
    // }

    bool Connection::operator()(const Connection *b)
    {
        return this->operator==(*b);
    }
}