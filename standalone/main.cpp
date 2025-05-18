#include "algorithm.h"
#include "loader.h"
#include <iostream>
#include "solver.h"
#include <QRandomGenerator>
#include <mutex>
#include <future>
#include <optional>
#include "rcpsp_parser.h"
#include "debugoutput.h"

int main(int argc, char** argv)
{
    Parser parser(argc, argv);
    if (parser.input_files.size() == 0) { std::cout << "No input files.\n"; return 1; } 
    if (!parser.debug_output) qInstallMessageHandler(DisabledDebugOutput);
    if (!parser.solver)
    {
        std::cout << "No solver." << "\n";
        for (auto std_input_file : parser.input_files)
        {
            if (parser.input_files.size() != 1)
            {
                std::cout << "Launching for file " << std_input_file;
            }
            QString input_file(std_input_file.c_str());
            Algorithm algorithm;
            std::vector<Job*> all_jobs;
            std::vector<Worker*> all_workers;
            Loader::Load(input_file, algorithm, all_workers, all_jobs);
            Loader::LoadPreferences(input_file, algorithm);
            if (algorithm.get_preference() == Preference::NONE)
            {
                algorithm.set_weights(*parser.weights);
            }
            else
            {
                if (!parser.weights.has_value())
                {
                    std::cout << "Got no weights, creating empty\n";
                    algorithm.set_weights(Weights::create_empty());
                }
            }
            auto max_time = algorithm.run();
            auto completed = algorithm.get_completed();
            Stats stats(completed, 0.1);
            std::cout << "Penalty: " << algorithm.get_penalty() << "\n";
            GLOBAL_LOG(std::string("Penalty") + std::to_string(algorithm.get_penalty()) + std::string("\n"));
            if (parser.debug_output)
            {
                std::cout << " max time: " << max_time << " ";
                std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
                std::cout << algorithm.get_string_result(completed);
                stats.print();
            }
        }
    }
    else
    {
        assert(!parser.weights.has_value());
        auto calculate_value = [](Point point, std::string input_file) -> double
        {
            Algorithm algorithm;
            std::vector<Job*> all_jobs;
            std::vector<Worker*> all_workers;
            Loader::Load(QString::fromStdString(input_file), algorithm, all_workers, all_jobs);
            Loader::LoadPreferences(QString::fromStdString(input_file), algorithm);
            assert(algorithm.get_preference() == Preference::NONE);
            // Check if point is outside of [0, 1].
            //print_point(point);
            //double sum = 0;
            //for (auto dim_value : point)
            //{
            //    //if (dim_value < 1 && dim_value > 0) { /* good */ }
            //    //else { return all_jobs.size() * 5; /* punishment for being too far */ }
            //    // Allow every point, just normalize afterwards
            //    sum += dim_value;
            //}
            std::size_t dim = 0;
            AlgorithmWeights weights;
            for (auto name : Weights::WeightsNames)
            {
                Weights::set(weights, name, point[dim]);
                dim++;
            }
            assert(Weights::are_valid(weights));
            //if (!Weights::are_valid(weights)) weights = Weights::fix(weights);
            algorithm.set_weights(weights);
            algorithm.run();
            //std::cout << " max time: " << algorithm.run() << "\n";
            std::size_t value = algorithm.get_penalty();
            //std::cout << " penalty: " << value << "\n";
            algorithm.reset();
            for (auto worker_group : algorithm.get_worker_groups())
            {
                delete worker_group;
            }
            for (auto worker : all_workers)
            {
                delete worker; // FIXME: weird outer destructor because Algorithm doesn't contain all_workers
            }
            for (auto job : all_jobs)
            {
                delete job;
            }
            return value;
        };
        particle_swarm(-1, 1, calculate_value, parser.input_files, parser.threads, parser.groups);
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
