#ifndef WORKER_H
#define WORKER_H

#include "job.h"
#include "plan.h"

struct CurrentJob
{
    int time_start;
    Job* job;
};

class Worker
{
private:
    int* clock;
    Plan plan;
    CurrentJob current_job;
public:
    Worker(std::vector<PlanElement> want_plan, int want_start, int* want_clock);
    void assign(Job* job);
    bool is_free();
    const Job* get_job();
    int left_before_free();
    const Plan Worker::get_plan();
};

class WorkerGroup
{

};

#endif // WORKER_H
