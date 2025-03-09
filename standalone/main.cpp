#include "algorithm.h"
#include "loader.h"
#include "debugoutput.h"
#include <iostream>
#include "solver.h"
#include <QRandomGenerator>
#include <mutex>
#include <future>
#include <optional>

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

void print_point(Point point)
{
    std::cout << "Point (";
    for (auto position_dimension : point)
    {
        std::cout << position_dimension << ", ";
    }
    std::cout << ")\n";
}

struct BestParticle
{
    Point m_position;
    std::mutex m_mutex;
    std::mutex m_mutex_position;
    std::mutex m_mutex_value;
    double m_value = __DBL_MAX__;
    Point get_position()
    {
        //std::lock_guard g(m_mutex_position);
        return m_position;
    };
    double get_value()
    {
        //std::lock_guard g(m_mutex_value);
        return m_value;
    }
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
        std::cout << "\n";
        std::cout << i << ") Best particle is f(";
        for (auto position_dimension : m_position)
        {
            std::cout << position_dimension << ", ";
        }
        std::cout << ") = " << m_value << "\n";
    }
};

std::pair<Point, double> particle_swarm(double min, double max, std::function<double(Point, std::string)> calculate_value, std::vector<std::string>& input_files)
{
    // https://en.wikipedia.org/wiki/Particle_swarm_optimization
    BestParticle best;
    struct Particle
    {
        BestParticle& best;
        std::function<double(Point, std::string)> calculate_value;
        std::vector<std::string> input_files;
        Point position;
        Point best_known_position;
        Point velocity;
        double value;
        double best_value;
// for each particle i = 1, ..., S do
//     Initialize the particle's position with a uniformly distributed random vector: xi ~ U(blo, bup)
//     Initialize the particle's best known position to its initial position: pi ← xi
//     if f(pi) < f(g) then
//         update the swarm's best known position: g ← pi
//     Initialize the particle's velocity: vi ~ U(-|bup-blo|, |bup-blo|)
        Particle(double min, double max, BestParticle& best_swarm_position, std::vector<std::string> input_files, std::function<double(Point, std::string)> calculate_value)
        : best(best_swarm_position), calculate_value(calculate_value), input_files(input_files)
        {
            double len = max - min;
            for (std::size_t i = 0; i < DIMENSIONS; i++)
            {
                double generated = random_double(min, max);
                position[i] = generated;
                best_known_position[i] = generated;
                velocity[i] = random_double(-len, len);
            }
            position = normalize_point(position);
            update_best();
        }
        void update_best()
        {
            double value = 0;
            for (auto input_file : input_files)
            {
                //std::cout << ".";
                value += calculate_value(position, input_file);
                std::cout << ".";
                std::cout.flush();
            }
            if (value < best_value)
            {
                best_known_position = position;
                best_value = value;
            }
            std::cout << "Penalty sum (" << value << ") for ";
            print_point(position);
            best.check_update(position, value);
        }
        
        Point normalize_point(Point point)
        {
            double sum = 0;
            for (auto dim : point) sum += std::abs(dim);
            for (auto& dim : point)
            {
                dim /= sum;
            }
            sum = 0; // further fix for double inaccuracy
            for (auto dim : point) sum += std::abs(dim);
            double diff = sum - 1;
            if (diff == 0) return point;
            if (diff < 0)
            {
                *point.begin() -= diff;
                return point;
            }
            for (auto& dim : point)
            {
                if (dim >= -1 + diff)
                {
                    dim -= diff;
                    return point;
                }
            }
            assert(false);
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
            Point best_position = best.get_position();
            for (std::size_t dimension = 0; dimension < DIMENSIONS; dimension++)
            {
                double rp = random_double(0, 1);
                double rg = random_double(0, 1);
                velocity[dimension] = W * velocity[dimension] +
                                                PHIp * rp * (best_known_position[dimension] - position[dimension]) +
                                                PHIg * rg * (best_position[dimension] - position[dimension]);
                position[dimension] += velocity[dimension];
            }
            position = normalize_point(position);
            update_best();
        }
    };
    const int PARTICLE_COUNT = 12;//algorithm_vector.size();
    const double PARTICLE_MIN = min;
    const double PARTICLE_MAX = max;

    //std::vector<Particle> particles(PARTICLE_COUNT, Particle(PARTICLE_MIN, PARTICLE_MAX, best));
    //std::mutex mutex_for_particles;
    std::vector<Particle> particles;
    std::vector<std::future<Particle>> particle_futures;

    for (int particle_common_id = 0; particle_common_id < PARTICLE_COUNT; particle_common_id++)
    {
        particle_futures.emplace_back(std::async(std::launch::async, [&]() -> Particle
        {
            return Particle(PARTICLE_MIN, PARTICLE_MAX, best, input_files, calculate_value);
        }));
    }
    for (auto& particle : particle_futures)
    {
        particle.wait();
        particles.push_back(particle.get());
    }
    for (int i = 0; i < 50; i++)
    {
        std::vector<std::unique_ptr<std::thread>> threads;
        for (auto& particle : particles)
        {
            threads.push_back(std::make_unique<std::thread>([&](){ particle.update(); }));
        }
        for (auto& t : threads)
        {
            if (t->joinable()) t->join();
        }
        best.print(i);
    }
    return {best.m_position, best.m_value};
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    QString input_file = "";
    bool solver = false;
    bool file_received = false;
    args.erase(args.begin());
    std::vector<std::string> input_files;
    std::optional<AlgorithmWeights> weights;
    for (std::size_t i = 0; i < args.size(); i++)
    {
        auto arg = args[i];
        if      (arg == "-DDO")     { qInstallMessageHandler(DisabledDebugOutput); } // Disable Debug Output
        else if (arg == "-SOLVER")  { solver = true; }
        else if (arg == "-WEIGHTS") { weights = AlgorithmWeights(); for (auto name : Weights::WeightsNames) { i++; double value = std::atof(args[i].c_str()); weights->insert({name, value}); } }
        else                        { input_files.push_back(arg); std::cout << "Input file " << input_files.size() << " " << arg << "\n"; }
    }
    input_file = QString::fromStdString(input_files.front());
    if (!solver)
    {
        std::cout << "No solver." << "\n";
        Algorithm algorithm;
        std::vector<Job*> all_jobs;
        std::vector<Worker*> all_workers;
        Loader::Load(input_file, algorithm, all_workers, all_jobs);
        Loader::LoadPreferences(input_file, algorithm);
        if (algorithm.get_preference() == Preference::NONE)
        {
            algorithm.set_weights(*weights);
        }
        else
        {
            assert(!weights.has_value());
        }
        std::cout << " max time: " << algorithm.run() << " ";
        std::cout << " failed jobs: " << algorithm.get_failed_jobs_count() << "\n";
        auto completed = algorithm.get_completed();
        Stats stats(completed, 0.1);
        std::cout << algorithm.get_string_result(completed);
        std::cout << "Penalty: " << algorithm.get_penalty();
        //std::cout << "STATS (wait_coeff): \n";
        //for (auto point : stats.wait_coeff)
        //{
        //    std::cout << "X = " << point.first << ", Y = " << point.second << "\n";
        //}
        //std::cout << "STATS (work_coeff): \n";
        //for (auto point : stats.work_coeff)
        //{
        //    std::cout << "X = " << point.first << ", Y = " << point.second << "\n";
        //}
        return 0;
    }

    //std::vector<Algorithm> algorithm_vector(thread_count);
    //std::vector<Job*> all_jobs;
    //std::vector<Worker*> all_workers;
    //for (auto& algorithm : algorithm_vector)
    //{
    //    Loader::Load(input_file, algorithm, all_workers, all_jobs);
    //    Loader::LoadPreferences(input_file, algorithm);
    //}

    assert(!weights.has_value());

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
        if (!Weights::are_valid(weights)) weights = Weights::fix(weights);
        algorithm.set_weights(weights);
        algorithm.run();
        //std::cout << " max time: " << algorithm.run() << "\n";
        std::size_t value = algorithm.get_penalty();
        //std::cout << " penalty: " << value << "\n";
        algorithm.reset();
        return value;
    };
    particle_swarm(-1, 1, calculate_value, input_files);
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
