#include "resultcheck.h"

ResultCheck::ResultCheck(std::vector<ResultPair> result)
{
    for (auto pair : result)
    {
        pair.job->get_start_after();
        for (Job* successor : *(pair.job->get_successors()))
        {
            int global_successor_id = successor->get_global_id();
            for (auto find_successor_pair : result)
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
