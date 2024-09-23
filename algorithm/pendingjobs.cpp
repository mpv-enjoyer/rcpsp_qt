#include "pendingjobs.h"

void PendingJobs::add(int start, int end, Job* job, std::vector<WorkerGroup*> workers)
{
    Data to_add = {start, end, job, workers, static_cast<int>(pending_jobs.size())};
    _data.push_back(to_add);
}

bool PendingJobs::tick()
{
    int current_time = *(_current_time);
    bool result = false;
    for (int i = 0; i < _data.size(); i++)
    {
        result = true; //moved up to not give up after current_time is too large.
        if (_data[i].start_after > current_time + _look_ahead_time) continue; // previously -
        if (!(_data[i].job->check_predecessors())) continue;

        next->add(current_time, _data[i]);
        _data.erase(_data.begin() + i);
        i--;
    }
    return result;
}
