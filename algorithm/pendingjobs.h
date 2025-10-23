#ifndef PENDINGJOBS_H
#define PENDINGJOBS_H

#include "algorithm.h"

class PendingJobs
{
    friend class basic; //TESTS
    int* _current_time;
    PendingFronts* next;
    const int _look_ahead_time;
    int _max_critical_time = 0;
    std::vector<JobPair> _data;
public:
    typedef JobPair Data;
    PendingJobs(int* current_time, PendingFronts* next, int look_ahead_time, std::vector<Data> data);
    bool tick();
    std::size_t data_size();
    int get_max_critical_time();
};

#endif // PENDINGJOBS_H
