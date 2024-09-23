#ifndef COMPLETEDJOBS_H
#define COMPLETEDJOBS_H

#include "algorithm.h"

class CompletedJobs
{
public:
    typedef ResultPair Data;
    CompletedJobs();
    void add(Data data);
    std::vector<Data> result();
    int failed_count();
    int succeded_count();
    void prepare_for_next_iteration();
private:
    std::vector<Data> _data;
};

#endif // COMPLETEDJOBS_H
