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
        if (workers[i]->get_job_count() == 0)
        {
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(*current_time, job_time);
        }
        else
        {
            int will_be_free_at = workers[i]->will_be_free_after(job->get_occupancy()) + *current_time;// + *current_time;
            current_nearest = workers[i]->get_plan().get_time_nearest_possible(will_be_free_at, job_time);
            if (current_nearest == -1) continue;
            current_nearest = current_nearest + will_be_free_at - *current_time;
            //current_nearest += (will_end_current_work - *current_time);
            if (current_nearest == -2) throw std::exception();
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
        if (workers[i]->is_free(job->get_occupancy()))
        {
            workers[i]->assign(job);
            return {workers[i], i};
        }
    }
    throw std::exception();
}

int WorkerGroup::get_size()
{
    return workers.size();
}
