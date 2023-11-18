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
    void update();
    Worker(Plan want_plan);
    void set_clock(int* new_clock);
    void assign(Job* job);
    bool is_free();
    const Job* get_job();
    int will_be_free_at();
    const Plan get_plan();
};

#endif // WORKER_H
