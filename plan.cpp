#include "plan.h"

Plan::Plan(std::vector<PlanElement> want_elements, int want_start) :
    elements(want_elements), start(want_start)
{
    int plan_loop_time = 0;
    for (int i = 0; i < want_elements.size(); i++)
    {
        plan_loop_time += want_elements[i].rest;
        plan_loop_time += want_elements[i].work;
    }
    if (plan_loop_time == 0)
    {
        throw std::exception();
    }
}

bool Plan::is_ready(int current_time) const
{
    current_time -= start;
    if (current_time < 0) return 0; //resting
    while (current_time >= 0)
    {
        for (int i = 0; i < elements.size(); i++)
        {
            current_time -= elements[i].work;
            if (current_time < 0) return 1; //ready
            current_time -= elements[i].rest;
            if (current_time < 0) return 0; //resting
        }
    }
}

int Plan::get_time_until_ready(int current_time) const
{
    current_time -= start;
    if (current_time < 0) return -current_time;
    while (current_time >= 0)
    {
        for (int i = 0; i < elements.size(); i++)
        {
            current_time -= elements[i].work;
            if (current_time < 0) return 0;
            current_time -= elements[i].rest;
            if (current_time < 0) return -current_time;
        }
    }
}

int Plan::get_time_until_rest(int current_time) const
{
    current_time -= start;
    if (current_time < 0) return 0;
    while (current_time >= 0)
    {
        for (int i = 0; i < elements.size(); i++)
        {
            current_time -= elements[i].work;
            if (current_time < 0) return -current_time;
            current_time -= elements[i].rest;
            if (current_time < 0) return 0;
        }
    }
}
