#ifndef RESULTCHECK_H
#define RESULTCHECK_H

#include "algorithm.h"

class ResultCheck
{
    const std::vector<ResultPair>& m_result;
    void successor() const;
    void plan() const;
    void occupancy() const;
public:
    ResultCheck(const std::vector<ResultPair>& result);
};

#endif // RESULTCHECK_H
