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
        int current_nearest = -1;
        if (workers[i]->get_job() == nullptr)
        {
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(*current_time, job_time);
        }
        else
        {
            int will_end_current_work = workers[i]->left_before_free() + *current_time;
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(will_end_current_work, job_time);
        }
        if (current_nearest == -1) continue;
        if (min == -1) min = current_nearest;
        if (min > current_nearest) min = current_nearest;
    }
    return min;
}

Worker* WorkerGroup::assign(Job *job)
{
    for (int i = 0; i < workers.size(); i++)
    {
        if (workers[i]->is_free())
        {
            workers[i]->assign(job);
            return workers[i];
        }
    }
}
