#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "jobgroup.h"
#include "workergroup.h"
#include <QtDebug>
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
#define REGISTER_WEIGHT(N) const std::string N = #N
namespace Weights
{
    REGISTER_WEIGHT(ancestors_per_left); // кол-во последователей / кол-во оставшихся требований
    REGISTER_WEIGHT(ancestors_per_job); // кол-во последователей / кол-во требований всего
    REGISTER_WEIGHT(critical_time_per_max_critical_time); // критическое время требования / максимальное критическое время всех требований
    REGISTER_WEIGHT(avg_occupancy); // средняя занятость станка во время выполнения
    REGISTER_WEIGHT(time_after_begin_per_overall_time); // время от начала выполнения до текущего момента / время всего на выполнение этого требования
    constexpr static size_t SIZE = 5;
    const std::set<std::string> WeightsNames =
    {
        "ancestors_per_left", // кол-во последователей / кол-во оставшихся требований
        "ancestors_per_job", // кол-во последователей / кол-во требований всего
        "critical_time_per_max_critical_time", // критическое время требования / максимальное критическое время всех требований
        "avg_occupancy", // средняя занятость станка во время выполнения
        "time_after_begin_per_overall_time" // время от начала выполнения до текущего момента / время всего на выполнение этого требования
    };
    double get(AlgorithmWeights weights, std::string name);
    bool set(AlgorithmWeights& weights, std::string name, double value);
    bool are_valid(AlgorithmWeights weights);
    AlgorithmWeights fix(AlgorithmWeights weights);
    std::string to_string(AlgorithmWeights weights);
    AlgorithmWeights create_equal();
}

struct AlgorithmDataForWeights
{
    double job_count_not_assigned;
    double job_count_overall;
    double max_critical_time;
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

class Algorithm
{
    Preference preference = NONE;
    std::vector<JobPair> _pending_jobs;
    std::vector<ResultPair> _completed_jobs;
    int look_ahead_time = 0;
    int longest_plan_loop = 0;
    int _failed_jobs_count = 0;
    AlgorithmWeights _weights;
    static const int CURRENT_EQUAL_MAX = 1;
    static const int PASS_MAX_COUNT = 1;
public:
    Algorithm();
    void add_job_group(JobGroup *jobs, std::vector<WorkerGroup *> worker_groups);
    void set_preference(Preference new_preference);
    int run();
    std::vector<ResultPair> get_completed();
    int get_look_ahead_time() const;
    Preference get_preference() const;
    void set_look_ahead_time(int newLook_ahead_time);
    void set_weights(AlgorithmWeights weights);
    void reset();
    int get_failed_jobs_count();
};

struct Stats
{
    std::map<double, double> wait_coeff; // Сколько работ Y выполняются через X относительных единиц (1 это 100% выделенного времени)
    std::map<double, double> work_coeff; // Какой процент X от выделенного времени уходит на выполнение работы, Y - количество работ
    Stats(std::vector<ResultPair> completed, double precision);
};

#endif // ALGORITHM_H
