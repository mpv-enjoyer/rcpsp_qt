#include "worker.h"

Worker::Worker(Plan want_plan) :
    plan(want_plan)
{
    current_job = {-1, nullptr, -1};
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
    int job_ends_at = *clock + job->get_time_to_spend();
    int time_ready = job_ends_at + plan.get_time_until_ready(job_ends_at);
    current_job = {*clock, job, time_ready};
}

void Worker::update()
{
    if (current_job.job == nullptr) return;
    if (*clock >= current_job.time_start + current_job.job->get_time_to_spend())
    {
        current_job.job->done();
        current_job.job = nullptr;
        current_job.time_start = -1;
        current_job.will_be_free_at = -1;
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

int Worker::will_be_free_at()
{
    update();
    int current_time = *clock;
    bool ready = plan.is_ready(current_time);
    if (!ready)
    {
        return plan.get_time_until_ready(current_time);// + current_time; THIS DOESN'T MAKE SENSE
    }
    if (is_free())
    {
        return 0;
    }
    return current_job.will_be_free_at;
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
