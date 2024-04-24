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
        JobPair new_pair = {start, end, jobs->get_job(i), { workers }, pending_jobs.size()};
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

bool compare_fronts(FrontData& lhs, FrontData& rhs)
{
    return (lhs.time < rhs.time);
}

bool compare_result(ResultPair& lhs, ResultPair& rhs)
{
    return (lhs.job_id > rhs.job_id);
}

bool Algorithm::check_nearest_front()
{
    bool result = false;
    current_time = pending_fronts[0].time;
    FrontData current_front = pending_fronts[0];
    static int assigned_count = 0;

    for (int i = 0; i < assigned_jobs.size(); i++)
    {
        assigned_jobs[i].worker->update();
        bool found = false;
        for (int j = 0; j < assigned_jobs[i].worker->get_job_count(); j++)
        {
            found |= (assigned_jobs[i].worker->get_job(j) == assigned_jobs[i].job);
        }
        //TODO: increased complexity here
        if (!found)
        {
            completed_jobs.push_back(assigned_jobs[i]);
            assigned_jobs.erase(assigned_jobs.begin() + i);
            i--;
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
            Placement earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending.job);
            //qDebug() << "check" << current_pending.id << "job, got" << earliest_placement.time_before << "current:" << current_time << "groupsize:" << current_pending.worker_groups[0]->get_size();
            if (!earliest_placement.worker) continue;
            if (earliest_placement.time_before == 0)
            {
                AssignedWorker assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                assigned_jobs.push_back( {current_pending.job, assigned_to.worker, current_time, current_pending.id, j, assigned_to.internal_id} );
                //qDebug() << "Moved to completed job" << current_pending.job->get_want_non_renewable() << " Time:" << current_time;
                current_pending.worker_groups[j]->set_clock(&current_time);
                current_front.job_pairs.erase(current_front.job_pairs.begin() + i);
                assigned_count++;
                new_front_time = -1;
                i--;
                break;
            }
            if (earliest_placement.time_before > 0 && earliest_placement.time_before <= look_ahead_time)
            {
                earliest_placement.worker->preserve(earliest_placement.time_before);
            }
            if (new_front_time == -1 || new_front_time > earliest_placement.time_before)
            {
                new_front_time = earliest_placement.time_before;
            }
        }
        if (new_front_time == -1) continue;
        result = true;
        new_front_time += current_time;
        FrontData new_front_data = { new_front_time, { current_pending } };
        SearchResult result = binarySearch(pending_fronts, new_front_data);
        if (!result.found)
        {
            pending_fronts.insert(pending_fronts.begin() + result.pos, new_front_data);
        }
        else
        {
            pending_fronts[result.pos].job_pairs.push_back(current_pending);
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
    std::vector<Job*>* current_ancestors = current_job_pair.job->get_ancestors();
    for (int j = 0; j < current_ancestors->size(); j++)
    {
        int internal_result = set_critical_time( { current_ancestors->at(j)->get_start_after(), current_ancestors->at(j)->get_end_before(), current_ancestors->at(j) } );
        result = internal_result < result ? internal_result : result;
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
        pending_jobs[i].job->set_start_after(pending_jobs[i].start_after);
        pending_jobs[i].job->set_end_before(pending_jobs[i].end_before);
        for (int j = 0; j < pending_jobs[i].worker_groups.size(); j++)
        {
            pending_jobs[i].worker_groups[j]->set_clock(&current_time);
        }
    }
    begin_set_critical_time();

    qDebug() << "Sorting done";

    for (int i = 0; i < pending_jobs.size(); i++)
    {
        if (pending_jobs[i].job->check_predecessors() && pending_jobs[i].start_after <= 0)
        {
            pending_fronts[0].job_pairs.push_back(pending_jobs[i]);
            pending_jobs.erase(pending_jobs.begin() + i);
        }
    }

    while (check_nearest_front())
    {
        /* Nothing? */
    }

    for (int i = 0; i < assigned_jobs.size(); i++)
    {
        completed_jobs.push_back(assigned_jobs[i]);
    };

    std::sort(completed_jobs.begin(), completed_jobs.end(), compare_result);

    qDebug() << "Ready to display";

}

SearchResult Algorithm::binarySearch(const std::vector<FrontData> &arr, const FrontData& x)
{
    int r = arr.size() - 1;
    int l = 0;
    while (l <= r) {
        int m = l + (r - l) / 2;

        // Check if x is present at mid
        if (arr[m].time == x.time)
            return { true, m };

        // If x greater, ignore left half
        if (arr[m].time < x.time)
            l = m + 1;

        // If x is smaller, ignore right half
        else
            r = m - 1;
    }

    // If we reach here, then element was not present
    return { false, l };
}

std::vector<ResultPair> Algorithm::get_completed()
{
    return completed_jobs;
}

std::vector<JobPair> Algorithm::get_failed()
{
    return pending_jobs;
}

void Algorithm::set_log_bar(QProgressBar *bar)
{
    log_bar = bar;
}

