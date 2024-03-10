#include "worker.h"

Worker::Worker(Plan want_plan) :
    plan(want_plan)
{

}

void Worker::assign(Job *job)
{
    update();
    if (!is_free(job->get_occupancy()))
    {
        throw std::exception();
    }
    if (plan.get_time_until_rest(*clock) < job->get_time_to_spend())
    {
        throw std::exception();
    }
    int job_ends_at = *clock + job->get_time_to_spend();
    int time_ready = job_ends_at + plan.get_time_until_ready(job_ends_at);
    current_jobs.push_back({*clock, job, time_ready});
    current_occupancy += job->get_occupancy();
}

void Worker::update()
{
    for (int i = 0; i < current_jobs.size(); i++)
    {
        if (*clock >= current_jobs[i].will_be_free_at)
        {
            current_jobs[i].job->done();
            current_occupancy -= current_jobs[i].job->get_occupancy();
            current_jobs.erase(current_jobs.begin() + i);
        }
    }
}

bool Worker::is_free(float occupancy)
{
    update();
    if (!plan.is_ready(*clock)) return false;
    return 1.0f - current_occupancy >= occupancy;
}

const Job* Worker::get_job(int index)
{
    update();
    return current_jobs[index].job;
}

int Worker::get_job_count()
{
    update();
    return current_jobs.size();
}

int Worker::will_be_free_after(float occupancy)
{
    update();
    int current_time = *clock;
    bool ready = plan.is_ready(current_time);
    if (!ready)
    {
        return plan.get_time_until_ready(current_time);// + current_time; THIS DOESN'T MAKE SENSE
    }
    if (is_free(occupancy))
    {
        return 0;
    }
    float need_occupancy = occupancy + current_occupancy - 1.0f;
    for (int i = 0; i < current_jobs.size(); i++)
    {
        need_occupancy -= current_jobs[i].job->get_occupancy();
        if (need_occupancy <= 0) return current_jobs[i].will_be_free_at - current_time;
    }
    throw std::exception();
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
