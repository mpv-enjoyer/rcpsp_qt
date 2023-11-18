#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "jobgroup.h"
#include "workergroup.h"

struct JobPair
{
    int start_after;
    int end_before;
    Job* job;
    std::vector<WorkerGroup*> worker_groups;
};

struct ResultPair
{
    Job* job;
    Worker* worker;
    int start;
};

enum Preference
{
    NONE = 0,
    SPT = 1,
    LPT = 2
};

class Algorithm
{
    Preference preference = NONE;
    std::vector<JobPair> pending_jobs;
    int current_time;
    std::vector<int> pending_fronts;
    std::vector<ResultPair> completed_jobs;
    bool check_nearest_front();
public:
    Algorithm();
    void set_preference(Preference new_preference);
    void add_job_group(JobGroup* jobs, WorkerGroup* workers);
    void run();
    std::vector<ResultPair> get_completed();
    std::vector<JobPair> get_failed();
};

#endif // ALGORITHM_H
