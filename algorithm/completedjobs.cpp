#include "completedjobs.h"

CompletedJobs::CompletedJobs() {}

void CompletedJobs::add(Data data)
{
    _data.push_back(data);
}
