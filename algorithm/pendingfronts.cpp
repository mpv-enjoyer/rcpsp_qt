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

void PendingFronts::sort_current_front(Data& current_front)
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

PendingFronts::PendingFronts(int *current_time, AssignedJobs *next, Preference preference, int look_ahead_time)
    : _current_time(current_time), next(next), _look_ahead_time(look_ahead_time), _preference(preference)
{

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

bool PendingFronts::tick()
{
    if (_data.size() == 0) return false;
    bool result = false;
    int current_time = _data[0].time;
    Data current_front = _data[0];
    sort_current_front(current_front);
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
                if (new_front_time == -1) throw std::exception();
                break;
            }
            if (new_front_time == -1 || new_front_time > earliest_placement.time_before)
            {
                new_front_time = earliest_placement.time_before;
            }
        }
        if (new_front_time == -1) continue; // Assigned right now
        result = true;
        new_front_time += current_time;
        add(new_front_time, current_pending);
        // Check if new_front_time == current_time -> exception()?
        transmitted_to_another_front++;
    }

    /*bool should_copy_front_plus_one = _data.size() == 1 && result;
    if (last_loop_check_begin != 1 &&
        last_loop_check_begin + _longest_plan_loop < current_time &&
        _data.size() == 1)
    {
        should_copy_front_plus_one = true;
    } // Just add everything in fronts instead of whatever this is?
    if (should_copy_front_plus_one)
    {
        _data[0] = current_front;
        _data[0].time++;
        return true;
    }*/

    // ^ Shouldn't be needed. Every job must have a front with time equal to it's arrival plus ^
    // ^  if this job cannot be started because of predecessors, they will trigger the update  ^

    if (_data[0].job_pairs.size() != transmitted_to_another_front + sent_to_next) throw std::exception(); // Some job was lost
    _data.erase(_data.begin());

    if (_data.size() != 0) (*_current_time) = _data[0].time;

    return result;
}
