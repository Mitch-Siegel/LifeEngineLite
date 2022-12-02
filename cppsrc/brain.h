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
private:
    unsigned int nextSensorIndex;

    // make one attempt to add a random connection from the input layer to the hidden layer
    void TryAddRandomInputConnection();

    // make one attempt to add a random connection from one neuron in the hidden layer to another
    void TryAddRandomHiddenConnection();

    // make one attempt to add a random connection from the hidden layer to the output layer
    void TryAddRandomOutputConnection();

public:
    Brain();
    Brain(const Brain &b);
    ~Brain();

    void SetBaselineInput(nn_num_t energyProportion, nn_num_t healthProportion);

    void SetSensoryInput(unsigned int senseCellIndex, nn_num_t values[cell_null]);

    void Mutate();

    unsigned int GetNewSensorIndex();

    enum Intent Decide();
};
