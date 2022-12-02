#include <stdio.h>
#include "feedforwardnn.h"
#include "dagnn.h"

void testFeedForwardNet()
{
    SimpleNets::FeedForwardNeuralNet n(2, {}, {1, SimpleNets::perceptron});
    n.Dump();
    bool needMoreTraining = true;
    size_t i = 0;
    while (needMoreTraining)
    {
        printf("Training epoch %lu\n", i++);
        needMoreTraining = false;
        for (int a = 0; a < 2; a++)
        {
            n.SetInput(0, {static_cast<nn_num_t>(a)});
            for (int b = 0; b < 2; b++)
            {
                n.SetInput(1, {static_cast<nn_num_t>(b)});
                bool result = a & b;
                printf("Input %d,%d: output %f - expected %d - %s\n",
                       a, b,
                       n.Output(), result,
                       (static_cast<nn_num_t>(result) == n.Output()) ? "[PASS]" : "[FAIL]");
                needMoreTraining |= static_cast<nn_num_t>(result) != n.Output();
                n.Learn({static_cast<nn_num_t>(result)}, 1.0);
            }
        }
    }
    printf("\n\nTesting:\n");

    for (int a = 0; a < 2; a++)
    {
        for (int b = 0; b < 2; b++)
        {
            n.SetInput({static_cast<nn_num_t>(a), static_cast<nn_num_t>(b)});
            bool result = a & b;

            printf("Input %d,%d: output %f - expected %d - %s\n",
                   a, b,
                   n.Output(), result,
                   (static_cast<nn_num_t>(result) == n.Output()) ? "[PASS]" : "[FAIL]");
        }
    }
    printf("\n\n");
    n.Dump();
}

void testDAGNet()
{
    SimpleNets::DAGNetwork *n = new SimpleNets::DAGNetwork(2, {}, {1, SimpleNets::perceptron});
    n->AddConnection(0, 3, 0.1);
    n->AddConnection(1, 3, 0.1);
    n->AddConnection(2, 3, 0.1);
    bool needMoreTraining = true;
    size_t i = 0;
    while (needMoreTraining)
    {
        printf("Training epoch %lu\n", i++);
        needMoreTraining = false;
        for (int a = 0; a < 2; a++)
        {
            for (int b = 0; b < 2; b++)
            {
                n->SetInput({static_cast<nn_num_t>(a), static_cast<nn_num_t>(b)});
                nn_num_t result = static_cast<nn_num_t>(a & b);
                nn_num_t output = n->Output();
                printf("Input %d,%d: output %lf - expected %lf - %s\n",
                       a, b,
                       output, result,
                       (result == output) ? "[PASS]" : "[FAIL]");
                needMoreTraining |= (result != output);
                n->Learn({result}, 1.0);
            }
        }
        if (needMoreTraining)
        {
            printf("need more training...\n");
        }
    }
    printf("\n\nTesting:\n");

    for (int a = 0; a < 2; a++)
    {
        for (int b = 0; b < 2; b++)
        {
            n->SetInput({static_cast<nn_num_t>(a), static_cast<nn_num_t>(b)});
            nn_num_t result = static_cast<nn_num_t>(a & b);
            nn_num_t output = n->Output();
            printf("Input %d,%d: output %f - expected %f - %s\n",
                   a, b,
                   output, result,
                   (result == output) ? "[PASS]" : "[FAIL]");
        }
    }
    n->Dump();
    printf("\n\nTesting Copy Constructor:\n");
    SimpleNets::DAGNetwork *secondN = new SimpleNets::DAGNetwork(*n);
    delete n;

    for (int a = 0; a < 2; a++)
    {
        for (int b = 0; b < 2; b++)
        {
            secondN->SetInput({static_cast<nn_num_t>(a), static_cast<nn_num_t>(b)});
            nn_num_t result = static_cast<nn_num_t>(a & b);
            nn_num_t output = secondN->Output();
            printf("Input %d,%d: output %f - expected %f - %s\n",
                   a, b,
                   output, result,
                   (result == output) ? "[PASS]" : "[FAIL]");
        }
    }
    secondN->Dump();
    delete secondN;
}

int main()
{
    // testFeedForwardNet();
    testDAGNet();
    return 0;
}