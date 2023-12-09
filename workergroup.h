#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#include "worker.h"

struct AssignedWorker
{
    Worker* worker;
    int internal_id;
};

class WorkerGroup
{
private:
    int* current_time;
    std::vector<Worker*> workers;
public:
    WorkerGroup();
    void set_clock(int* clock);
    void add_worker(Worker* new_worker);
    int get_earliest_placement_time(Job* job);
    AssignedWorker assign(Job* job);
    int get_size();
};

#endif // WORKERGROUP_H
