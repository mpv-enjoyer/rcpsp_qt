#include "pendingjobs.h"

PendingJobs::add(int start, int end, Job* job, std::vector<WorkerGroup*> workers)
{
    Data to_add = {start, end, job, workers, static_cast<int>(pending_jobs.size())};
    _data.push_back(to_add);
}
