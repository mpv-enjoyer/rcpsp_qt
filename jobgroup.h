#ifndef JOBGROUP_H
#define JOBGROUP_H

#include "job.h"

class JobGroup
{
private:
    std::vector<Job*> jobs;
    int start_after;
    int end_before;
public:
    JobGroup(std::vector<Job*> want_jobs, int want_start_after, int want_end_before);
    int get_size() const;
    int get_start() const;
    int get_end() const;
    Job* get_job(int index);
};

#endif // JOBGROUP_H
