#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#include "worker.h"

struct AssignedWorker
{
    Worker* worker;
    int internal_id;
};

struct Placement
{
    Worker* worker;
    int time_before;
};

class WorkerGroup
{
private:
    Shared shared;
    std::vector<Worker*> workers;
    int global_id;
public:
    WorkerGroup();
    void set_shared(Shared shared);
    void set_global_id(int id);
    void add_worker(Worker* new_worker);
    Placement get_earliest_placement_time(Job* job);
    AssignedWorker assign(Job* job);
    int get_size();
    int get_global_id();
    bool check_if_job_is_possible(const Job* job);
    const Worker* get_worker(int id) const;
};

#endif // WORKERGROUP_H
