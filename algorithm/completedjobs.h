#ifndef COMPLETEDJOBS_H
#define COMPLETEDJOBS_H

#include "algorithm.h"

class CompletedJobs
{
public:
    typedef AssignedJobs::Data Data;
    CompletedJobs();
    void add(Data data);
private:
    std::vector<Data> _data;
};

#endif // COMPLETEDJOBS_H
