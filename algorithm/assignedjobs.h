#ifndef ASSIGNEDJOBS_H
#define ASSIGNEDJOBS_H

#include "algorithm.h"

class AssignedJobs
{
    const int* _current_time;
    CompletedJobs* next;
public:
    typedef ResultPair Data;
    AssignedJobs(const int* current_time, CompletedJobs* next);
    void add(int start, int worker_group_id, int worker_internal_id, JobPair job_pair, Worker* worker);
    bool tick();
private:
    std::vector<Data> _data;
};

#endif // ASSIGNEDJOBS_H
