#include "algorithm.h"
#include "loader.h"
#include "debugoutput.h"
#include <iostream>
#include "solver.h"

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    QString input_file = "";
    bool solver = false;
    for (auto arg : args)
    {
        if      (arg == "-DDO")    qInstallMessageHandler(DisabledDebugOutput); // Disable Debug Output
        else if (arg == "-SOLVER") solver = true;
        else                       input_file = arg.c_str();
    }
    Algorithm algorithm;
    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
    Loader::Load(input_file, algorithm, all_workers, all_jobs);
    Loader::LoadPreferences(input_file, algorithm);
    if (!solver)
    {
        if (algorithm.get_preference() == Preference::NONE) Loader::LoadWeights(input_file, algorithm);
        std::cout << " max time: " << algorithm.run() << " ";
        std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
        return 0;
    }

    while (true)
    {
        AlgorithmWeights weights = generate_random_weights();
        assert(Weights::are_valid(weights));
        algorithm.set_weights(weights);
        std::cout << Weights::to_string(weights) << "\n";
        std::cout << " max time: " << algorithm.run() << "\n";
        std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";

        //std::cout << Weights::to_string(weights) << "\n";
        //std::cout << " max time: " << algorithm.run() << " ";
        //std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
        algorithm.reset();
    }
}

/*
Данные, которые можно считывать каждую итерацию фронта для сортировки требований по важности:
- (V) количество последователей / кол-во оставшихся требований
- (V) кол-во последователей / кол-во требований всего
- (?) важность последователей (использовать такую же сортировку, но с ограничениями чтобы избежать лютого O(2^n))
- (V) критическое время требования / максимальное критическое время всех требований
- (Х) пересчитывать критическое время требования во время выполнения, а не только в начале
- (V) время от начала выполнения до текущего момента / время всего на выполнение этого требования
- (V) средняя загруженность во время выполнения
- (V) максимальная загруженность при выполнении
- (?) сколько процентов времени занимает выполнение требования

Данные, которые можно использовать для калибровки, но которые скорее всего будут одинаковые для конкурирующих требований:
- (X) сколько станков доступно(без учета плана) в среднем для всех групп работ ( -\frac{1}{x+1}+1 )
- (X) сколько процентов станков в среднем доступно с учетом плана (-\frac{1}{x+1}+1)
- (X) количество последователей / макс. кол-во возможных последователей для такого количества работ

- (?) количество станков способных выполнить требование и их текущая занятость
- (?) сколько процентов времени осталось до того, как станок будет "нерабочим" по плану
- (?) сколько процентов требований ждут поступления, ждут обработки станками, выполняются и уже выполнены
- (?) статистические характеристики требований: среднее значение, дисперсия, максимальное и минимальное значения
- (?) отношения среднего и максимального времени выполнения требований к среднему и максимальному времени рабочего состояния в графике работы станков
*/
