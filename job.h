#ifndef JOB_H
#define JOB_H

#include <vector>
#include <QRandomGenerator>
#include <QColor>

class Job
{
private:
    bool _done = false;
    int _critical_time = -1;
    int* _following_time_to_spend = nullptr;
    int want_renewable;
    int want_non_renewable;
    int time_to_spend;
    std::vector<Job*> predecessors;
    bool _predecessors_done = false;
public:
    Job(int renewable, int non_renewable, int time_to_spend);
    int get_time_to_spend() const;
    int get_want_renewable() const;
    int get_want_non_renewable() const;
    bool is_done() const;
    void done();
    void undone();
    bool check_predecessors();
    void set_predecessors(std::vector<Job*> new_predecessors);
    bool is_predecessor(Job* job);
    int get_critical_time() const;
    bool critical_time_exists() const;
    void set_critical_time(int time);
    void reset_critical_time();
};

#endif // JOB_H
