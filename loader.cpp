#include "loader.h"
#include "algorithm.h"

bool Loader::Load(QString file_name, Algorithm& algorithm, std::vector<Worker*>& all_workers, std::vector<Job*>& all_jobs)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        return false;
    }
    // Создаём поток для извлечения данных из файла
    QTextStream in(&file);
    // Считываем данные до конца файла
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
            JobLoad current = { nullptr, 0, {{0, 0}}, std::vector<int>() };
            current.id = list[1].toInt();
            int i = 2;
            for (; list[i] != ']'; i += 2)
            {
                current.occupancy.push_back({list[i].toInt(), list[i+1].toFloat()});
            }
            i++;
            for (; i < list.size(); i++)
            {
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
            PlanLoad current = { nullptr, 0, 0, std::vector<PlanElement>() };
            current.id = list[1].toInt();
            current.start_at = list[2].toInt();
            for (int i = 3; i < list.size(); i+=2)
            {
                current.plan.push_back({ list[i].toInt(), list[i + 1].toInt() });
            }
            plans_load.push_back(current);
        }
        if (list[0] == "job_group")
        {
            JobGroupLoad current = { nullptr, 0, 0, 0, 0, std::vector<int>() };
            current.id = list[1].toInt();
            current.start_after = list[2].toInt();
            current.end_before = list[3].toInt();
            current.worker_group = list[4].toInt();
            for (int i = 5; i < list.size(); i++)
            {
                current.jobs.push_back(list[i].toInt());
            }
            job_groups_load.push_back(current);
        }
        if (list[0] == "worker_group")
        {
            WorkerGroupLoad current = { nullptr, 0, std::vector<int>() };
            current.id = list[1].toInt();
            for (int i = 2; i < list.size(); i++)
            {
                current.workers.push_back(list[i].toInt());
            }
            worker_groups_load.push_back(current);
        }
        if (list[0] == "preference")
        {
            if (list[1] == "SPT") algorithm.set_preference(SPT);
            if (list[1] == "LPT") algorithm.set_preference(LPT);
            if (list[1] == "FLS") algorithm.set_preference(EST);
        }
        // 1) job | worker | plan | job_group | worker_group | preference
        // 2) id (from 0 without skips) | preference: "SPT | LPT | EST"
        // 3) job: time, busyness... | worker: plan_id | plan: start_at | job_group: start_after | worker_group: workers...
        // 4) job: ] | | plan: work, rest... | job_group: end_before |
        // 5) job: ancestors... | | | job_group: worker_group
        // 6) | | | job_group: jobs...
    }
    file.close();

    for (int i = 0; i < plans_load.size(); i++)
    {
        plans_load[i].assign = new Plan(plans_load[i].plan, plans_load[i].start_at);
        std::swap(plans_load[i], plans_load[plans_load[i].id]);
    }
    all_workers = std::vector<Worker*>(workers_load.size());
    for (int i = 0; i < workers_load.size(); i++)
    {
        workers_load[i].assign = new Worker(*plans_load[workers_load[i].plan].assign);
        int id_true = workers_load[i].id;
        std::swap(workers_load[i], workers_load[id_true]);
        all_workers[id_true] = workers_load[id_true].assign;
    }
    for (int i = 0; i < worker_groups_load.size(); i++)
    {
        WorkerGroup* worker_group = new WorkerGroup();
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
        std::vector<Job*> want_jobs;
        for (int j = 0; j < job_groups_load[i].jobs.size(); j++)
        {
            int id = job_groups_load[i].jobs[j];
            want_jobs.push_back(jobs_load[id].assign);
        }
        job_groups_load[i].assign = new JobGroup(want_jobs, job_groups_load[i].start_after, job_groups_load[i].end_before);
        int current_worker_group_id = job_groups_load[i].worker_group;
        algorithm.add_job_group(job_groups_load[i].assign, worker_groups_load[current_worker_group_id].assign);
    }
    return true;
}
