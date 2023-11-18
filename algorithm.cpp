#include "algorithm.h"

Algorithm::Algorithm()
{

}

void Algorithm::add_job_group(JobGroup* jobs, WorkerGroup* workers)
{
    for (int i = 0; i < jobs->get_size(); i++)
    {
        int start = jobs->get_start();
        int end = jobs->get_end();
        JobPair new_pair = {start, end, jobs->get_job(i), { workers }};
        pending_jobs.push_back(new_pair);
    }
}

void Algorithm::set_preference(Preference new_preference)
{
    preference = new_preference;
}

bool Algorithm::check_nearest_front()
{
    bool result = false;
    current_time = pending_fronts[0];
    pending_fronts.erase(pending_fronts.begin());

    //Workaround: in some cases completed_jobs can't properly update
    for (int i = 0; i < completed_jobs.size(); i++)
    {
        completed_jobs[i].worker->update();
    }

    for (int i = 0; i < pending_jobs.size(); i++)
    {
        JobPair current_pending = pending_jobs[i];
        if (current_pending.start_after > current_time)
        {
            result = true;
            continue;
        }
        if (current_pending.end_before <= current_time) continue;
        result = true;
        if (!current_pending.job->check_predecessors()) continue;
        int new_front_time = -1;
        for (int j = 0; j < current_pending.worker_groups.size(); j++)
        {
            int earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending.job);
            if (earliest_placement == -1) continue;
            if (earliest_placement == 0)
            {
                Worker* assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                completed_jobs.push_back( {current_pending.job, assigned_to, current_time} );
                current_pending.worker_groups[j]->set_clock(&current_time);
                pending_jobs.erase(pending_jobs.begin() + i);
                i--;
                break;
            }
            if (new_front_time == -1 || new_front_time > earliest_placement)
            {
                new_front_time = earliest_placement;
            }
        }
        if (new_front_time == -1) continue;
        new_front_time += current_time;
        int new_index = 0;
        for (int j = 0; j < pending_fronts.size(); j++)
        {
            if (pending_fronts[j] > new_front_time) break;
            if (pending_fronts[j] == new_front_time)
            {
                new_index = -1;
                break;
            }
            if (pending_fronts[j] < new_front_time) new_index++;
        }
        if (new_index != -1)
        {
            pending_fronts.insert(pending_fronts.begin() + new_index, new_front_time);
        }
    }
    if (pending_fronts.size() == 0 && result)
    {
        pending_fronts.push_back(current_time + 1);
    }
    return result;
}

bool compare_SPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() < rhs.job->get_time_to_spend());
}

bool compare_LPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() > rhs.job->get_time_to_spend());
}

void Algorithm::run()
{
    pending_fronts = { 0 };
    for (int i = 0; i < pending_jobs.size(); i++)
    {
        for (int j = 0; j < pending_jobs[i].worker_groups.size(); j++)
        {
            pending_jobs[i].worker_groups[j]->set_clock(&current_time);
        }
    }

    switch (preference)
    {
    case SPT:
        std::sort(pending_jobs.begin(), pending_jobs.end(), compare_SPT);
        break;
    case LPT:
        std::sort(pending_jobs.begin(), pending_jobs.end(), compare_LPT);
    }

    while (check_nearest_front())
    {
        /* Nothing? */
    }
}

std::vector<ResultPair> Algorithm::get_completed()
{
    return completed_jobs;
}

std::vector<JobPair> Algorithm::get_failed()
{
    return pending_jobs;
}
