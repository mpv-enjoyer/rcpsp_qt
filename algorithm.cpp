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
        JobPair new_pair = {start, end, jobs->get_job(i), { workers }, static_cast<int>(pending_jobs.size())};
        pending_jobs.push_back(new_pair);
    }
    for (int i = 0; i < workers->get_size(); i++)
    {
        if (workers->get_worker(i)->get_plan().get_time_loop() > longest_plan_loop)
            longest_plan_loop = workers->get_worker(i)->get_plan().get_time_loop();
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

    qDebug() << current_time << current_front.job_pairs.size() << completed_jobs.size() << assigned_jobs.size() << '\n';

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
        if (pending_jobs[i].start_after > current_time - look_ahead_time) continue;
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
    case FLS:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_EST);
        break;
    }

    for (int i = 0; i < current_front.job_pairs.size(); i++)
    {
        int coeff = current_front.job_pairs[i].job->get_preference_coefficient();
        if (coeff == 0) continue;
        int j = i - coeff;
        if (j < 0) j = 0;
        std::swap(current_front.job_pairs[i], current_front.job_pairs[j]);
        //auto copy = current_front.job_pairs[i];
        //current_front.job_pairs.erase(current_front.job_pairs.begin() + i);
        //current_front.job_pairs.insert(current_front.job_pairs.begin() + j, copy);
    }

    for (int i = 0; i < current_front.job_pairs.size(); i++)
    {
        JobPair current_pending = current_front.job_pairs[i];
        int new_front_time = -1;
        std::vector<Placement> placements;
        for (int j = 0; j < current_pending.worker_groups.size(); j++)
        {
            Placement earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending.job);
            //qDebug() << "check" << current_pending.id << "job, got" << earliest_placement.time_before << "current:" << current_time << "groupsize:" << current_pending.worker_groups[0]->get_size();
            if (!earliest_placement.worker) continue;
            placements.push_back(earliest_placement);
            if (earliest_placement.time_before == 0)
            {
                AssignedWorker assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                assigned_jobs.push_back( {current_pending.job, assigned_to.worker, current_time, current_pending.job->get_global_id(), current_pending.worker_groups[j]->get_global_id(), assigned_to.internal_id} );
                qDebug() << "Moved to completed job" << current_pending.job->get_global_id() << current_time;
                current_pending.worker_groups[j]->set_clock(&current_time);
                current_front.job_pairs.erase(current_front.job_pairs.begin() + i);
                assigned_count++;
                new_front_time = -1;
                i--;
                break;
            }
            if (earliest_placement.time_before > 0 && earliest_placement.time_before <= look_ahead_time)
            {
                if (!earliest_placement.worker->is_preserved())
                    earliest_placement.worker->preserve(earliest_placement.time_before);
                // if (new_front_time > earliest_placement.time_before)
            }
            if (new_front_time == -1 || new_front_time > earliest_placement.time_before)
            {
                new_front_time = earliest_placement.time_before;
            }
        }
        //TODO: Process placements?
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

    if (last_loop_check_begin == -1 && !result)
    {
        last_loop_check_begin = current_time;
        // If we currently see no action and
        // every pending job is no longer in boundaries
        // we do one last plan loop and wait for any action.
    }

    if (result) last_loop_check_begin = -1;

    bool should_copy_front_plus_one = false;
    should_copy_front_plus_one |= pending_fronts.size() == 1 && result;
    if (last_loop_check_begin != 1 &&
        last_loop_check_begin + longest_plan_loop < current_time &&
        pending_fronts.size() == 1)
    {
        should_copy_front_plus_one = true;
        qDebug() << "Last loop check begin is being used!!" << '\n';
    }
    if (should_copy_front_plus_one)
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
        set_critical_time(pending_jobs[i]);
    }
}

int Algorithm::get_look_ahead_time() const
{
    return look_ahead_time;
}

void Algorithm::set_look_ahead_time(int newLook_ahead_time)
{
    look_ahead_time = newLook_ahead_time;
}

void Algorithm::run()
{
    std::vector<JobPair> input = pending_jobs;
    int best_failed_jobs = __INT_MAX__;
    int current_failed_jobs = 0;
    int current_equal_failed = 0;
    std::vector<ResultPair> best_completed_jobs;
    last_loop_check_begin = -1;
    longest_plan_loop = 0;

    while (true)
    {
        assigned_jobs.clear();
        completed_jobs.clear();
        current_time = 0;
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
            if (pending_jobs[i].job->check_predecessors() && pending_jobs[i].start_after <= look_ahead_time)
            {
                pending_fronts[0].job_pairs.push_back(pending_jobs[i]);
                pending_jobs.erase(pending_jobs.begin() + i);
                i--;
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

        current_failed_jobs = 0;
        for (auto job : completed_jobs)
        {
            current_failed_jobs += job.start + job.job->get_time_to_spend() > job.job->get_end_before();
        }

        if (current_failed_jobs == 0)
        {
            best_completed_jobs = completed_jobs;
            break;
        }
        if (current_failed_jobs > best_failed_jobs)
        {
            best_completed_jobs = completed_jobs;
            break;
        }
        if (current_failed_jobs == best_failed_jobs)
        {
            current_equal_failed++;
            // look_ahead_time acts as a magic
            // number of repeats before we give
            // up on trying to find a better
            // solution. This may be rewritten
            // to use something like current_equal_max.
            if (current_equal_failed > look_ahead_time)
            {
                best_completed_jobs = completed_jobs;
                break;
            }
        }
        current_equal_failed = 0;
        best_failed_jobs = current_failed_jobs;
        for (auto job : completed_jobs)
        {
            job.job->undone();
            job.worker->undone();
            if (job.start + job.job->get_time_to_spend() <= job.job->get_end_before()) continue;
            auto old_coefficient = job.job->get_preference_coefficient();
            job.job->set_preference_coefficient(old_coefficient + 1);
        }

        pending_jobs = input;

        qDebug() << "current failed job count:" << best_failed_jobs << "with" << completed_jobs.size() << "completed";
    }
    if (best_failed_jobs != __INT_MAX__)
        completed_jobs = best_completed_jobs;
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

