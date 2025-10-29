#include "job.h"
#include <stdexcept>

Job::Job(std::vector<OccupancyPair> occupancy, std::vector<Job *> successors, int global_id)
{
    this->occupancy = occupancy;
    time_to_spend = 0;
    for (int i = 0; i < occupancy.size(); i++)
    {
        avg_occupancy_buffered += occupancy[i].time * occupancy[i].occupancy;
        time_to_spend += occupancy[i].time;
    }
    avg_occupancy_buffered /= time_to_spend;
    set_global_id(global_id);
    set_successors(successors);
}

void Job::init_group(int start_after_time, int end_before_time, int global_group_id)
{
    set_start_after(start_after_time);
    set_end_before(end_before_time);
    set_global_group_id(global_group_id);
}

int Job::get_time_to_spend() const
{
    return time_to_spend;
}

void Job::done()
{
    _done = true;
    for (int i = 0; i < successors.size(); i++)
    {
        successors[i]->decrement_predecessors_count();
    }
}

void Job::undone()
{
    _done = false;
    for (int i = 0; i < successors.size(); i++)
    {
        successors[i]->increment_predecessors_count();
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

void Job::set_successors(std::vector<Job*> new_ancestors)
{
    successors = new_ancestors;
    for (int i = 0; i < new_ancestors.size(); i++)
    {
        new_ancestors[i]->increment_predecessors_count();
    }
}

std::vector<Job*>* Job::get_successors()
{
    return &successors;
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

int Job::init_critical_time()
{
    if (critical_time_exists())
    {
        return get_critical_time();
    }
    int result = get_end_before();
    std::vector<Job*>* current_ancestors = get_successors();
    for (int j = 0; j < current_ancestors->size(); j++)
    {
        int internal_result = current_ancestors->at(j)->init_critical_time();
        result = internal_result < result ? internal_result : result;
    }
    double output = result - get_time_to_spend();
    if (output == -1) output = 0;
    set_critical_time(output);
    return result - get_time_to_spend();
}

void Job::set_critical_time(int time)
{
    _critical_time = time;
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

// Slow
float Job::get_occupancy_at(int time) const
{
    if (time >= get_time_to_spend()) throw std::invalid_argument("get_occupancy_at invalid time");
    int i = 0;
    for (int t = 0; i < occupancy.size() && t + occupancy[i].time < time; t += occupancy[i].time, i++) { }
    return occupancy[i].occupancy;
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

#include <fstream>

//std::ofstream GLOBAL_LOGGING_FILE("GLOBAL_LOG.LOG");

void GLOBAL_LOG(std::string log)
{
    //GLOBAL_LOGGING_FILE << log;
}
