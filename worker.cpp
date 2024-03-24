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
    if (preserved_until > *clock)
    {
        throw std::exception();
    }
    int job_ends_at = *clock + job->get_time_to_spend();
    int time_ready = job_ends_at + plan.get_time_until_ready(job_ends_at);
    current_jobs.push_back({*clock, job, time_ready});
    //forgot to sort current_jobs by time_ready?
}

void Worker::update()
{
    if (preserved_until != -1 && preserved_until <= *clock)
    {
        preserved_until = -1;
    }
    for (int i = 0; i < current_jobs.size(); i++)
    {
        if (*clock >= current_jobs[i].will_be_free_at)
        {
            current_jobs[i].job->done();
            current_jobs.erase(current_jobs.begin() + i);
        }
    }
}

float Worker::current_occupancy()
{
    float output = 0.0f;
    for (auto job : current_jobs)
    {
        auto job_occupancy = job.job->get_occupancy();
        int time_spent = *clock - job.time_start;
        for (int i = 0; i < job_occupancy.size(); i++)
        {
            time_spent -= job_occupancy[i].time;
            if (time_spent <= 0)
            {
                output += job_occupancy[i].occupancy;
                break;
            }
        }
        if (time_spent > 0) throw std::exception();
    }
    if (output > 1.0f) throw std::exception();
    return output;
}

bool Worker::is_free(std::vector<OccupancyPair> want_occupancy, int fetch_time)
{
    if (fetch_time == -1) fetch_time = *clock;
    update();
    if (preserved_until != -1) return false;
    if (!plan.is_ready(fetch_time)) return false;

    std::vector<std::vector<OccupancyPair>> all_occupancies;
    for (auto job : current_jobs)
    {
        auto current = job.job->get_occupancy();
        int difference = fetch_time - job.time_start;
        for (int i = 0; i < current.size(); i++)
        {
            if (current[i].time - difference <= 0)
            {
                difference -= current[i].time;
                current.erase(current.begin());
            }
            else
            {
                current[i].time -= difference;
                break;
            }
        }
    }
    while (want_occupancy.size() != 0)
    {
        float occupancy = 0.0f;
        occupancy += want_occupancy[0].occupancy;
        want_occupancy[0].time -= 1;
        if (want_occupancy[0].time <= 0) want_occupancy.erase(want_occupancy.begin());
        for (int i = 0; i < all_occupancies.size(); i++)
        {
            occupancy += all_occupancies[i][0].occupancy;
            all_occupancies[i][0].time -= 1;
            if (all_occupancies[i][0].time <= 0) all_occupancies.erase(all_occupancies.begin());
        }
        if (occupancy > 1.0f) return false;
    }
    return false;
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

int Worker::will_be_free_after(std::vector<OccupancyPair> occupancy)
{
    update();
    int current_time = *clock;
    bool ready = plan.is_ready(current_time);
    if (!ready)
    {
        return plan.get_time_until_ready(current_time);
    }
    if (is_free(occupancy))
    {
        return 0;
    }
    int time;
    if (preserved_until != -1) time = preserved_until;
    else time = *clock;
    while (!is_free(occupancy, time))
    {
        time++;
        // this might hang the program
    }
    return time - current_time;
}

void Worker::preserve(int interval)
{
    update();
    //if (!is_free(1.0f)) throw std::exception();
    //idk what this line did
    if (preserved_until != -1) throw std::exception();
    if (interval <= 0) return;
    preserved_until = *clock + interval;
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
