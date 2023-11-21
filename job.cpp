#include "job.h"

Job::Job(int renewable, int non_renewable, int want_time_to_spend) :
    want_renewable(renewable), want_non_renewable(non_renewable), time_to_spend(want_time_to_spend)
{

}

int Job::get_time_to_spend() const
{
    return time_to_spend;
}

int Job::get_want_renewable() const
{
    return want_renewable;
}

int Job::get_want_non_renewable() const
{
    return want_non_renewable;
}

void Job::done()
{
    _done = true;
}

void Job::undone()
{
    _done = false;
}

bool Job::is_done() const
{
    return _done;
}

bool Job::check_predecessors()
{
    if (_predecessors_done) return true;
    for (int i = 0; i < predecessors.size(); i++)
    {
        if (!predecessors[i]->is_done()) return false;
    }
    _predecessors_done = true;
    return true;
}

void Job::set_predecessors(std::vector<Job*> new_predecessors)
{
    predecessors = new_predecessors;
}

bool Job::is_predecessor(Job *job)
{
    for (int i = 0; i < predecessors.size(); i++)
    {
        if (predecessors[i] == job) return true;
    }
    return false;
}

int Job::get_critical_time() const
{
    if (!critical_time_exists()) throw std::exception();
    return _critical_time;
}

bool Job::critical_time_exists() const
{
    return _critical_time != -1;
}

void Job::set_critical_time(int time)
{
    _critical_time = time;
}

void Job::reset_critical_time()
{
    _critical_time = -1;
}
