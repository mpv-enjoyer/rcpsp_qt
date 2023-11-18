#include "workergroup.h"

WorkerGroup::WorkerGroup(int* time) : current_time(time)
{

}

void WorkerGroup::add_worker(Worker* new_worker)
{
    workers.push_back(new_worker);
}

int WorkerGroup::get_earliest_placement_time(Job *job)
{
    int min = -1;
    int job_time = job->get_time_to_spend();
    for (int i = 0; i < workers.size(); i++)
    {
        int current_nearest = workers[i]->get_plan().get_time_nearest_possible(*current_time, job_time);
        if (current_nearest == -1) continue;
        if (min == -1) min = current_nearest;
        if (min > current_nearest) min = current_nearest;
    }
    return min;
}

void WorkerGroup::assign(Job *job)
{
    for (int i = 0; i < workers.size(); i++)
    {
        if (workers[i]->is_free())
        {
            workers[i]->assign(job);
            return;
        }
    }
}
