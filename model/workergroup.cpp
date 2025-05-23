#include "workergroup.h"
#include <QDebug>

WorkerGroup::WorkerGroup()
{

}

void WorkerGroup::set_clock(int* clock)
{
    current_time = clock;
    for (int i = 0; i < workers.size(); i++)
    {
        workers[i]->set_clock(clock);
        workers[i]->update();
    }
}

void WorkerGroup::set_global_id(int id)
{
    global_id = id;
}

void WorkerGroup::add_worker(Worker* new_worker)
{
    workers.push_back(new_worker);
}

Placement WorkerGroup::get_earliest_placement_time(Job *job)
{
    Placement output = {nullptr, -1};
    int job_time = job->get_time_to_spend();
    int lookup_time = *current_time >= job->get_start_after() ? *current_time : job->get_start_after();
    for (int i = 0; i < workers.size(); i++)
    {
        int current_nearest = -1;
        if (workers[i]->get_job_count() == 0 && !workers[i]->is_preserved())
        {
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(lookup_time, job_time);
            if (current_nearest == -1) continue;
            if (lookup_time != *current_time)
                current_nearest += job->get_start_after() - *current_time;
        }
        else
        {
            int will_be_free_at = workers[i]->can_be_placed_after(job->get_occupancy()) + lookup_time;
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(will_be_free_at, job_time);
            if (current_nearest == -1) continue;
            current_nearest = current_nearest + will_be_free_at - lookup_time;
        }
        if (current_nearest == -1) continue;
        if (!output.worker || output.time_before > current_nearest)
        {
            output.worker = workers[i];
            output.time_before = current_nearest;
        }
    }
    if (lookup_time + output.time_before < job->get_start_after())
    {
        throw std::invalid_argument("get_earliest_placement_time wanted to return invalid time_before value");
    }
    return output;
}

AssignedWorker WorkerGroup::assign(Job *job)
{
    for (int i = 0; i < workers.size(); i++)
    {
        workers[i]->update();
        if (workers[i]->is_free(job->get_occupancy()))
        {
            workers[i]->assign(job);
            return {workers[i], i};
        }
    }
    throw std::invalid_argument("Assign to a busy enough group");
    return {nullptr, -1};
}

int WorkerGroup::get_size()
{
    return workers.size();
}

int WorkerGroup::get_global_id()
{
    return global_id;
}

bool WorkerGroup::check_if_job_is_possible(const Job *job)
{
    for (const auto worker : workers)
    {
        auto plan_elements = worker->get_plan().get_elements();
        for (auto plan_element : plan_elements)
        {
            if (plan_element.work >= job->get_time_to_spend()) return true;
        }
    }
    return false;
}

const Worker *WorkerGroup::get_worker(int id) const
{
    return workers.at(id);
}
