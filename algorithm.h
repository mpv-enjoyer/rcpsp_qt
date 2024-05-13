#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "jobgroup.h"
#include "workergroup.h"
#include <QtDebug>
#include <QProgressBar>
#include <QFile>

struct JobPair
{
    int start_after;
    int end_before;
    Job* job;
    std::vector<WorkerGroup*> worker_groups;
    int id;
};

struct ResultPair
{
    Job* job;
    Worker* worker;
    int start;
    int job_id;
    int worker_group_id;
    int worker_internal_id;
};

struct FrontData
{
    int time;
    std::vector<JobPair> job_pairs;
};

enum Preference
{
    NONE = 0,
    SPT = 1,
    LPT = 2,
    FLS = 3
};

struct SearchResult
{
    bool found;
    int pos;
};

class Algorithm
{
    Preference preference = NONE;
    std::vector<JobPair> pending_jobs;
    int current_time;
    std::vector<FrontData> pending_fronts;
    std::vector<ResultPair> completed_jobs;
    std::vector<ResultPair> assigned_jobs; //To later become completed.
    bool check_nearest_front();
    int set_critical_time(JobPair current_job_pair);
    void begin_set_critical_time();

    int look_ahead_time = 0;

    int last_loop_check_begin = -1;
    int longest_plan_loop = 0;
public:
    Algorithm();
    void set_preference(Preference new_preference);
    void add_job_group(JobGroup* jobs, WorkerGroup* workers);
    void run();
    SearchResult binarySearch(const std::vector<FrontData>& arr, const FrontData& x);
    std::vector<ResultPair> get_completed();
    std::vector<JobPair> get_failed();
    void set_log_bar(QProgressBar* bar);
    void LoadCSV(QString file_name, std::vector<Worker *> &all_workers, std::vector<Job *> &all_jobs);
    int get_look_ahead_time() const;
    void set_look_ahead_time(int newLook_ahead_time);
};

struct JobLoad
{
    Job* assign;
    int id;
    std::vector<OccupancyPair> occupancy;
    std::vector<int> ancestors;
};

struct WorkerLoad
{
    Worker* assign;
    int id;
    int plan;
};

struct PlanLoad
{
    Plan* assign;
    int id;
    int start_at;
    std::vector<PlanElement> plan;
};

struct JobGroupLoad
{
    JobGroup* assign;
    int id;
    int start_after;
    int end_before;
    int worker_group;
    std::vector<int> jobs;
};

struct WorkerGroupLoad
{
    WorkerGroup* assign;
    int id;
    std::vector<int> workers;
};

#endif // ALGORITHM_H
