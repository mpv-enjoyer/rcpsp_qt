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
    int* current_time;
    std::vector<Worker*> workers;
    int global_id;
public:
    WorkerGroup();
    void set_clock(int* clock);
    void set_global_id(int id);
    void add_worker(Worker* new_worker);
    Placement get_earliest_placement_time(Job* job);
    AssignedWorker assign(Job* job);
    int get_size();
    int get_global_id();
};

#endif // WORKERGROUP_H
