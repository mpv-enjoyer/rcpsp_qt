#ifndef WORKER_H
#define WORKER_H

#include "job.h"
#include "plan.h"

struct CurrentJob
{
    int time_start;
    Job* job;
    int will_be_free_at;
};

class Worker
{
private:
    int* clock;
    Plan plan;
    std::vector<CurrentJob> current_jobs;
    float current_occupancy = 0.0f;
public:
    Worker(Plan want_plan);
    void update();
    void set_clock(int* new_clock);
    void assign(Job* job);
    bool is_free(float occupancy);
    const Job* get_job(int index);
    int get_job_count();
    int will_be_free_after(float occupancy = 1.0f);
    const Plan get_plan();
};

#endif // WORKER_H
