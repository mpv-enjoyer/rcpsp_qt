#ifndef PENDINGFRONTS_H
#define PENDINGFRONTS_H

#include "algorithm.h"

class PendingFronts
{
    int* _current_time;
    AssignedJobs* next;
    const Preference _preference;
    int last_loop_check_begin = -1;
    const int _longest_plan_loop;
public:
    struct Data
    {
        int time;
        std::vector<PendingJobs::Data> job_pairs;
    };
    PendingFronts(int* current_time, AssignedJobs* next, Preference preference, int longest_plan_loop);
    void add(int front_time, PendingJobs::Data job_pairs);
    void add(int front_time); // Query updates for everyone except PendingFronts
    bool tick();
private:
    std::vector<Data> _data;
    SearchResult binarySearch(const Data &x);
    void sort_current_front(Data &current_front);
    void apply_preference_coefficient_to_current_front(Data &current_front);
};

#endif // PENDINGFRONTS_H
