#ifndef LOADER_H
#define LOADER_H

#include "algorithm.h"
#include <QFile>

class Loader
{
public:
    static bool Load(QString file_name, Algorithm &algorithm, std::vector<Worker *> &all_workers, std::vector<Job *> &all_jobs);
    static bool LoadPreferences(QString file_name, Algorithm& algorithm);
};

#endif // LOADER_H
