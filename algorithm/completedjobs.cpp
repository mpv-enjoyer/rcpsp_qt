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
        if (data.start + data.job->get_time_to_spend() > data.job->get_end_before()) result++;
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
        job.job->undone();
        job.worker->undone();
        if (job.start + job.job->get_time_to_spend() <= job.job->get_end_before()) continue;
        auto old_coefficient = job.job->get_preference_coefficient();
        job.job->set_preference_coefficient(old_coefficient + 1);
    }
}
