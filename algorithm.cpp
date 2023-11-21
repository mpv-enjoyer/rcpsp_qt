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

    static int assigned_count = 0;

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
                assigned_count++;
                if (assigned_count % 500 == 0) qDebug() << "Assigned job " << assigned_count;
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

int Algorithm::set_critical_time(JobPair current_job_pair)
{
    if (current_job_pair.job->critical_time_exists())
    {
        return current_job_pair.job->get_critical_time();
    }
    int result = current_job_pair.end_before;
    for (int i = 0; i < pending_jobs.size(); i++)
    {
        if (pending_jobs[i].job->is_predecessor(current_job_pair.job))
        {
            int internal_result = set_critical_time(pending_jobs[i]);
            result = internal_result < result ? internal_result : result;
        }
    }
    current_job_pair.job->set_critical_time(result - current_job_pair.job->get_time_to_spend());
    return result - current_job_pair.job->get_time_to_spend();
}

void Algorithm::begin_set_critical_time()
{
    for (int i = 0; i < pending_jobs.size(); i++)
    {
        if (i % 500 == 0) qDebug() << "Sorting element " << i;
        set_critical_time(pending_jobs[i]);
    }
}

bool compare_SPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() < rhs.job->get_time_to_spend());
}

bool compare_LPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() > rhs.job->get_time_to_spend());
}

bool compare_EST(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_critical_time() < rhs.job->get_critical_time());
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
    begin_set_critical_time();
    switch (preference)
    {
    case SPT:
        std::sort(pending_jobs.begin(), pending_jobs.end(), compare_SPT);
        break;
    case LPT:
        std::sort(pending_jobs.begin(), pending_jobs.end(), compare_LPT);
        break;
    case EST:
        std::sort(pending_jobs.begin(), pending_jobs.end(), compare_EST);
        break;
    }

    qDebug() << "Sorting done";

    while (check_nearest_front())
    {
        /* Nothing? */
    }

    qDebug() << "Ready to display";

}

std::vector<ResultPair> Algorithm::get_completed()
{
    return completed_jobs;
}

std::vector<JobPair> Algorithm::get_failed()
{
    return pending_jobs;
}
