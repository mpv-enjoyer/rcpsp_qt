#include "loader.h"
#include "algorithm.h"
#include "../arena_cpp.h"
#include <unordered_set>
struct JobLoad
{
    Job* assign;
    int id;
    std::vector<OccupancyPair> occupancy;
    std::vector<int> successors;
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
bool Loader::Load(QString file_name, Algorithm& algorithm)
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
                current.successors.push_back(list[i].toInt());
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

    /* Invalid ID's check here? */
    Arena_Allocator& allocator = algorithm.reset_allocator_and_get();

    for (int i = 0; i < plans_load.size(); i++)
    {
        plans_load[i].assign = Plan(plans_load[i].plan, plans_load[i].start_at);
        std::swap(plans_load[i], plans_load[plans_load[i].id]);
    }
    auto all_workers = std::vector<Worker*>(workers_load.size());
    
    for (int i = 0; i < workers_load.size(); i++)
    {
        workers_load[i].assign = new(allocator.allocate<Worker>()) Worker(plans_load[workers_load[i].plan].assign);
        int id_true = workers_load[i].id;
        all_workers[id_true] = workers_load[i].assign;
    }

    std::vector<WorkerGroup*> all_worker_groups = std::vector<WorkerGroup*>(worker_groups_load.size());

    for (int i = 0; i < worker_groups_load.size(); i++)
    {
        WorkerGroup* worker_group = new(allocator.allocate<WorkerGroup>()) WorkerGroup();
        worker_group->set_global_id(worker_groups_load[i].id);
        worker_groups_load[i].assign = worker_group;
        all_worker_groups[worker_groups_load[i].id] = worker_group;
        for (int j = 0; j < worker_groups_load[i].workers.size(); j++)
        {
            worker_group->add_worker(all_workers[worker_groups_load[i].workers[j]]);
        }
    }
    auto jobs_load_fix = std::vector<JobLoad>(jobs_load.size());
    for (int i = 0; i < jobs_load.size(); i++)
    {
        jobs_load_fix[jobs_load[i].id] = jobs_load[i];
    }
    bool changed = true;

    std::function<std::vector<Job*>(const std::vector<int>&)> init_jobs = [&allocator, &jobs_load_fix, &init_jobs](const std::vector<int>& job_ids)
    {
        std::vector<Job*> output;
        output.reserve(job_ids.size());
        for (auto job_id : job_ids)
        {
            auto& job = jobs_load_fix[job_id].assign;
            if (!job)
            {
                auto successors = init_jobs(jobs_load_fix[job_id].successors);
                job = new(allocator.allocate<Job>()) Job(jobs_load_fix[job_id].occupancy, successors, job_id);
            }
            output.push_back(job);
        }
        return output;
    };
    auto init_job_group = [&init_jobs, &all_worker_groups](JobGroupLoad job_group_load) -> std::vector<Job*>
    {
        auto jobs = init_jobs(job_group_load.jobs);
        for (int i = 0; i < jobs.size(); i++)
        {
            jobs[i]->set_global_group_id(i);
            jobs[i]->set_start_after(job_group_load.start_after);
            jobs[i]->set_end_before(job_group_load.end_before);
        }
        return jobs;
    };

    std::vector<JobGroupLoad> job_groups_load_fix(job_groups_load.size());
    for (auto job_group_load : job_groups_load)
    {
        job_groups_load_fix[job_group_load.id] = job_group_load;
    }
    for (auto job_group_load : job_groups_load_fix)
    {
        auto job_group = init_job_group(job_group_load);
        const auto& current_worker_group_ids = job_group_load.worker_groups;
        std::vector<WorkerGroup*> worker_groups_for_job;
        for (auto worker_group_id : current_worker_group_ids)
        {
            worker_groups_for_job.push_back(all_worker_groups[worker_group_id]);
        }
        algorithm.add_job_group(job_group, worker_groups_for_job);
    }
    return true;
}

// preference     | SPT or LPT or SLS
// look_ahead     | number
// pass_max_count | number
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
        if (list[0] == "pass_max_count")
        {
            algorithm.set_pass_max_count(list[1].toInt());
        }
    }
    return true;
}

AlgorithmWeights Loader::LoadWeights(QString file_name)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        return Weights::create_empty();
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
        return Weights::create_empty();
    }
    return weights;
}

bool Loader::LoadWeights(QString file_name, Algorithm &algorithm)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        throw std::invalid_argument("Cannot start executing without weights");
        // probably unnessecary check
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
        qDebug("Got invalid weights...");
        return false;
        //throw std::invalid_argument("Invalid weights");
    }
    algorithm.set_weights(weights);
    return true;
}
