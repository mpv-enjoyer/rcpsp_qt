#ifndef PENDINGFRONTS_H
#define PENDINGFRONTS_H

#include "algorithm.h"

class PendingFronts
{
    friend class basic; //TESTS
    int* _current_time;
    AssignedJobs* next;
    const Preference _preference;
    int last_loop_check_begin = -1;
    const int _look_ahead_time;
    const AlgorithmWeights _weights;
public:
    struct Data
    {
        int time;
        std::vector<JobPair> job_pairs;
    };
    PendingFronts(int* current_time, AssignedJobs* next, Preference preference, int look_ahead_time, AlgorithmWeights weights);
    void add(int front_time, JobPair job_pair);
    void add(int front_time); // Query updates for everyone except PendingFronts
    
    bool tick(AlgorithmDataForWeights data_for_weights);
    bool update_time_to_front();
private:
    std::vector<Data> _data;
    struct SearchResult
    {
        bool found;
        int pos;
    };
    SearchResult binarySearch(const Data &x);
    void sort_current_front(Data &current_front, AlgorithmDataForWeights data_for_weights);
    void sort_current_front_by_preference(Data &current_front);
    void apply_preference_coefficient_to_current_front(Data &current_front);
};

#endif // PENDINGFRONTS_H
