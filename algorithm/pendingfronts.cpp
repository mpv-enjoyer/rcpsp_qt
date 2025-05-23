#include "pendingfronts.h"
#include "assignedjobs.h"

PendingFronts::SearchResult PendingFronts::binarySearch(const Data& x)
{
    auto& arr = _data;
    int r = arr.size() - 1;
    int l = 0;
    while (l <= r) {
        int m = l + (r - l) / 2;

        // Check if x is present at mid
        if (arr[m].time == x.time)
            return { true, m };

        // If x greater, ignore left half
        if (arr[m].time < x.time)
            l = m + 1;

        // If x is smaller, ignore right half
        else
            r = m - 1;
    }

    // If we reach here, then element was not present
    return { false, l };
}

bool compare_SPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() < rhs.job->get_time_to_spend());
}

bool compare_LPT(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_time_to_spend() > rhs.job->get_time_to_spend());
}

bool compare_EST(JobPair lhs, JobPair rhs)
{
    return (lhs.job->get_critical_time() < rhs.job->get_critical_time());
}

bool compare_weights(JobPair lhs, JobPair rhs)
{
    return (lhs.current_preference > rhs.current_preference);
    // BREAKING CHANGE!!! All previous weights MUST BE REVERSED
}

void PendingFronts::sort_current_front(Data& current_front, AlgorithmDataForWeights data_for_weights)
{
    double job_count_not_assigned = data_for_weights.job_count_not_assigned;
    for (const auto& front : _data)
    {
        job_count_not_assigned += front.job_pairs.size();
    }

    for (std::size_t i = 0; i < current_front.job_pairs.size(); i++)
    {
        auto& job_pair = current_front.job_pairs[i];
        auto* job = job_pair.job;
        AlgorithmWeights jw;
        Weights::set(jw, Weights::ancestors_per_left, job->get_ancestors()->size() / job_count_not_assigned);
        Weights::set(jw, Weights::ancestors_per_job, job->get_ancestors()->size() / data_for_weights.job_count_overall);
        Weights::set(jw, Weights::critical_time_per_max_critical_time, job->get_critical_time() / data_for_weights.max_critical_time);
        Weights::set(jw, Weights::avg_occupancy, job->get_average_occupancy());
        Weights::set(jw, Weights::time_after_begin_per_overall_time,
            (*_current_time - job->get_start_after()) / static_cast<double>(job->get_end_before() - job->get_start_after()));
        Weights::set(jw, Weights::worker_count_per_max_worker_count, job_pair.worker_count / data_for_weights.worker_max_count);
        Weights::set(jw, Weights::bias, 1);
        job_pair.current_preference = 0;
        for (auto& name : Weights::WeightsNames)
        {
            job_pair.current_preference += jw.at(name) * _weights.at(name);
        }
        // Current preference will probably be <= 1 in theory? No.
    }
    std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_weights);
    for (auto& job : current_front.job_pairs)
    {
        qDebug() << "Time" << (*_current_time) << "Job" << job.job->get_global_id() << "Preference" << job.current_preference;
    }
}

void PendingFronts::apply_preference_coefficient_to_current_front(Data& current_front)
{
    for (int i = 0; i < current_front.job_pairs.size(); i++)
    {
        int coeff = current_front.job_pairs[i].job->get_preference_coefficient();
        if (coeff == 0) continue;
        int j = i - coeff;
        if (j < 0) j = 0;
        std::swap(current_front.job_pairs[i], current_front.job_pairs[j]);
    }
}

PendingFronts::PendingFronts(int *current_time, AssignedJobs *next, Preference preference, int look_ahead_time, AlgorithmWeights weights)
    : _current_time(current_time), next(next), _look_ahead_time(look_ahead_time), _preference(preference), _weights(weights)
{

}

bool PendingFronts::update_time_to_front()
{
    if (_data.size() != 0)
    {
        *_current_time = _data[0].time;
        return true;
    }
    return false;
}

void PendingFronts::add(int front_time, JobPair job_pair)
{
    Data new_data = { front_time, { job_pair } };
    SearchResult result = binarySearch(new_data);
    if (!result.found)
    {
        _data.insert(_data.begin() + result.pos, new_data);
    }
    else
    {
        _data[result.pos].job_pairs.push_back(job_pair);
    }
}

void PendingFronts::add(int front_time)
{
    Data new_data = { front_time, { } };
    SearchResult result = binarySearch(new_data);
    if (!result.found)
    {
        _data.insert(_data.begin() + result.pos, new_data);
    }
}

void PendingFronts::sort_current_front_by_preference(Data& current_front)
{
    switch (_preference)
    {
    case SPT:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_SPT);
        break;
    case LPT:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_LPT);
        break;
    case FLS:
        std::sort(current_front.job_pairs.begin(), current_front.job_pairs.end(), compare_EST);
        break;
    }
}

bool PendingFronts::tick(AlgorithmDataForWeights data_for_weights)
{
    if (_data.size() == 0) return false;
    int current_time = _data[0].time;
    Data current_front = _data[0];
    if (_preference == Preference::NONE) sort_current_front(current_front, data_for_weights);
    else sort_current_front_by_preference(current_front);

    apply_preference_coefficient_to_current_front(current_front);

    int transmitted_to_another_front = 0;
    int sent_to_next = 0;

    for (int i = 0; i < current_front.job_pairs.size(); i++)
    {
        PendingJobs::Data current_pending = current_front.job_pairs[i];
        int new_front_time = -1;
        for (int j = 0; j < current_pending.worker_groups.size(); j++)
        {
            Placement earliest_placement = current_pending.worker_groups[j]->get_earliest_placement_time(current_pending.job);
            
            if (!earliest_placement.worker) continue; // No available worker in that worker group
            if (earliest_placement.time_before == 0 && current_time < current_pending.job->get_start_after())
            {
                earliest_placement.time_before = current_pending.job->get_start_after() - current_time; // Data fixer for look_ahead_time
            }
            if (earliest_placement.time_before == 0) // Assign right now
            {
                AssignedWorker assigned_to = current_pending.worker_groups[j]->assign(current_pending.job);
                next->add(current_time, j, assigned_to.internal_id, current_pending, assigned_to.worker);

                add(current_time + current_pending.job->get_time_to_spend()); // In case this job is the last predecessor
                current_pending.worker_groups[j]->set_clock(_current_time); // Just in case I guess?
                current_front.job_pairs.erase(current_front.job_pairs.begin() + i);
                new_front_time = -1;
                i--;
                sent_to_next++;
                break;
            }
            if (earliest_placement.time_before > 0 && earliest_placement.time_before <= _look_ahead_time)
            { // Assign using preserve
                if (!earliest_placement.worker->is_preserved())
                    earliest_placement.worker->preserve(earliest_placement.time_before);
                new_front_time = earliest_placement.time_before;
                break;
            }
            if (new_front_time == -1 || new_front_time > earliest_placement.time_before)
            {
                new_front_time = earliest_placement.time_before;
            }
        }
        if (new_front_time == -1) continue; // Assigned right now
        new_front_time += current_time;
        add(new_front_time, current_pending);
        // Check if new_front_time == current_time -> exception()?
        transmitted_to_another_front++;
    }

    if (_data[0].job_pairs.size() != transmitted_to_another_front + sent_to_next)
    {
        throw std::invalid_argument("Some job was lost");
    }
    _data.erase(_data.begin());

    //qDebug() << "Time before is" << (*_current_time);

    return update_time_to_front();
}
