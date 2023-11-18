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

class Algorithm
{
    std::vector<JobPair> pending_jobs;
    int current_time;
    std::vector<int> pending_fronts;
    std::vector<ResultPair> completed_jobs;
    void check_nearest_front();
public:
    Algorithm();
    void add_job_group(JobGroup* jobs, WorkerGroup* workers);
    void run();
    std::vector<ResultPair> get_completed();
    std::vector<JobPair> get_failed();
};

#endif // ALGORITHM_H
