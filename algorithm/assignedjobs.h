#ifndef ASSIGNEDJOBS_H
#define ASSIGNEDJOBS_H

#include "algorithm.h"

class AssignedJobs
{
    const int* _current_time;
    CompletedJobs* next;
public:
    struct Data
    {
        Job* job;
        Worker* worker;
        int start;
        int job_id;
        int worker_group_id;
        int worker_internal_id;
    };
    AssignedJobs(const int* current_time, CompletedJobs* next);
    void add(int start, int worker_group_id, int worker_internal_id, PendingJobs::Data job_pair, Worker* worker);
    bool tick();
private:
    std::vector<Data> _data;
};

#endif // ASSIGNEDJOBS_H
