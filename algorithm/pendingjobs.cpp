#include "pendingjobs.h"
#include "pendingfronts.h"

PendingJobs::PendingJobs(int *current_time, PendingFronts *next, int look_ahead_time, std::vector<Data> data)
    : next(next), _current_time(current_time), _look_ahead_time(look_ahead_time), _data(data)
{
    for (int i = 0; i < _data.size(); i++)
    {
        next->add(_data[i].start_after);
        _data[i].job->set_start_after(_data[i].start_after);
        _data[i].job->set_end_before(_data[i].end_before); // IDK why didn't we do it earlier
        for (int j = 0; j < _data[i].worker_groups.size(); j++)
        {
            _data[i].worker_groups[j]->set_clock(current_time);
        }
    }
    begin_set_critical_time();
}

int PendingJobs::set_critical_time(Data current_job_pair)
{
    if (current_job_pair.job->critical_time_exists())
    {
        return current_job_pair.job->get_critical_time();
    }
    int result = current_job_pair.end_before;
    std::vector<Job*>* current_ancestors = current_job_pair.job->get_ancestors();
    for (int j = 0; j < current_ancestors->size(); j++)
    {
        int internal_result = set_critical_time( { current_ancestors->at(j)->get_start_after(), current_ancestors->at(j)->get_end_before(), current_ancestors->at(j) } );
        result = internal_result < result ? internal_result : result;
    }
    current_job_pair.job->set_critical_time(result - current_job_pair.job->get_time_to_spend());
    return result - current_job_pair.job->get_time_to_spend();
}

void PendingJobs::begin_set_critical_time()
{
    for (int i = 0; i < _data.size(); i++)
    {
        set_critical_time(_data[i]);
    }
}

//void PendingJobs::add(int start, int end, Job* job, std::vector<WorkerGroup*> workers)
//{
//    Data to_add = {start, end, job, workers, static_cast<int>(_data.size())};
//    _data.push_back(to_add);
//    next->add(start);
//}

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
    qDebug() << "Pending jobs left:" << _data.size();
    return result;
}
