#ifndef GENERATOR_H
#define GENERATOR_H

#include "algorithm.h"
#include <QRandomGenerator>
#include <qfile.h>

class Generator
{
    QString file_name;
public:
    int lowestJobTime = -1;
    int highestJobTime = -1;
    int allJobsSize = -1;
    int allWorkersSize = -1;
    int jobGroupLowestBegin = -1;
    int jobGroupHighestBegin = -1;
    int jobGroupLowestEnd = -1;
    int jobGroupHighestEnd = -1;
    int jobGroupsCount = -1;
    int groupSizeEntropy = -1;
    int look_ahead = -1;
    Plan commonPlan;
    Generator(QString filename, Plan common_plan);
    bool is_ready();
    void generate_and_write();
};

#endif // GENERATOR_H
