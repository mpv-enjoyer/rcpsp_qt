#include "workergroup.h"

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
            int will_end_current_work = workers[i]->will_be_free_at();// + *current_time;
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(will_end_current_work, job_time);
            current_nearest += (will_end_current_work - *current_time);
        }
        if (current_nearest == -1) continue;
        if (min == -1) min = current_nearest;
        if (min > current_nearest) min = current_nearest;
    }
    return min;
}

AssignedWorker WorkerGroup::assign(Job *job)
{
    for (int i = 0; i < workers.size(); i++)
    {
        workers[i]->update();
        if (workers[i]->is_free())
        {
            workers[i]->assign(job);
            return {workers[i], i};
        }
    }
    throw std::exception();
}
