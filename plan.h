#ifndef PLAN_H
#define PLAN_H

#include <vector>

struct PlanElement
{
    int work;
    int rest;
};

class Plan
{
private:
    std::vector<PlanElement> elements;
    int start;
public:
    Plan(std::vector<PlanElement> want_elements, int want_start = 0);
    bool is_ready(int current_time) const;
    int get_time_until_ready(int current_time) const;
    int get_time_until_rest(int current_time) const;
};

#endif // PLAN_H
