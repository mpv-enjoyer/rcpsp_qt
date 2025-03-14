#include "loader.h"
#include "algorithm.h"
#include <unordered_set>
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
    Plan assign;
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
    std::vector<int> worker_groups;
    std::vector<int> jobs;
};

struct WorkerGroupLoad
{
    WorkerGroup* assign;
    int id;
    std::vector<int> workers;
};

// 0            | 1                         | 2                 | 3             | 4            | 5       | 6            | 7...
// --------------------------------------------------------------------------------------------------------------------------------------------------
// job          | id (from 0 without skips) | time, busyness... | ]             | ancestors... |         |              |
// worker       | id (from 0 without skips) | plan_id           |               |              |         |              |
// plan         | id (from 0 without skips) | start_at          | work, rest... |              |         |              |
// job_group    | id (from 0 without skips) | start_after       | end_before    | worker_group | jobs... | ] (optional) | worker_groups... (optional)
// worker_group | id (from 0 without skips) | workers...        |               |              |         |              |
bool Loader::Load(QString file_name, Algorithm& algorithm, std::vector<Worker*>& all_workers, std::vector<Job*>& all_jobs)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        return false;
    }
    QTextStream in(&file);
    std::vector<JobLoad> jobs_load;
    std::vector<WorkerLoad> workers_load;
    std::vector<PlanLoad> plans_load;
    std::vector<JobGroupLoad> job_groups_load;
    std::vector<WorkerGroupLoad> worker_groups_load;
    algorithm.set_preference(NONE);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split(";");
        if (list[0] == "job")
        {
            JobLoad current = { nullptr, 0, {}, std::vector<int>() };
            current.id = list[1].toInt();
            int i = 2;
            for (; list[i] != "]"; i += 2)
            {
                if (list[i].size() == 0) break;
                current.occupancy.push_back({list[i].toInt(), list[i+1].toFloat()});
            }
            i++;
            for (; i < list.size(); i++)
            {
                if (list[i].size() == 0) break;
                current.ancestors.push_back(list[i].toInt());
            }
            jobs_load.push_back(current);
        }
        if (list[0] == "worker")
        {
            WorkerLoad current = { nullptr, 0, 0 };
            current.id = list[1].toInt();
            current.plan = list[2].toInt();
            workers_load.push_back(current);
        }
        if (list[0] == "plan")
        {
            PlanLoad current = { Plan({{1, 1}}, 0), 0, 0, std::vector<PlanElement>() };
            current.id = list[1].toInt();
            current.start_at = list[2].toInt();
            for (int i = 3; i < list.size(); i+=2)
            {
                if (list[i].size() == 0) break;
                current.plan.push_back({ list[i].toInt(), list[i + 1].toInt() });
            }
            plans_load.push_back(current);
        }
        if (list[0] == "job_group")
        {
            JobGroupLoad current = { nullptr, 0, 0, 0, std::vector<int>(), std::vector<int>() };
            current.id = list[1].toInt();
            current.start_after = list[2].toInt();
            current.end_before = list[3].toInt();
            current.worker_groups.push_back(list[4].toInt());
            int i = 5;
            for (; i < list.size(); i++)
            {
                if (list[i] == "]" || list[i].size() == 0) break;
                current.jobs.push_back(list[i].toInt());
            }
            i++;
            for (; i < list.size(); i++)
            {
                if (list[i].size() == 0) break;
                current.worker_groups.push_back(list[i].toInt());
            }
            job_groups_load.push_back(current);
        }
        if (list[0] == "worker_group")
        {
            WorkerGroupLoad current = { nullptr, 0, std::vector<int>() };
            current.id = list[1].toInt();
            for (int i = 2; i < list.size(); i++)
            {
                if (list[i].size() == 0) break;
                current.workers.push_back(list[i].toInt());
            }
            worker_groups_load.push_back(current);
        }
    }
    file.close();

    for (int i = 0; i < plans_load.size(); i++)
    {
        plans_load[i].assign = Plan(plans_load[i].plan, plans_load[i].start_at);
        std::swap(plans_load[i], plans_load[plans_load[i].id]);
    }
    all_workers = std::vector<Worker*>(workers_load.size());
    
    bool workers_load_skipped_someone = true; // TODO: I can do this in one cycle
    while (workers_load_skipped_someone)
    {
        workers_load_skipped_someone = false;
        for (int i = 0; i < workers_load.size(); i++)
        {
            if (workers_load[i].assign != nullptr) continue;
            workers_load[i].assign = new Worker(plans_load[workers_load[i].plan].assign);
            int id_true = workers_load[i].id;
            std::swap(workers_load[i], workers_load[id_true]);
            all_workers[id_true] = workers_load[id_true].assign;
            workers_load_skipped_someone = true;
        }
    }

    std::unordered_set<WorkerGroup*> unused_worker_groups;

    for (int i = 0; i < worker_groups_load.size(); i++)
    {
        WorkerGroup* worker_group = new WorkerGroup();
        unused_worker_groups.insert(worker_group);
        worker_group->set_global_id(worker_groups_load[i].id);
        worker_groups_load[i].assign = worker_group;
        std::swap(worker_groups_load[i], worker_groups_load[worker_groups_load[i].id]);
        for (int j = 0; j < worker_groups_load[i].workers.size(); j++)
        {
            worker_group->add_worker(workers_load[worker_groups_load[i].workers[j]].assign);
        }
    }
    all_jobs = std::vector<Job*>(jobs_load.size());
    for (int i = 0; i < jobs_load.size(); i++)
    {
        std::swap(jobs_load[i], jobs_load[jobs_load[i].id]);
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < jobs_load.size(); i++)
        {
            if (jobs_load[i].assign) continue;
            bool all_ancestors_assigned = true;
            std::vector<Job*> ancestors;
            for (int j = 0; j < jobs_load[i].ancestors.size(); j++)
            {
                if (!jobs_load[jobs_load[i].ancestors[j]].assign)
                {
                    all_ancestors_assigned = false;
                    break;
                }
                ancestors.push_back(jobs_load[jobs_load[i].ancestors[j]].assign);
            }
            if (!all_ancestors_assigned) continue;
            changed = true;
            Job* job = new Job(0, 0, jobs_load[i].occupancy);
            job->set_ancestors(ancestors);
            job->set_global_id(i);
            jobs_load[i].assign = job;
            all_jobs[i] = job;
        }
    }
    for (int i = 0; i < job_groups_load.size(); i++)
    {
        std::swap(job_groups_load[i], job_groups_load[job_groups_load[i].id]);
    }
    for (int i = 0; i < job_groups_load.size(); i++)
    {
        if (job_groups_load[i].id != i) throw std::invalid_argument("Bad job group ID");
    }
    for (int i = 0; i < job_groups_load.size(); i++)
    {
        std::vector<Job*> want_jobs;
        for (int j = 0; j < job_groups_load[i].jobs.size(); j++)
        {
            int id = job_groups_load[i].jobs[j];
            jobs_load[id].assign->set_global_group_id(i);
            want_jobs.push_back(jobs_load[id].assign);
        }
        job_groups_load[i].assign = new JobGroup(want_jobs, job_groups_load[i].start_after, job_groups_load[i].end_before);
        auto current_worker_group_ids = job_groups_load[i].worker_groups;
        std::vector<WorkerGroup*> worker_groups_for_job;
        for (auto worker_group_id : current_worker_group_ids)
        {
            if (unused_worker_groups.count(worker_groups_load[worker_group_id].assign) != 0) unused_worker_groups.erase(worker_groups_load[worker_group_id].assign);
            worker_groups_for_job.push_back(worker_groups_load[worker_group_id].assign);
        }
        algorithm.add_job_group(job_groups_load[i].assign, worker_groups_for_job);
        delete job_groups_load[i].assign;
    }
    for (auto unused_worker_group : unused_worker_groups)
    {
        delete unused_worker_group;
    }
    return true;
}

// preference | SPT or LPT or FLS
// look_ahead | number
bool Loader::LoadPreferences(QString file_name, Algorithm &algorithm)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        return false;
    }
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split(";");
        if (list[0] == "preference")
        {
            if (list[1] == "SPT") algorithm.set_preference(SPT);
            if (list[1] == "LPT") algorithm.set_preference(LPT);
            if (list[1] == "FLS") algorithm.set_preference(FLS);
        }
        if (list[0] == "look_ahead")
        {
            algorithm.set_look_ahead_time(list[1].toInt());
        }
    }
    return true;
}

bool Loader::LoadWeights(QString file_name, Algorithm &algorithm)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        throw std::invalid_argument("Cannot start executing without weights");
        return false;
    }
    QTextStream in(&file);
    AlgorithmWeights weights = { };
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split(";");
        if (list.size() < 2) continue;
        QString name = list[0];
        double value = list[1].toDouble();
        Weights::set(weights, name.toStdString(), value);
    }
    if (!Weights::are_valid(weights))
    {
        throw std::invalid_argument("Invalid weights");
    }
    algorithm.set_weights(weights);
    return true;
}
