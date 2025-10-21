#ifndef LOADER_H
#define LOADER_H

#include "algorithm.h"
#include <QFile>

class Loader
{
public:
    static bool Load(QString file_name, Algorithm &algorithm);
    static bool LoadPreferences(QString file_name, Algorithm& algorithm);
    static bool LoadWeights(QString file_name, Algorithm &algorithm);
    static AlgorithmWeights LoadWeights(QString file_name);
};

#endif // LOADER_H
