#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "jobgroup.h"
#include "workergroup.h"
#include <QtDebug>
#include <QProgressBar>

struct JobPair
{
    int start_after;
    int end_before;
    Job* job;
    std::vector<WorkerGroup*> worker_groups;
    int id;
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

struct FrontData
{
    int time;
    std::vector<JobPair> job_pairs;
};

enum Preference
{
    NONE = 0,
    SPT = 1,
    LPT = 2,
    EST = 3
};

struct SearchResult
{
    bool found;
    int pos;
};

class Algorithm
{
    Preference preference = NONE;
    std::vector<JobPair> pending_jobs;
    int previous_time;
    int current_time;
    std::vector<FrontData> pending_fronts;
    std::vector<ResultPair> completed_jobs;
    std::vector<ResultPair> assigned_jobs; //To later become completed.
    bool check_nearest_front();
    int set_critical_time(JobPair current_job_pair);
    void begin_set_critical_time();
    int initial_size_divided_by_100 = 1;
    QProgressBar* log_bar = nullptr;
public:
    Algorithm();
    void set_preference(Preference new_preference);
    void add_job_group(JobGroup* jobs, WorkerGroup* workers);
    void run();
    SearchResult binarySearch(const std::vector<FrontData>& arr, const FrontData& x);
    std::vector<ResultPair> get_completed();
    std::vector<JobPair> get_failed();
    void set_log_bar(QProgressBar* bar);
};

#endif // ALGORITHM_H
