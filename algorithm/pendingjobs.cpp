#include "pendingjobs.h"
#include "pendingfronts.h"

PendingJobs::PendingJobs(int *current_time, PendingFronts *next, int look_ahead_time, std::vector<Data> data)
    : next(next), _current_time(current_time), _look_ahead_time(look_ahead_time), _data(data)
{
    for (int i = 0; i < _data.size(); i++)
    {
        next->add(_data[i].job->get_start_after());
        for (int j = 0; j < _data[i].worker_groups.size(); j++)
        {
            _data[i].worker_groups[j]->set_clock(current_time);
        }
    }

    for (auto job : _data)
    {
        int critical_time = job.job->init_critical_time();
        _max_critical_time = std::max(_max_critical_time, critical_time);
    }

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
    }
    std::sort(_data.begin(), _data.end(), [](const Data& l, const Data& r) -> bool { return l.job->get_start_after() < r.job->get_start_after(); });
}

bool PendingJobs::tick()
{
    int current_time = *(_current_time);
    bool result = false;
    for (int i = 0; i < _data.size(); i++)
    {
        result = true;
        if (_data[i].job->get_start_after() > current_time + _look_ahead_time) break; // sorted
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
