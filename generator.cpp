#include "generator.h"

Generator::Generator(QString filename, Plan common_plan) : file_name(filename), commonPlan(common_plan)
{

}

bool Generator::is_ready()
{
    if (lowestJobTime == -1) return false;
    if (highestJobTime == -1) return false;
    if (allJobsSize == -1) return false;
    if (allWorkersSize == -1) return false;
    if (jobGroupLowestBegin == -1) return false;
    if (jobGroupHighestBegin == -1) return false;
    if (jobGroupLowestEnd == -1) return false;
    if (jobGroupHighestEnd == -1) return false;
    if (jobGroupsCount == -1) return false;
    if (groupSizeEntropy == -1) return false;
    return true;
}

void Generator::generate_and_write()
{
    if (!is_ready()) throw std::invalid_argument("Generator is not ready");

    QString filename = "generated.csv";
    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite)) qDebug() << "cannot create generated.csv";
    QTextStream stream(&file);


    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
    qDebug() << "Begin example generation";
    all_jobs.clear();
    for (int i = 0; i < allJobsSize; i++)
    {
        int time = QRandomGenerator::global()->bounded(lowestJobTime, highestJobTime);
        Job* generated = new Job(0, 0, {{time, 0.25f}, {time, 0.50f}, {time, 0.1f}});
        all_jobs.push_back(generated);
        int predecessor = QRandomGenerator::global()->bounded(0, allJobsSize);
        stream << "job;" << i << ";";
        stream << time << ";" << 1.0f << ";";
        stream << time << ";" << 1.0f << ";";
        stream << time << ";" << 1.0f << ";";
        stream << "]";
        if (predecessor < i)
        {
            generated->set_ancestors({all_jobs[predecessor]});
            stream << ";" << predecessor;
        }
        stream << '\n';
    }
    auto rng = std::default_random_engine {};
    stream << "plan;" << 0 << ";" << 0;
    for (int i = 0; i < commonPlan.get_elements().size(); i++)
    {
        stream << ";" << commonPlan.get_elements()[i].work << ";" << commonPlan.get_elements()[i].rest;
    }
    stream << '\n';
    //std::shuffle(all_jobs.begin(), all_jobs.end(), rng);
    int current_job = 0;
    std::vector<JobGroup*> job_groups = std::vector<JobGroup*>();
    for (int i = 0; i < jobGroupsCount; i++)
    {
        stream << "job_group;" << i << ";";
        int new_group_size = QRandomGenerator::global()->bounded(allJobsSize / jobGroupsCount - groupSizeEntropy, allJobsSize / jobGroupsCount + groupSizeEntropy);
        if (current_job + new_group_size >= allJobsSize || i == jobGroupsCount - 1)
        {
            new_group_size = allJobsSize - current_job;
        }
        std::vector<Job*> new_group_jobs = std::vector<Job*>(new_group_size);
        int begin = QRandomGenerator::global()->bounded(jobGroupLowestBegin, jobGroupHighestBegin);
        int end = QRandomGenerator::global()->bounded(jobGroupLowestEnd, jobGroupHighestEnd);
        stream << begin << ";" << end << ";0";
        for (int j = current_job; j < new_group_size + current_job; j++)
        {
            stream << ";" << j;
            new_group_jobs[j - current_job] = all_jobs[j];
        }
        stream << '\n';
        job_groups.push_back(new JobGroup(new_group_jobs, begin, end));
        current_job += new_group_size;
        if (current_job >= allJobsSize) break;
    }

    for (int i = 0; i < allWorkersSize; i++)
    {
        stream << "worker;" << i << ";0\n";
    }

    all_workers.clear();
    WorkerGroup* worker_group = new WorkerGroup();
    stream << "worker_group;0";
    for (int i = 0; i < allWorkersSize; i++)
    {
        Worker* generated = new Worker(commonPlan);
        all_workers.push_back(generated);
        worker_group->add_worker(generated);
        stream << ";" << i;
    }
    stream << '\n';

    if (look_ahead != -1)
        stream << "look_ahead;" << look_ahead << '\n';
    qDebug() << "example generated";
}
