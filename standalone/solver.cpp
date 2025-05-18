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
    GLOBAL_LOG("Point (");
    std::cout << "Point (";
    for (auto position_dimension : point)
    {
        GLOBAL_LOG(std::to_string(position_dimension) + ", ");
        std::cout << position_dimension << ", ";
    }
    GLOBAL_LOG(")\n");
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
        GLOBAL_LOG("\n");
        std::cout << "\n";
        GLOBAL_LOG(std::to_string(i) + ") Best particle is f(");
        std::cout << i << ") Best particle is f(";
        for (auto position_dimension : m_position)
        {
            GLOBAL_LOG(std::to_string(position_dimension) + ", ");
            std::cout << position_dimension << ", ";
        }
        GLOBAL_LOG(std::string(") = ") + std::to_string(m_value) + std::string("\n"));
        std::cout << ") = " << m_value << "\n";
    }
};

std::pair<Point, double> particle_swarm(
    double min,
    double max,
    std::function<double(Point, std::string)> calculate_value,
    std::vector<std::string>& input_files,
    const int PROCESSOR_CORE_COUNT,
    const int PARTICLE_COUNT_COEFF)
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
        double best_value = __DBL_MAX__;
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
            position = reflect_boundaries(position);
            update_best();
        }
        Point reflect_boundaries(Point position) // TODO: use min max from layer above here
        {
            for (std::size_t i = 0; i < DIMENSIONS; i++)
            {
                auto& dim = position[i];
                auto& vel = velocity[i];
                dim = std::clamp(dim, -2.0, 2.0);
                if (dim > 1)
                {
                    dim = 1 - (dim - 1);
                    vel = -vel;
                }
                else if (dim < -1)
                {
                    dim = (-1) - (dim - (-1));
                    vel = -vel;
                }
            }
            return position;
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
            GLOBAL_LOG(std::string("Penalty sum (") + std::to_string(value) + std::string(") for"));
            std::cout << "Penalty sum (" << value << ") for ";
            print_point(position);
            best.check_update(position, value);
        }
        
        Point normalize_point(Point point) // Use reflect_boundaries instead!
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
            position = reflect_boundaries(position);
            update_best();
        }
    };

    //const int PROCESSOR_CORE_COUNT = 1;
    //const int PARTICLE_COUNT_COEFF = 1;

    //const int PARTICLE_COUNT = (PROCESSOR_CORE_COUNT - 1) * PARTICLE_COUNT_COEFF;
    // const int PARTICLE_COUNT = 11;//algorithm_vector.size();
    const double PARTICLE_MIN = min;
    const double PARTICLE_MAX = max;

    //std::vector<Particle> particles(PARTICLE_COUNT, Particle(PARTICLE_MIN, PARTICLE_MAX, best));
    //std::mutex mutex_for_particles;
    std::vector<std::vector<Particle>> particles;
    for (int coeff_id = 0; coeff_id < PARTICLE_COUNT_COEFF; coeff_id++)
    {
        particles.push_back(std::vector<Particle>());
        std::vector<std::future<Particle>> particle_futures;

        for (int particle_common_id = 0; particle_common_id < PROCESSOR_CORE_COUNT; particle_common_id++)
        {
            particle_futures.emplace_back(std::async(std::launch::async, [&]() -> Particle
            {
                return Particle(PARTICLE_MIN, PARTICLE_MAX, best, input_files, calculate_value);
            }));
        }
        for (auto& particle : particle_futures)
        {
            particle.wait();
            particles.back().push_back(particle.get());
        }
        GLOBAL_LOG("|");
        std::cout << "|";
    }

    for (int i = 0; i < 50; i++)
    {
        for (int coeff_id = 0; coeff_id < PARTICLE_COUNT_COEFF; coeff_id++)
        {
            std::vector<std::unique_ptr<std::thread>> threads;
            for (auto& particle : particles[coeff_id])
            {
                threads.push_back(std::make_unique<std::thread>([&](){ particle.update(); }));
            }
            for (auto& t : threads)
            {
                if (t->joinable()) t->join();
            }
            GLOBAL_LOG("|");
            std::cout << "|";
        }
        best.print(i);
    }
    return {best.m_position, best.m_value};
}