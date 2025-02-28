#include "completedjobs.h"

CompletedJobs::CompletedJobs() {}

void CompletedJobs::add(Data data)
{
    _data.push_back(data);
}

std::vector<CompletedJobs::Data> CompletedJobs::result()
{
    return _data;
}

int CompletedJobs::failed_count()
{
    int result = 0;
    for (const auto& data : _data)
    {
        if (data.job->is_failed(data.start)) result++;
    }
    return result;
}

int CompletedJobs::succeded_count()
{
    return _data.size() - failed_count();
}

void CompletedJobs::prepare_for_next_iteration()
{
    for (auto job : _data)
    {
        // We use the same Job entity to preserve preference so we need to 'undone'
        job.job->undone();
        job.worker->undone();
        if (job.start + job.job->get_time_to_spend() <= job.job->get_end_before()) continue;
        auto old_coefficient = job.job->get_preference_coefficient();
        job.job->set_preference_coefficient(old_coefficient + 1);
    }
}

std::size_t CompletedJobs::data_size()
{
    return _data.size();
}
