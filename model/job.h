#ifndef JOB_H
#define JOB_H

#include <vector>

#include <string>
void GLOBAL_LOG(std::string log);

struct OccupancyPair
{
    int time;
    float occupancy;
};

class Job
{
private:
    bool _done = false;
    int _critical_time = -1;
    std::vector<Job*> successors;
    int predecessors_count = 0;
    int begin_min = 0;
    int end_max = 0;
    int time_to_spend;
    int global_id = 0;
    std::vector<OccupancyPair> occupancy;
    int preference_coefficient = 0;
    double avg_occupancy_buffered = 0;
    int global_group_id = -1;
    void set_global_id(int id);
    void set_successors(std::vector<Job*> new_ancestors);
    void set_start_after(int time);
    void set_end_before(int time);
    void set_global_group_id(int id);
    void set_critical_time(int time);
public:
    Job(std::vector<OccupancyPair> occupancy, std::vector<Job*> successors, int global_id);
    void init_group(int start_after_time, int end_before_time, int global_group_id);
    int get_time_to_spend() const;
    bool is_done() const;
    void done();
    void undone();
    bool check_predecessors();
    std::vector<Job*>* get_successors();
    void increment_predecessors_count();
    void decrement_predecessors_count();
    int get_critical_time() const;
    bool critical_time_exists() const;
    int init_critical_time();
    int get_start_after() const;
    int get_end_before() const;
    int get_global_id();
    std::vector<OccupancyPair> get_occupancy() const;
    double get_average_occupancy() const;
    int get_preference_coefficient() const;
    void set_preference_coefficient(int coefficient);
    int get_global_group_id() const;
    bool is_failed(int start_time) const;
};

#endif // JOB_H
