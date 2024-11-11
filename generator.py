#1) job | worker | plan | job_group | worker_group | preference | look_ahead
#2) id (from 0 without skips) | preference: "SPT | LPT | FLS" | look_ahead: number
#3) job: time, busyness... | worker: plan_id | plan: start_at | job_group: start_after | worker_group: workers...
#4) job: ] | | plan: work, rest... | job_group: end_before |
#5) job: ancestors... | | | job_group: worker_group
#6) | | | job_group: jobs...

import numpy as np

## whatever dist - dist that is defined only for that line and is randomized
## dependant dist - whatever dist that can be changed according to the previous value

# job_count = uniform dist 10 ~ 10000
# worker_count = uniform dist ( job_count / 100 ) ~ job_count / 5
# plan_count = uniform dist ( worker_count / 50 ) ~ ( worker_count / 2 )
# max_plan_loop = uniform dist 10000 ~ 50000
# current_plan_unit_count: N = poisson dist (LIMIT N <= 100)
# current_plan_current_unit_time = dependant dist 1 ~ ( max_plan_loop / current_plan_section_count )
# max_plan_unit = MAX(current_plan_current_unit_time...)
# min_job_time_to_spend, max_job_time_to_spend = MIN, MAX from: uniform dist 1 ~ max_plan_unit, uniform dist 1 ~ max_plan_unit
# current_job_time_to_spend = whatever dist min_job_time_to_spend ~ max_job_time_to_spend
# current_job_busyness_section_count = whatever dist 1 ~ current_job_time_to_spend / 2
# current_job_current_busyness_value = dependant dist 0 ~ 1
# max_job_group_count = uniform dist 1 ~ job_count
# current_job_group = whatever dist 1 ~ max_job_group_count
# job_group_count = clamp unused job_groups
# max_worker_group_count = uniform dist 1 ~ job_group_count
# current_worker's_group_id = whatever dist 1 ~ max_worker_group_count
# worker_group_count = clamp unused worker_groups
# current_job_group_corresponds_to_N_worker_groups: N = poisson dist (LIMIT 0 < N <= worker_group_count)
# current_job_group_to_id_N_worker_group: N = uniform dist 1 ~ worker_group_count
# current_job_ancestors_count = dependant dist 0 ~ possible_ancestors_left



print(job_count := np.random.randint(10, 10000))
print(worker_count := np.random.randint(max(job_count / 100, 1), job_count / 5))
print(plan_count := np.random.randint(max(worker_count / 50, 1), max(worker_count / 2, 1)))
print(max_plan_loop := np.random.randint(10000, 50000))
for i in range(plan_count):
    print(current_plan_unit_count := min(np.random.poisson(1) + 1, 100))
