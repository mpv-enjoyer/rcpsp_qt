#include "job.h"
#include <stdexcept>

Job::Job(int renewable, int non_renewable, std::vector<OccupancyPair> occupancy) :
    want_renewable(renewable), want_non_renewable(non_renewable)
{
    this->occupancy = occupancy;
    time_to_spend = 0;
    for (int i = 0; i < occupancy.size(); i++)
    {
        avg_occupancy_buffered += occupancy[i].time * occupancy[i].occupancy;
        time_to_spend += occupancy[i].time;
    }
    avg_occupancy_buffered /= time_to_spend;
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
    for (int i = 0; i < ancestors.size(); i++)
    {
        ancestors[i]->decrement_predecessors_count();
    }
}

void Job::undone()
{
    _done = false;
    for (int i = 0; i < ancestors.size(); i++)
    {
        ancestors[i]->increment_predecessors_count();
    }
}

bool Job::is_done() const
{
    return _done;
}

bool Job::check_predecessors()
{
    return predecessors_count == 0;
    /*
    if (_predecessors_done) return true;
    for (int i = 0; i < predecessors.size(); i++)
    {
        if (!predecessors[i]->is_done()) return false;
    }
    _predecessors_done = true;
    return true;
    */
}

void Job::increment_predecessors_count()
{
    predecessors_count++;
}

void Job::decrement_predecessors_count()
{
    if (predecessors_count <= 0) throw std::invalid_argument("Tried to decrement predecessor count that is zero");
    predecessors_count--;
}

void Job::set_ancestors(std::vector<Job*> new_ancestors)
{
    ancestors = new_ancestors;
    for (int i = 0; i < new_ancestors.size(); i++)
    {
        new_ancestors[i]->increment_predecessors_count();
    }
}

std::vector<Job*>* Job::get_ancestors()
{
    return &ancestors;
}
/*bool Job::is_predecessor(Job *job)
void Job::reset()
{
    _done = false;
}
{
    for (int i = 0; i < predecessors.size(); i++)
    {
        if (predecessors[i] == job) return true;
    }
    return false;
}*/

int Job::get_critical_time() const
{
    if (!critical_time_exists()) throw std::invalid_argument("Critical time doesn't exist");
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

void Job::set_start_after(int time)
{
    begin_min = time;
}

int Job::get_start_after() const
{
    return begin_min;
}

void Job::set_end_before(int time)
{
    end_max = time;
}

int Job::get_end_before() const
{
    return end_max;
}

void Job::set_global_id(int id)
{
    global_id = id;
}

int Job::get_global_id()
{
    return global_id;
}

std::vector<OccupancyPair> Job::get_occupancy() const
{
    return occupancy;
}

double Job::get_average_occupancy() const
{
    return avg_occupancy_buffered;
}

int Job::get_preference_coefficient() const
{
    return preference_coefficient;
}

void Job::set_preference_coefficient(int coefficient)
{
    preference_coefficient = coefficient;
}

int Job::get_global_group_id() const
{
    if (global_group_id == -1) throw std::invalid_argument("Global group id not set");
    return global_group_id;
}

void Job::set_global_group_id(int id)
{
    if (global_group_id != -1) throw std::invalid_argument("Global group id already set");
    global_group_id = id;
}

bool Job::is_failed(int start_time) const
{
    return (start_time + get_time_to_spend()) > get_end_before();
}

std::optional<std::pair<int, int>> Job::get_preferred() const
{
    if (preferred)
    {
        auto[start_after, end_before] = *preferred;
        if (start_after + get_time_to_spend() > end_before) throw std::invalid_argument("preferred_start_after + time_to_spend > preferred_end_before");
        if (start_after < get_start_after()) throw std::invalid_argument("preferred_begin < start_after");
        if (end_before > get_end_before()) throw std::invalid_argument("preferred_end > end_before");
    }
    return preferred;
}

void Job::set_preferred(int start_after, int end_before)
{
    preferred = std::pair<int, int>{start_after, end_before};
}

#include <fstream>

//std::ofstream GLOBAL_LOGGING_FILE("GLOBAL_LOG.LOG");

void GLOBAL_LOG(std::string log)
{
    //GLOBAL_LOGGING_FILE << log;
}
