#ifndef PENDINGJOBS_H
#define PENDINGJOBS_H

#include "algorithm.h"

class PendingJobs
{
    const int _look_ahead_time;
    int* _current_time;
    PendingFronts* next;
public:
    typedef JobPair Data;
    PendingJobs(int* current_time, PendingFronts* next, int look_ahead_time, std::vector<Data> data);
    void add(int start, int end, Job *job, std::vector<WorkerGroup *> workers) = delete; // Initialize in constructor
    bool tick();
private:
    std::vector<Data> _data;
    int set_critical_time(Data current_job_pair);
    void begin_set_critical_time();
};

#endif // PENDINGJOBS_H
