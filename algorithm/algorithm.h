#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "jobgroup.h"
#include "workergroup.h"
#include <QtDebug>
#include <sstream>
//#include <QProgressBar>
#include <QFile>
#include <map>
#include <set>
#include <numeric>

class PendingJobs;
class PendingFronts;
class AssignedJobs;
class CompletedJobs;

enum Preference
{
    NONE = 0,
    SPT = 1,
    LPT = 2,
    FLS = 3
};

struct JobPair
{
    int start_after;
    int end_before;
    Job* job;
    std::vector<WorkerGroup*> worker_groups;
    int id;
    double worker_count;
    double current_preference = 0;
};

struct ResultPair
{
    Job* job;
    Worker* worker;
    int start;
    int job_id;
    int worker_group_id;
    int worker_internal_id;
};

using AlgorithmWeights = std::unordered_map<std::string, double>;
#define REGISTER_WEIGHT(N) const static std::string N = #N
namespace Weights
{
    REGISTER_WEIGHT(ancestors_per_job); // кол-во последователей / кол-во требований всего
    REGISTER_WEIGHT(ancestors_per_left); // кол-во последователей / кол-во оставшихся требований
    REGISTER_WEIGHT(avg_occupancy); // средняя занятость станка во время выполнения
    REGISTER_WEIGHT(bias);
    REGISTER_WEIGHT(critical_time_per_max_critical_time); // критическое время требования / максимальное критическое время всех требований
    REGISTER_WEIGHT(time_after_begin_per_overall_time); // время от начала выполнения до текущего момента / время всего на выполнение этого требования
    REGISTER_WEIGHT(worker_count_per_max_worker_count); // кол-во станков / максимальное число станков для требования
    constexpr static size_t SIZE = 7;
    const std::set<std::string> WeightsNames =
    {
        "ancestors_per_job", // кол-во последователей / кол-во требований всего
        "ancestors_per_left", // кол-во последователей / кол-во оставшихся требований
        "avg_occupancy", // средняя занятость станка во время выполнения
        "bias",
        "critical_time_per_max_critical_time", // критическое время требования / максимальное критическое время всех требований
        "time_after_begin_per_overall_time", // время от начала выполнения до текущего момента / время всего на выполнение этого требования
        "worker_count_per_max_worker_count"  // кол-во станков / максимальное число станков для требования
    };
    double get(AlgorithmWeights weights, std::string name);
    bool set(AlgorithmWeights& weights, std::string name, double value);
    bool are_valid(AlgorithmWeights& weights);
    //AlgorithmWeights fix(AlgorithmWeights weights);
    std::string to_string(AlgorithmWeights weights);
    AlgorithmWeights create_equal();
    AlgorithmWeights create_empty();
}

struct AlgorithmDataForWeights
{
    double job_count_not_assigned;
    double job_count_overall;
    double max_critical_time;
    double worker_max_count;
};

#include "pendingjobs.h"
#include "pendingfronts.h"
#include "assignedjobs.h"
#include "completedjobs.h"

struct FrontData
{
    int time;
    std::vector<JobPair> job_pairs;
};

struct SearchResult
{
    bool found;
    int pos;
};

#include <unordered_set>
#include "../arena_cpp.h"

class Algorithm
{
    Preference preference = NONE;
    std::vector<JobPair> _pending_jobs;
    std::vector<ResultPair> _completed_jobs;
    int look_ahead_time = 0;
    int longest_plan_loop = 0;
    int _failed_jobs_count = 0;
    std::size_t _penalty = 0;
    AlgorithmWeights _weights;
    static const int CURRENT_EQUAL_MAX = 1;
    int pass_max_count = 1;
    std::unique_ptr<Arena_Allocator> arena;
    static std::size_t calculate_penalty(std::vector<ResultPair>& result);
public:
    Algorithm();
    void add_job_group(JobGroup *jobs, std::vector<WorkerGroup *> worker_groups);
    void set_preference(Preference new_preference);
    void shuffle_pending_jobs();
    int run();
    std::vector<ResultPair> get_completed();
    int get_look_ahead_time() const;
    Preference get_preference() const;
    void set_look_ahead_time(int newLook_ahead_time);
    void set_weights(AlgorithmWeights weights);
    AlgorithmWeights get_weights();
    void reset();
    Arena_Allocator& reset_allocator_and_get(); // Remove everything!
    int get_failed_jobs_count();
    std::string get_string_result(const std::vector<ResultPair>& completed) const;
    std::size_t get_penalty() const;
    std::unordered_set<WorkerGroup*> get_worker_groups();
    int get_pass_max_count() const;
    void set_pass_max_count(int count);
    ~Algorithm();
};

#include <functional>

class Stats
{
public:
    std::map<double, double> wait_coeff; // Сколько работ Y выполняются через X относительных единиц (1 это 100% выделенного времени)
    std::map<double, double> work_coeff; // Какой процент X от выделенного времени уходит на выполнение работы, Y - количество работ
    std::size_t leads_to_impossible_jobs_counter = 0;
    Stats(std::vector<ResultPair> completed, double precision, bool print_raw = false);
    void print();
private:
    void init_coeff(std::map<double, double>& coeff, std::function<double(ResultPair)> calculate_coeff, std::vector<ResultPair>& completed, double precision);
};

#endif // ALGORITHM_H
