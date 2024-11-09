#include "algorithm.h"
#include "loader.h"
#include "debugoutput.h"

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    QString input_file = "";
    for (auto arg : args)
    {
        if (arg == "-DDO") qInstallMessageHandler(DisabledDebugOutput); // Disable Debug Output
        else input_file = arg.c_str();
    }
    Algorithm algorithm;
    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
    Loader::Load(input_file, algorithm, all_workers, all_jobs);
    Loader::LoadPreferences(input_file, algorithm);
    Loader::LoadWeights(input_file, algorithm);
    algorithm.run();
}