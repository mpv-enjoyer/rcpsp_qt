#include "assignedjobs.h"
#include "completedjobs.h"

AssignedJobs::AssignedJobs(const int *current_time, CompletedJobs *next)
    : _current_time(current_time), next(next)
{

}

void AssignedJobs::add(int start, int worker_group_id, int worker_internal_id, PendingJobs::Data job_pair, Worker* worker)
{
    Data data;
    data.job = job_pair.job;
    data.job_id = job_pair.id;
    data.start = start;
    data.worker = worker;
    data.worker_group_id = worker_group_id;
    data.worker_internal_id = worker_internal_id;
    _data.push_back(data);
}

bool AssignedJobs::tick()
{
    bool result = false;
    int current_time = *(_current_time);
    for (int i = 0; i < _data.size(); i++)
    {
        _data[i].worker->update();
        int job_end = _data[i].start + _data[i].job->get_time_to_spend();
        bool job_ended = job_end >= current_time;
        if (job_ended)
        {
            next->add(_data[i]);
            _data.erase(_data.begin() + i);
            i--;
            result = true;
        }
    }
    return result;
}
