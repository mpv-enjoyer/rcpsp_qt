#pragma once

#include "algorithm.h"
#include <optional>

struct Parser
{
    bool solver = false;
    bool debug_output = true;
    int threads = 1;
    int groups = 1;
    std::vector<std::string> input_files;
    std::optional<AlgorithmWeights> weights;
    Parser(int argc, char** argv);
    Parser() = delete;
    Parser(const Parser& p) = delete;
    void error_out(std::string arg, const char* reason);
};