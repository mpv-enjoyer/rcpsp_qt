#include "worker.h"

Worker::Worker(Plan want_plan) :
    plan(want_plan)
{
    current_job = {-1, nullptr};
}

void Worker::assign(Job *job)
{
    update();
    if (!is_free())
    {
        throw std::exception();
    }
    if (plan.get_time_until_rest(*clock) < job->get_time_to_spend())
    {
        throw std::exception();
    }
    current_job = {*clock, job};
}

void Worker::update()
{
    if (current_job.job == nullptr) return;
    if (*clock >= current_job.time_start + current_job.job->get_time_to_spend())
    {
        current_job.job->done();
        current_job.job = nullptr;
        current_job.time_start = -1;
    }
}

bool Worker::is_free()
{
    update();
    return current_job.job == nullptr && plan.is_ready(*clock);
}

const Job* Worker::get_job()
{
    update();
    return current_job.job;
}

int Worker::left_before_free()
{
    update();
    int current_time = *clock;
    bool ready = plan.is_ready(current_time);
    if (!ready)
    {
        return plan.get_time_until_ready(current_time);
    }
    if (is_free())
    {
        return 0;
    }
    int job_ends_at = current_job.time_start + current_job.job->get_time_to_spend();
    return job_ends_at + plan.get_time_until_ready(job_ends_at);
}

const Plan Worker::get_plan()
{
    update();
    return plan;
}

void Worker::set_clock(int* new_clock)
{
    clock = new_clock;
}
