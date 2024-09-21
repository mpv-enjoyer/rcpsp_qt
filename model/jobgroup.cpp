#include "jobgroup.h"

JobGroup::JobGroup(std::vector<Job*> want_jobs, int want_start_after, int want_end_before) :
    jobs(want_jobs), start_after(want_start_after), end_before(want_end_before)
{

}

int JobGroup::get_size() const
{
    return jobs.size();
}

int JobGroup::get_start() const
{
    return start_after;
}

int JobGroup::get_end() const
{
    return end_before;
}

Job* JobGroup::get_job(int index)
{
    return jobs[index];
}
