#include "resultcheck.h"

void ResultCheck::successor() const
{
    for (const auto& pair : m_result)
    {
        pair.job->get_start_after();
        for (Job* successor : *(pair.job->get_successors()))
        {
            int global_successor_id = successor->get_global_id();
            for (const auto& find_successor_pair : m_result)
            {
                if (find_successor_pair.job_id == global_successor_id)
                {
                    if (pair.start + pair.job->get_time_to_spend() > find_successor_pair.start)
                    {
                        throw std::invalid_argument("Invalid successor");
                    }
                    break;
                }
            }
        }
    }
}

void ResultCheck::plan() const
{
    for (const auto& pair : m_result)
    {
        if (pair.worker->get_plan().get_time_until_rest(pair.start) < pair.job->get_time_to_spend())
        {
            throw std::invalid_argument("Bad job start because plan");
        }
    }
}

void ResultCheck::occupancy() const
{
    for (const auto& pair : m_result)
    {
        int start = pair.start;
        int end = pair.start + pair.job->get_time_to_spend();
        auto occ_sum = std::vector<float>(pair.job->get_time_to_spend());
        for (const auto& current : m_result)
        {
            int current_start = current.start;
            int current_end = current_start + current.job->get_time_to_spend();
            if (current_start > end) continue;
            if (current_end < start) continue;
            if (current.worker != pair.worker) continue;
            int current_offset = std::max(0, start - current_start);
            int offset_max = std::min(current.job->get_time_to_spend(), end - current_start);
            for (; current_offset < offset_max; current_offset++)
            {
                occ_sum[-(start - current_start) + current_offset] += current.job->get_occupancy_at(current_offset);
            }
        }
        //for (auto occ : occ_sum)
        //{
        //    if (occ > 1.000001f) throw std::invalid_argument("Bad occupancy");
        //}
    }
}

ResultCheck::ResultCheck(const std::vector<ResultPair> &result)
    : m_result(result)
{
    successor();
    plan();
    occupancy();
}
