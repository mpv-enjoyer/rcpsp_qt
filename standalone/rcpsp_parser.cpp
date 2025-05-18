#include "rcpsp_parser.h"
#include "argh.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <QFile>

Parser::Parser(int argc, char **argv)
{
    std::string buffer;
    argh::parser argh_args(argc, argv);
    for (auto arg : argh_args.flags())
    {
        if (arg == "DDO")
        {
            debug_output = false;
        }
        else if (arg == "SOLVER")
        {
            solver = true;
            std::cout << "Solver enabled\n";
        }
        else
        {
            error_out(arg, "unknown flag");
        }
    }

    for (auto arg : argh_args.params())
    {
        if (arg.first == "WEIGHTS")
        {
            if (weights.has_value()) throw std::invalid_argument("got 2 or more sets of arg weights");
            std::stringstream weights_string(arg.second);
            weights = AlgorithmWeights();
            auto weights_iter = Weights::WeightsNames.begin();
            while (weights_string >> buffer)
            {
                weights->insert({*weights_iter, std::atof(buffer.c_str())});
                if (weights_iter++ == Weights::WeightsNames.end())
                {
                    error_out(arg.second, "too many arg weights");
                }
            }
            if (weights_iter != Weights::WeightsNames.end())
            {
                error_out(arg.second, "not enough arg weights");
            }
        }
        else if (arg.first == "THREADS")
        {
            threads = std::atoi(arg.second.c_str());
        }
        else if (arg.first == "GROUPS")
        {
            groups = std::atoi(arg.second.c_str());
        }
        else
        {
            error_out(arg.first, "unknown param");
        }
    }

    std::vector<std::string> file_args = argh_args.pos_args();
    file_args.erase(file_args.begin());
    for (auto arg : file_args)
    {
        input_files.push_back(arg);
        std::cout << "Input file " << input_files.size() << " : "  << arg << "\n";
        if (!QFile::exists(QString(arg.c_str())))
        {
            error_out(arg, "file not exists");
        }
    }
}

void Parser::error_out(std::string arg, const char* reason)
{
    std::cout << "ERROR: Got " << arg << "\n";
    throw std::invalid_argument(reason);
}
