#include "solver.h"
#include <QRandomGenerator>

AlgorithmWeights generate_random_weights()
{
    std::set<double> delimiters{1.0};
    std::size_t size = Weights::WeightsNames.size();
    for (std::size_t i = 0; i < size - 1; i++)
    {
        delimiters.insert(QRandomGenerator::global()->generateDouble());
    }
    double sum = 0.0;
    AlgorithmWeights output;
    auto weight_it = Weights::WeightsNames.begin();
    auto delimiter_it = delimiters.begin();
    for (std::size_t i = 0; i < size; i++, ++weight_it, ++delimiter_it)
    {
        double value = *delimiter_it - sum;
        output[*weight_it] = value;
        sum = *delimiter_it;
    }
    return output;
};