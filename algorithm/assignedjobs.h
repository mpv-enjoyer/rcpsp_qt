#ifndef ASSIGNEDJOBS_H
#define ASSIGNEDJOBS_H

#include "algorithm.h"

class AssignedJobs
{
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
    AssignedJobs();
private:
    std::vector<Data> _data;
};

#endif // ASSIGNEDJOBS_H
