#ifndef PENDINGJOBS_H
#define PENDINGJOBS_H

#include "algorithm.h"

class PendingJobs
{
    const int* _current_time;
public:
    struct Data
    {
        int start_after;
        int end_before;
        Job* job;
        std::vector<WorkerGroup*> worker_groups;
        int id;
    };
    PendingJobs(const int* current_time) : _current_time(current_time) { }
    add(int start, int end, Job *job, std::vector<WorkerGroup *> workers);
private:
    std::vector<Data> _data;
};

#endif // PENDINGJOBS_H
