#ifndef PENDINGJOBS_H
#define PENDINGJOBS_H

#include "algorithm.h"

class PendingJobs
{
    const int _look_ahead_time;
    int* _current_time;
    PendingFronts* next;
public:
    struct Data
    {
        int start_after;
        int end_before;
        Job* job;
        std::vector<WorkerGroup*> worker_groups;
        int id;
    };
    PendingJobs(int* current_time, PendingFronts* next, int look_ahead_time) : next(next), _current_time(current_time), _look_ahead_time(look_ahead_time) { }
    void add(int start, int end, Job *job, std::vector<WorkerGroup *> workers);
    bool tick();
private:
    std::vector<Data> _data;
};

#endif // PENDINGJOBS_H
