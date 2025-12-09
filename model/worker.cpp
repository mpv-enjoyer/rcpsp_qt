#include "worker.h"
#include <stdexcept>

Worker::Worker(Plan want_plan) :
    plan(want_plan)
{

}

void Worker::assign(Job *job)
{
    update();
    if (job->get_start_after() > shared.clock())
    {
        throw std::invalid_argument("Assign a job before the start_after");
    }
    if (!is_free(job->get_occupancy()))
    {
        throw std::invalid_argument("Worker is already busy enough");
    }
    if (plan.get_time_until_rest(shared.clock()) < job->get_time_to_spend())
    {
        throw std::invalid_argument("Not enough time until rest");
    }
    if (preserved_until > shared.clock())
    {
        throw std::invalid_argument("Attempted to assign preserved worker");
    }
    int job_ends_at = shared.clock() + job->get_time_to_spend();
    int time_ready = job_ends_at;
    current_jobs.push_back({shared.clock(), job, time_ready});
    //forgot to sort current_jobs by time_ready?
}

void Worker::update()
{
    if (preserved_until != -1 && preserved_until <= shared.clock())
    {
        preserved_until = -1;
    }
    for (int i = 0; i < current_jobs.size(); i++)
    {
        if (shared.clock() >= current_jobs[i].will_be_free_at)
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
        int time_spent = shared.clock() - job.time_start;
        for (int i = 0; i < job_occupancy.size(); i++)
        {
            time_spent -= job_occupancy[i].time;
            if (time_spent <= 0)
            {
                output += job_occupancy[i].occupancy;
                break;
            }
        }
        if (time_spent > 0) throw std::invalid_argument("Worker is assigned to a job that he already completed, update() needed");
    }
    if (output > 1.0f) throw std::invalid_argument("Worker is busy for >100%");
    return output;
}

void Worker::set_shared(Shared shared)
{
    this->shared = shared;
}

bool Worker::is_free(std::vector<OccupancyPair> want_occupancy, int fetch_time)
{
    if (fetch_time == -1) fetch_time = shared.clock();
    update();
    if (preserved_until > fetch_time) return false;
    //if (!plan.is_ready(fetch_time)) return false;
    int time_left = plan.get_time_until_rest(fetch_time);
    int time_want_to_use = 0;
    for (auto occupancy : want_occupancy)
    {
        time_want_to_use += occupancy.time;
    }
    if (time_left < time_want_to_use) return false;

    struct OccupancyWithIndex
    {
        std::vector<OccupancyPair> data;
        std::size_t i;
    };

    std::vector<OccupancyWithIndex> all_occupancies;
    all_occupancies.reserve(current_jobs.size());
    for (auto job : current_jobs)
    {
        auto current = job.job->get_occupancy();
        int difference = fetch_time - job.time_start;
        std::size_t i = 0;
        for (; i < current.size(); i++)
        {
            if (current[i].time - difference <= 0)
            {
                difference -= current[i].time;
                //current.erase(current.begin());
                //i--;
            }
            else
            {
                current[i].time -= difference;
                break;
            }
        }
        if (current.size() != 0) all_occupancies.push_back(OccupancyWithIndex{ .data = std::vector<OccupancyPair>(current.begin() + i, current.end()), .i = 0 });
    }
    std::size_t want_occupancy_i = 0;
    while (want_occupancy_i < want_occupancy.size())
    {
        float occupancy = 0.0f;
        occupancy += want_occupancy[want_occupancy_i].occupancy;
        want_occupancy[want_occupancy_i].time -= 1;
        if (want_occupancy[want_occupancy_i].time <= 0) want_occupancy_i++;
        for (int i = 0; i < all_occupancies.size(); i++)
        {
            auto& local_occupancy = all_occupancies[i];
            if (local_occupancy.i >= local_occupancy.data.size()) continue;
            occupancy += local_occupancy.data[local_occupancy.i].occupancy;
            local_occupancy.data[local_occupancy.i].time -= 1;
            if (local_occupancy.data[local_occupancy.i].time <= 0)
            {
                local_occupancy.i++;

                //all_occupancies.erase(all_occupancies[i].begin());
            }
        }
        if (occupancy > 1.0f) return false;
    }
    return true;
}

void Worker::undone()
{
    preserved_until = -1;
    current_jobs = std::vector<CurrentJob>();
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

int Worker::can_be_placed_after(std::vector<OccupancyPair> occupancy)
{
    update();
    /*int current_time = *clock;
    bool ready = plan.is_ready(current_time);
    if (!ready)
    {
        return plan.get_time_until_ready(current_time);
    }
    if (is_free(occupancy))
    {
        return 0;
    }*/
    int time;
    if (preserved_until != -1) time = preserved_until;
    else time = shared.clock();
    while (!is_free(occupancy, time))
    {
        time++;
        // this might hang the program
    }
    return time - shared.clock();
}

void Worker::preserve(int interval)
{
    update();
    //if (!is_free(1.0f)) throw std::exception();
    //idk what this line did
    if (preserved_until != -1) throw std::invalid_argument("Tried to preserve an already preserved worker");
    if (interval <= 0) return;
    preserved_until = shared.clock() + interval;
}

const Plan Worker::get_plan() const
{
    return plan;
}

int Worker::is_preserved() const
{
    return preserved_until != -1;
}
