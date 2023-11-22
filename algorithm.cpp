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


bool Algorithm::check_nearest_front()
{
    bool result = false;
    current_time = pending_fronts[0].time;
    FrontData current_front = pending_fronts[0];
    static int assigned_count = 0;

    for (int i = 0; i < assigned_jobs.size(); i++)
    {
        if (assigned_jobs[i].worker->is_free())
        {
            completed_jobs.push_back(assigned_jobs[i]);
            assigned_jobs.erase(assigned_jobs.begin() + i);
        }
    };

    for (int i = 0; i < pending_jobs.size(); i++)
    {
        if (pending_jobs[i].end_before <= current_time) continue;
        result = true;
        if (pending_jobs[i].start_after > current_time) continue;
        if (!pending_jobs[i].job->check_predecessors()) continue;
        current_front.job_pairs.push_back(pending_jobs[i]);
        pending_jobs.erase(pending_jobs.begin() + i);
        i--;
    }

    switch (preference)
    {
    case SPT:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_SPT);
        break;
    case LPT:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_LPT);
        break;
    case EST:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_EST);
        break;
    }

    for (int i = 0; i < current_front.job_pairs.size(); i++)
    {
        JobPair current_pending = current_front.job_pairs[i];
        int new_front_time = -1;
        for (int j = 0; j < current_pending.worker_groups.size(); j++)
        {
            int earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending.job);
            if (earliest_placement == -1) continue;
            if (earliest_placement == 0)
            {
                Worker* assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                assigned_jobs.push_back( {current_pending.job, assigned_to, current_time} );
                //qDebug() << "Moved to completed job" << current_pending.job->get_want_non_renewable() << " Time:" << current_time;
                current_pending.worker_groups[j]->set_clock(&current_time);
                current_front.job_pairs.erase(current_front.job_pairs.begin() + i);
                assigned_count++;
                new_front_time = -1;
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
        int new_index = pending_fronts.size();
        for (int j = 1; j < pending_fronts.size(); j++)
        {
            if (pending_fronts[j].time > new_front_time)
            {
                new_index = j;
                break;
            }
            if (pending_fronts[j].time == new_front_time)
            {
                new_index = -1;
                pending_fronts[j].job_pairs.push_back(current_pending);
                //qDebug() << "Pushed back to" << new_front_time <<"front Job" << current_pending.job->get_want_non_renewable();
                break;
            }
            //if (pending_fronts[j].time < new_front_time) new_index++;
        }
        if (new_index != -1)
        {
            //qDebug() << "Created front with Job " << current_pending.job->get_want_non_renewable() << "at" << current_time << " to Time" << new_front_time;
            pending_fronts.insert(pending_fronts.begin() + new_index, { new_front_time, { current_pending } });
        }
    }

    if (pending_fronts.size() == 1 && result)
    {
        //qDebug() << "Front copied ++ at" << current_time;
        pending_fronts[0] = current_front;
        pending_fronts[0].time++;
        current_time++;
        return true;
    }
    pending_fronts.erase(pending_fronts.begin());
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

void Algorithm::run()
{
    pending_fronts.clear();
    pending_fronts.push_back(FrontData{0, std::vector<JobPair>()});
    for (int i = 0; i < pending_jobs.size(); i++)
    {
        if (pending_jobs[i].job->check_predecessors() && pending_jobs[i].start_after <= 0)
        {
            pending_fronts[0].job_pairs.push_back(pending_jobs[i]);
        }
        for (int j = 0; j < pending_jobs[i].worker_groups.size(); j++)
        {
            pending_jobs[i].worker_groups[j]->set_clock(&current_time);
        }
    }
    begin_set_critical_time();

    qDebug() << "Sorting done";

    while (check_nearest_front())
    {
        /* Nothing? */
    }

    for (int i = 0; i < assigned_jobs.size(); i++)
    {
        completed_jobs.push_back(assigned_jobs[i]);
    };

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
