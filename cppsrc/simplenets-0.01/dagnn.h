#include "nnet.h"

namespace SimpleNets
{
    // a network with arbitrary structure, the only rule is that it must be acylclic
    class DAGNetwork : public NeuralNet
    {

    private:
        void BackPropagate(const std::vector<nn_num_t> &expectedOutput);
        void UpdateWeights(nn_num_t learningRate);
        void Recalculate();

        std::map<size_t, size_t> postNumbers_; // mapping from POST number to unit ID

        void GeneratePostNumbers();

        // returns true if any cycle found, false if no cycle
        // bool CheckForCycle();

        bool OnConnectionAdded(Connection *c) override;
        bool OnConnectionRemoved(Connection *c) override;

    public:
        DAGNetwork(size_t nInputs,
                   std::vector<std::pair<neuronTypes, size_t>> hiddenNeurons,
                   std::pair<size_t, neuronTypes> outputFormat);
        DAGNetwork(const DAGNetwork &n) = default;
        ~DAGNetwork() = default;

        // void AddLayer(size_t size, enum neuronTypes t);

        void Learn(const std::vector<nn_num_t> &expectedOutput, nn_num_t learningRate);

        nn_num_t Output() override;

        void PrintPOSTNumbers();

        const std::map<size_t, size_t> &PostNumbers();

        size_t AddNeuron(neuronTypes t);

        void RemoveHiddenNeuron(size_t id);

        size_t AddInput();
    };
} // namespace SimpleNets
