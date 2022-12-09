#include "config.h"
#include "dagnn.h"

enum Intent
{
    intent_idle,                    // do nothing
    intent_forward,                 // move forward
    intent_back,                    // move backward
    intent_left,                    // move left
    intent_right,                   // move right
    intent_rotate_clockwise,        // rotate clockwise
    intent_rotate_counterclockwise, // rotate counterclockwise
};

class Brain : private SimpleNets::DAGNetwork
{
    friend class OrganismView;
private:
    unsigned int nextSensorIndex;

    // make one attempt to add a random connection from the input layer to the hidden layer
    bool TryAddRandomInputConnectionBySrc(size_t srcId);
    bool TryAddRandomInputConnectionByDst(size_t dstId);
    bool TryAddRandomInputConnection();

    // make one attempt to add a random connection from the input layer to the output layer
    bool TryAddRandomInputOutputConnectionBySrc(size_t srcId);
    bool TryAddRandomInputOutputConnectionByDst(size_t dstId);
    bool TryAddRandomInputOutputConnection();

    // make one attempt to add a random connection from one neuron in the hidden layer to another
    bool TryAddRandomHiddenConnectionBySrc(size_t srcId);
    bool TryAddRandomHiddenConnectionByDst(size_t dstId);
    bool TryAddRandomHiddenConnection();

    // make one attempt to add a random connection from the hidden layer to the output layer
    bool TryAddRandomOutputConnectionBySrc(size_t srcId);
    bool TryAddRandomOutputConnectionByDst(size_t dstId);
    bool TryAddRandomOutputConnection();

    void AddRandomHiddenNeuron();

public:
    Brain();
    Brain(const Brain &b);
    ~Brain() = default;

    void SetBaselineInput(nn_num_t energyProportion, nn_num_t healthProportion);

    void SetSensoryInput(unsigned int senseCellIndex, nn_num_t values[cell_null]);

    size_t NeuronCount() {return this->units().size();};
    
    size_t SynapseCount() {return this->connections().size();};

    void Mutate();

    unsigned int GetNewSensorIndex();

    enum Intent Decide();
};
