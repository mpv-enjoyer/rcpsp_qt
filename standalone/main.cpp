#include "algorithm.h"
#include "loader.h"
#include "debugoutput.h"
#include <iostream>
#include "solver.h"
#include <QRandomGenerator>
#include <mutex>
#include <future>

static constexpr auto DIMENSIONS = Weights::SIZE;
using Point = std::array<double, DIMENSIONS>;
double cross_in_tray(Point point) // https://al-roomi.org/benchmarks/unconstrained/2-dimensions/44-cross-in-tray-function
{
    double x = point[0];
    double y = point[1];
    return -0.0001 * std::pow(std::abs(std::sin(x) * std::sin(y) * std::exp(std::abs(100 - std::sqrt(x * x + y * y) / M_PI))) + 1, 0.1);
}

double random_double(double min, double max)
{
    return QRandomGenerator::global()->generateDouble() * (max - min) + min;
}


struct BestParticle
{
    Point m_position;
    std::mutex m_mutex;
    double m_value = __DBL_MAX__;
    bool check_update(Point position, double value)
    {
        std::lock_guard g(m_mutex);
        if (m_value > value)
        {
            m_position = position;
            m_value = value;
            return true;
        }
        return false;
    }
    void print(std::size_t i)
    {
        std::cout << i << ") Best particle is f(";
        for (auto position_dimension : m_position)
        {
            std::cout << position_dimension << ", ";
        }
        std::cout << ") = " << m_value << "\n";
    }
};
BestParticle particle_swarm(double min, double max, std::function<double(Point, Algorithm&)> calculate_value, std::vector<Algorithm> algorithm_vector) // https://en.wikipedia.org/wiki/Particle_swarm_optimization
{
    BestParticle best;
    struct Particle
    {
        Point position;
        Point best_known_position;
        Point velocity;
        BestParticle& best;
        double value;
        double best_value;
        std::function<double(Point)> calculate_value;
// for each particle i = 1, ..., S do
//     Initialize the particle's position with a uniformly distributed random vector: xi ~ U(blo, bup)
//     Initialize the particle's best known position to its initial position: pi ← xi
//     if f(pi) < f(g) then
//         update the swarm's best known position: g ← pi
//     Initialize the particle's velocity: vi ~ U(-|bup-blo|, |bup-blo|)
        Particle(double min, double max, BestParticle& best_swarm_position) : best(best_swarm_position), calculate_value(calculate_value)
        {
            double len = max - min;
            for (std::size_t i = 0; i < DIMENSIONS; i++)
            {
                double generated = random_double(min, max);
                position[i] = generated;
                best_known_position[i] = generated;
                velocity[i] = random_double(-len, len);
            }
            
            update_best();
        }
        void update_best()
        {
            value = calculate_value(position);
            if (value < best_value)
            {
                best_known_position = position;
                best_value = value;
            }
            best.check_update(position, value);
        }
        
        void update()
        {
// while a termination criterion is not met do:
//     for each particle i = 1, ..., S do
//         for each dimension d = 1, ..., n do
//             Pick random numbers: rp, rg ~ U(0,1)
//             Update the particle's velocity: vi,d ← w vi,d + φp rp (pi,d-xi,d) + φg rg (gd-xi,d)
//         Update the particle's position: xi ← xi + vi
//         if f(xi) < f(pi) then
//             Update the particle's best known position: pi ← xi
//             if f(pi) < f(g) then
//                 Update the swarm's best known position: g ← pi
            static constexpr double W = 0.5; // Inertia weight < 1
            static constexpr double PHIp = 1.5; // "Typical values are in [1, 3]"
            static constexpr double PHIg = 2.5; // "Typical values are in [1, 3]"
            for (std::size_t dimension = 0; dimension < DIMENSIONS; dimension++)
            {
                double rp = random_double(0, 1);
                double rg = random_double(0, 1);
                velocity[dimension] = W * velocity[dimension] +
                                                PHIp * rp * (best_known_position[dimension] - position[dimension]) +
                                                PHIg * rg * (best.m_position[dimension] - position[dimension]);
                position[dimension] += velocity[dimension];
            }
            update_best();
        }
    };
    const double PARTICLE_COUNT = algorithm_vector.size();
    static constexpr double PARTICLE_MIN = 0;
    static constexpr double PARTICLE_MAX = 1;

    std::vector<Particle> particles(PARTICLE_COUNT, Particle(PARTICLE_MIN, PARTICLE_MAX, best));
    for (auto p : particles)
    {
        best.check_update(p.position, p.value);
    }

    for (int i = 0; i < 3; i++)
    {
        std::vector<std::thread> threads;
        for (auto& particle : particles)
        {
            std::thread t([&](){ particle.update(); });
            threads.emplace_back(t);
            t.detach();
        }
        for (auto& t : threads)
        {
            t.join();
        }
        best.print(i);
    }
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    QString input_file = "";
    bool solver = false;
    for (auto arg : args)
    {
        if      (arg == "-DDO")    { qInstallMessageHandler(DisabledDebugOutput); } // Disable Debug Output
        else if (arg == "-SOLVER") { solver = true; }
        else                       { assert(input_file.size() == 0); input_file = arg.c_str(); }
    }
    std::vector<Algorithm> algorithm_vector;
    //Algorithm algorithm;
    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
    for (auto& algorithm : algorithm_vector)
    {
        Loader::Load(input_file, algorithm, all_workers, all_jobs);
        Loader::LoadPreferences(input_file, algorithm);
    }
    if (!solver)
    {
        assert("Not solver -> not ")
    //    if (algorithm.get_preference() == Preference::NONE) Loader::LoadWeights(input_file, algorithm);
    //    std::cout << " max time: " << algorithm.run() << " ";
    //    std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
    //    return 0;
    }

    while (true)
    {
        auto calculate_value = [](Point point, Algorithm& algorithm) -> double
        {
            // Check if point is outside of [0, 1].
            for (auto dim : point)
            {
                if (dim < 1 && dim > 0) { /* good */ }
                else { return all_jobs.size() * 5; /* punishment for being too far */ }
            }

        };
        algorithm.reset();
        AlgorithmWeights weights = generate_random_weights();
        assert(Weights::are_valid(weights));
        algorithm.set_weights(weights);
        std::cout << Weights::to_string(weights) << "\n";
        std::cout << " max time: " << algorithm.run() << "\n";
        std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";

        //std::cout << Weights::to_string(weights) << "\n";
        //std::cout << " max time: " << algorithm.run() << " ";
        //std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
        //algorithm.reset();
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
