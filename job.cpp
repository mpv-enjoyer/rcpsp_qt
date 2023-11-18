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

AssignedJob::AssignedJob(Job& to_assign, int time_begin) : Job(to_assign)
{
    begin_at = time_begin;
}

int AssignedJob::get_begin_at() const
{
    return begin_at;
}

VisualJob::VisualJob(AssignedJob& assigned) : AssignedJob(assigned)
{
    int R = QRandomGenerator::global()->bounded(255);
    int G = QRandomGenerator::global()->bounded(255);
    int B = QRandomGenerator::global()->bounded(255);
    color = QColor(R, G, B);
}

QColor VisualJob::get_color() const
{
    return color;
}


