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

int Plan::get_time_nearest_possible(int current_time, int job_time) const
{
    int current_element = 0;
    current_time -= start;
    if (current_time < 0) return 0;
    while (current_time >= 0)
    {
        for (int i = 0; i < elements.size(); i++)
        {
            current_time -= elements[i].work;
            if (current_time < 0)
            {
                current_element = i + 1;
                if (-current_time >= job_time)
                {
                    return -current_time;
                }
                current_time -= elements[i].rest;
                break;
            }
            current_time -= elements[i].rest;
            if (current_time < 0)
            {
                current_element = i + 1;
                break;
            }
        }
    }
    int start_element = current_element;
    for (int i = 0; i < elements.size(); i++)
    {
        current_element = start_element + i;
        if (elements[current_element].work <= job_time)
        {
            return -current_time;
        }
        current_time -= elements[current_element].work;
        current_time -= elements[current_element].rest;
    }
    return -1;
}
