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
        pending_jobs.push_back( {start, end, jobs->get_job(i), workers} );
    }
}

void Algorithm::check_nearest_front()
{
    int current_time = pending_fronts[0];
    for (int i = 0; i < pending_gobs.size(); i++)
    {
        JobPair current_pending = pending_jobs[i];
        if (current_pending.start_after > current_time) continue;
        if (current_pending.end_before <= current_time) continue;
        if (!current_pending.job->check_predecessors()) continue;
        int new_front_time = -1;
        for (int j = 0; j < current_pending.worker_groups.size(); j++)
        {
            int earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending);
            if (earliest_placement == -1) continue;
            if (earliest_placement == 0)
            {
                Worker* assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                completed_jobs.push_back( {current_pending.job, assigned_to, current_time} );
                pending_jobs.erase(pending_jobs.begin() + i);
                continue;
            }
            if (new_front_time = -1 || new_front_time > earliest_placement)
            {
                new_front_time = earliest_placement;
            }
        }
        if (new_front_time == -1) continue;
        new_front_time += current_time;
        int new_index = 0;
        for (int j = 0; j < pending_fronts[j]; j++)
        {
            if (pending_fronts[j] > new_front_time) break;
            if (pending_fronts[j] == new_front_time) new_index = -1;
            if (pending_fronts[j] < new_front_time) new_index++;
        }
        if (new_index != -1)
        {
            pending_fronts.insert(pending_fronts.begin() + new_index, new_front_time);
        }
    }
}
