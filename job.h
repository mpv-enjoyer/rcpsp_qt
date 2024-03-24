#ifndef JOB_H
#define JOB_H

#include <vector>
#include <QRandomGenerator>

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
    int* _following_time_to_spend = nullptr;
    std::vector<Job*> ancestors;
    int predecessors_count = 0;
    int begin_min = 0;
    int end_max = 0;
    int want_renewable;
    int want_non_renewable;
    int time_to_spend;
    std::vector<OccupancyPair> occupancy;
public:
    Job(int renewable, int non_renewable, std::vector<OccupancyPair> occupancy);
    int get_time_to_spend() const;
    int get_want_renewable() const;
    int get_want_non_renewable() const;
    bool is_done() const;
    void done();
    void undone();
    bool check_predecessors();
    void set_ancestors(std::vector<Job*> new_ancestors);
    std::vector<Job*>* get_ancestors();
    void increment_predecessors_count();
    void decrement_predecessors_count();
    //bool is_predecessor(Job* job);
    int get_critical_time() const;
    bool critical_time_exists() const;
    void set_critical_time(int time);
    void reset_critical_time();
    void set_start_after(int time);
    int get_start_after() const;
    void set_end_before(int time);
    int get_end_before() const;
    std::vector<OccupancyPair> get_occupancy() const;
};

#endif // JOB_H
