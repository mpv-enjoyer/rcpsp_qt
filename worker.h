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
    int preserved_until = -1;
public:
    Worker(Plan want_plan);
    void update();
    float current_occupancy();
    void set_clock(int* new_clock);
    void assign(Job* job);
    const Job* get_job(int index);
    int get_job_count();
    int can_be_placed_after(std::vector<OccupancyPair> occupancy);
    void preserve(int interval);
    const Plan get_plan();
    bool is_free(std::vector<OccupancyPair> want_occupancy, int fetch_time = -1);
};

#endif // WORKER_H
