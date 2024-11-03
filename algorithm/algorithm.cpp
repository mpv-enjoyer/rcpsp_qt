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
        JobPair new_pair = {start, end, jobs->get_job(i), { workers }, static_cast<int>(_pending_jobs.size())};
        _pending_jobs.push_back(new_pair);
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

bool compare_result(ResultPair& lhs, ResultPair& rhs)
{
    return (lhs.job->get_global_id() > rhs.job->get_global_id());
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
    int best_failed_jobs = __INT_MAX__;
    std::vector<ResultPair> best_completed_jobs;
    int current_failed_jobs = 0;
    int current_equal_failed = 0;
    std::vector<ResultPair> current_completed_jobs;

    AlgorithmDataForWeights for_weights = { 0 };
    for (auto job : _pending_jobs)
    {
        if (for_weights.max_critical_time < job.job->get_critical_time()) for_weights.max_critical_time = job.job->get_critical_time();
    }

    while (true)
    {
        int current_time = 0;
        CompletedJobs completed_jobs;
        AssignedJobs assigned_jobs = AssignedJobs(&current_time, &completed_jobs);
        PendingFronts pending_fronts = PendingFronts(&current_time, &assigned_jobs, preference, longest_plan_loop, _weights);
        PendingJobs pending_jobs = PendingJobs(&current_time, &pending_fronts, look_ahead_time, _pending_jobs);
        pending_fronts.update_time_to_front();
        bool must_continue = true;
        while (must_continue)
        {
            for_weights.job_count_not_assigned = pending_jobs.data_size();
            for_weights.job_count_overall = _pending_jobs.size();

            must_continue = false;
            must_continue |= pending_jobs.tick();
            must_continue |= pending_fronts.tick(for_weights);
            must_continue |= assigned_jobs.tick();
        }

        current_completed_jobs = completed_jobs.result();
        if (current_completed_jobs.size() != _pending_jobs.size()) throw std::invalid_argument("Lost some jobs"); // Lost some jobs

        current_failed_jobs = completed_jobs.failed_count();
        if (current_failed_jobs == 0 || DO_NOT_REPEAT)
        {
            best_failed_jobs = current_failed_jobs;
            best_completed_jobs = current_completed_jobs;
            break;
        }
        if (current_failed_jobs > best_failed_jobs)
        {
            // Don't set to a worse value
            break;
        }
        if (current_failed_jobs == best_failed_jobs)
        {
            current_equal_failed++;
            if (current_equal_failed > CURRENT_EQUAL_MAX)
            {
                best_completed_jobs = current_completed_jobs;
                break;
            }
        }
        current_equal_failed = 0;
        best_failed_jobs = current_failed_jobs;
        completed_jobs.prepare_for_next_iteration();
        qDebug() << "current failed job count:" << best_failed_jobs << "with" << completed_jobs.result().size() << "completed";
    }
    if (best_failed_jobs != __INT_MAX__) // Does it ever occur?
        _completed_jobs = best_completed_jobs;
    std::sort(_completed_jobs.begin(), _completed_jobs.end(), compare_result);
    for (int i = 0; i < _completed_jobs.size(); i++)
    {
        _completed_jobs[i].job_id = _completed_jobs[i].job->get_global_id();
    }
    qDebug() << "final failed job count:" << best_failed_jobs << "with" << _completed_jobs.size() << "completed";
}

std::vector<ResultPair> Algorithm::get_completed()
{
    return _completed_jobs;
}
