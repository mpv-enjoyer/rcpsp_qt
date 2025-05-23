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

    for (const auto& job : _data)
    {
        bool job_possible = false;
        for (const auto worker_group : job.worker_groups)
        {
            job_possible |= worker_group->check_if_job_is_possible(job.job);
        }
        if (!job_possible)
        {
            throw std::invalid_argument("Job is impossible for current plan");
        }
        if (_max_critical_time < job.job->get_critical_time()) _max_critical_time = job.job->get_critical_time();
    }
    std::sort(_data.begin(), _data.end(), [](const Data& l, const Data& r) -> bool { return l.start_after < r.start_after; });
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
    double output = result - current_job_pair.job->get_time_to_spend();
    if (output == -1) output = 0;
    current_job_pair.job->set_critical_time(output);
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
        result = true;
        if (_data[i].start_after > current_time + _look_ahead_time) break; // sorted
        if (!(_data[i].job->check_predecessors())) continue;

        next->add(current_time, _data[i]);
        _data.erase(_data.begin() + i);
        i--;
    }
    //qDebug() << "Pending jobs left:" << _data.size();
    return result;
}

std::size_t PendingJobs::data_size()
{
    return _data.size();
}

int PendingJobs::get_max_critical_time()
{
    return _max_critical_time;
}
