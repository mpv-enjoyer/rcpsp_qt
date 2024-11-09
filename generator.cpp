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

    qDebug() << "Begin example generation";
    for (int i = 0; i < allJobsSize; i++)
    {
        int time = QRandomGenerator::global()->bounded(lowestJobTime, highestJobTime);
        int predecessor = QRandomGenerator::global()->bounded(0, allJobsSize);
        stream << "job;" << i << ";";
        stream << time << ";" << 1.0f << ";";
        stream << time << ";" << 1.0f << ";";
        stream << time << ";" << 1.0f << ";";
        stream << "]";
        if (predecessor < i)
        {
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
        }
        stream << '\n';
        current_job += new_group_size;
        if (current_job >= allJobsSize) break;
    }

    for (int i = 0; i < allWorkersSize; i++)
    {
        stream << "worker;" << i << ";0\n";
    }

    stream << "worker_group;0";
    for (int i = 0; i < allWorkersSize; i++)
    {
        stream << ";" << i;
    }
    stream << '\n';

    if (look_ahead != -1)
        stream << "look_ahead;" << look_ahead << '\n';
    qDebug() << "example generated";
}
