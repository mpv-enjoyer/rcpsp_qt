#ifndef JOB_H
#define JOB_H

#include <vector>
#include <QRandomGenerator>
#include <QColor>

class Job
{
private:
    bool _done = false;
    int want_renewable;
    int want_non_renewable;
    int time_to_spend;
    std::vector<Job*> predecessors;
public:
    Job(int renewable, int non_renewable, int time_to_spend);
    int get_time_to_spend() const;
    int get_want_renewable() const;
    int get_want_non_renewable() const;
    bool is_done() const;
    void done();
    bool check_predecessors() const;
};

//TODO: AssignedJob not needed.
//This info will be stored inside Worker
class AssignedJob : public Job
{
private:
    int begin_at;
public:
    AssignedJob(Job& to_assign, int time_begin);
    int get_begin_at() const;
};

class VisualJob : public AssignedJob
{
private:
    QColor color;
public:
    VisualJob(AssignedJob& assigned);
    QColor get_color() const;
};

#endif // JOB_H
