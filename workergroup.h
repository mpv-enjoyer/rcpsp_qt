#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#include "worker.h"

class WorkerGroup
{
private:
    int* current_time;
    std::vector<Worker*> workers;
public:
    WorkerGroup(int* time);
    void add_worker(Worker* new_worker);
    int get_earliest_placement_time(Job* job);
    void assign(Job* job);
};

#endif // WORKERGROUP_H
