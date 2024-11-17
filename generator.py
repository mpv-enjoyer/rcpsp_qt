# 0            | 1                         | 2                 | 3             | 4            | 5       | 6            | 7...
# --------------------------------------------------------------------------------------------------------------------------------------------------
# job          | id (from 0 without skips) | time, busyness... | ]             | ancestors... |         |              |
# worker       | id (from 0 without skips) | plan_id           |               |              |         |              |
# plan         | id (from 0 without skips) | start_at          | work, rest... |              |         |              |
# job_group    | id (from 0 without skips) | start_after       | end_before    | worker_group | jobs... | ] (optional) | worker_groups... (optional)
# worker_group | id (from 0 without skips) | workers...        |               |              |         |              |

import numpy as np
from scipy.stats import truncnorm
import pdb

## whatever dist - dist that is defined only for that line and is randomized
## dependant dist - whatever dist that can be changed according to the previous value

# job_count = uniform dist 10 ~ 10000
# worker_count = uniform dist ( job_count / 100 ) ~ job_count / 5
# plan_count = uniform dist ( worker_count / 50 ) ~ ( worker_count / 2 )
# max_plan_loop = uniform dist 10000 ~ 50000
# current_plan_unit_count: N = poisson dist lambda 1 (LIMIT N <= 100)
# current_plan_start_at: uniform dist 0 ~ max_plan_loop
# current_plan_current_unit_time = dependant dist 1 ~ ( max_plan_loop / current_plan_section_count )
# max_plan_unit = MAX(current_plan_current_unit_time...)
# min_job_time_to_spend, max_job_time_to_spend = MIN, MAX from: uniform dist 1 ~ max_plan_unit, uniform dist 1 ~ max_plan_unit
# max_job_group_count = uniform dist 1 ~ job_count
# current_job_max_time_to_spend = whatever dist min_job_time_to_spend ~ max_job_time_to_spend
# current_job_busyness_section_count = whatever dist 1 ~ current_job_max_time_to_spend / 2
# current_job_current_busyness_value = dependant dist 0 ~ 1
# current_job_current_busyness_time = uniform dist 1 ~ current_job_max_time_to_spend / current_job_busyness_section_count
# current_job_ancestors_count = whatever dist 0 ~ possible_ancestors_left
# current_job_group = whatever dist 1 ~ max_job_group_count
# job_group_count = clamp unused job_groups
# max_worker_group_count = uniform dist 1 ~ job_group_count
# current_worker's_group_id = whatever dist 1 ~ max_worker_group_count
# worker_group_count = clamp unused worker_groups
# current_job_group_corresponds_to_N_worker_groups: N = poisson dist (LIMIT 0 < N <= worker_group_count)
# current_job_group_to_id_N_worker_group: N = uniform dist 1 ~ worker_group_count
# current_job_group_start_at = whatever dist 0 ~ max_plan_loop

rng = np.random.default_rng()
ROUND = 3

def get_truncated_normal(mean=0.0, sd=1.0, low=0.0, upp=10.0, size=1):
    if size == 1:
        return round(truncnorm((low - mean) / sd, (upp - mean) / sd, loc=mean, scale=sd).rvs(), ROUND)
    return [round(i, ROUND) for i in truncnorm((low - mean) / sd, (upp - mean) / sd, loc=mean, scale=sd).rvs(size)]

def get_random_whatever_dist() -> str:
    return rng.choice(['trnorm', 'uniform'])

def get_random_dependant_dist() -> str:
    return rng.choice(['uniform_asc', 'uniform_desc', 'uniform_back_and_forth', 'trnorm' ,'uniform'])

def get_random_int(low: int, high: int, size = 1) -> int:
    if low == high:
        high += 1
    if size == 1:
        return rng.integers(low, high)
    return rng.integers(low, high, size)

def whatever_dist_int(low: int, high: int, size = 1, dist = 'none'):
    if low == high:
        high += 1
    if dist == 'none':
        dist = get_random_whatever_dist()
    if dist == 'trnorm':
        mean = get_random_int(low, high)
        if size == 1:
            return int(get_truncated_normal(mean, 1, low, high))
        return list(map(int, get_truncated_normal(mean, 1, low, high, size)))
    if dist == 'uniform':
        if size == 1:
            return get_random_int(low, high)
        return get_random_int(low, high, size)
    raise "invalid dist"

def dependant_dist_int(low: int, high: int, size: int, dist = 'none'):
    if dist == 'none':
        dist = get_random_dependant_dist()
    section_size = float((high - low) / size)
    if dist == 'uniform_asc':
        return [get_random_int(low + section_size * i, low + section_size * (i + 1)) for i in range(size)]
    if dist == 'uniform_desc':
        #rev_i = size - i - 1
        return [get_random_int(low + section_size * (size - i - 1), low + section_size * (size - i)) for i in range(size)]
    if dist == 'trnorm' or dist == 'uniform':
        return whatever_dist_int(low, high, size, dist)
    output = [0] * size
    if dist == 'uniform_back_and_forth':
        for i in range(size):
            if i == 0:
                output[0] = get_random_int(low, high)
            elif i % 2 == 0:
                output[i] = get_random_int(output[i - 1], high)
            else:
                output[i] = get_random_int(low, output[i - 1])
        return output
    raise "invalid dist"

def get_random_float(low: float, high: float, size = 1) -> float:
    if size == 1:
        return round(rng.random() * (high - low) + low, ROUND)
    return [round(i * (high - low) + low, ROUND) for i in rng.random(size)]

def whatever_dist_float(low: float, high: float, size = 1, dist = 'none'):
    if dist == 'none':
        dist = get_random_whatever_dist()
    if dist == 'trnorm':
        mean = get_random_float(low, high)
        if size == 1:
            return get_truncated_normal(mean, 1, low, high)
        return list(map(float, get_truncated_normal(mean, 1, low, high, size)))
    if dist == 'uniform':
        if size == 1:
            return get_random_float(low, high)
        return get_random_float(low, high, size)
    raise "invalid dist"

def dependant_dist_float(low: float, high: float, size: int, dist = 'none'):
    if dist == 'none':
        dist = get_random_dependant_dist()
    section_size = float((high - low) / size)
    if dist == 'uniform_asc':
        return [get_random_float(low + section_size * i, low + section_size * (i + 1)) for i in range(size)]
    if dist == 'uniform_desc':
        #rev_i = size - i - 1
        return [get_random_float(low + section_size * (size - i - 1), low + section_size * (size - i)) for i in range(size)]
    if dist == 'trnorm' or dist == 'uniform':
        return whatever_dist_float(low, high, size, dist)
    output = [0.0] * size
    if dist == 'uniform_back_and_forth':
        for i in range(size):
            if i == 0:
                output[0] = get_random_float(low, high)
            elif i % 2 == 0:
                output[i] = get_random_float(output[i - 1], high)
            else:
                output[i] = get_random_float(low, output[i - 1])
        return output
    raise "invalid dist"

# Begin generation:
generated = ""
print(job_count := get_random_int(10, 10000))
print(worker_count := get_random_int(max(job_count / 100, 1), job_count / 5))
print(plan_count := get_random_int(max(worker_count / 100, 1), max(worker_count / 50, 2)))
print(max_plan_loop := get_random_int(10000, 50000))
for plan in range(plan_count):
    print(current_plan_unit_count := min(rng.poisson(1) + 1, 100))
    if plan == 0:
        current_plan_start_at = 0
    else:
        current_plan_start_at = get_random_int(0, max_plan_loop / 100)
    generated += "plan;" + str(plan) + ";" + str(current_plan_start_at) + ";"
    work = dependant_dist_int(1, max(max_plan_loop / current_plan_unit_count, 1), current_plan_unit_count)
    rest = dependant_dist_int(1, max(max_plan_loop / current_plan_unit_count, 1), current_plan_unit_count)
    if current_plan_unit_count == 1:
        generated += str(work) + ";" + str(rest) + ";"
    else:
        for unit in range(current_plan_unit_count):
            generated += str(work[unit]) + ";" + str(rest[unit]) + ";"
    generated += "\n"
max_plan_unit = max(work)
min_job_time_to_spend = get_random_int(1, max_plan_unit)
max_job_time_to_spend = get_random_int(1, max_plan_unit)
if max_job_time_to_spend <= min_job_time_to_spend:
    max_job_time_to_spend, min_job_time_to_spend = min_job_time_to_spend + 1, max_job_time_to_spend
# Swapped them if necessary
max_job_group_count = get_random_int(1, job_count)
wdist1 = get_random_whatever_dist() # We need to preserve the same "whatever" dists for all jobs
wdist2 = get_random_whatever_dist()
wdist3 = get_random_whatever_dist()
wdist4 = get_random_whatever_dist()
job_groups_dict = dict() # id with skips -> job ids

import timeit

f = open("generated_sample.txt", "w+")
f.write(generated)
generated = ""

for job in range(job_count):
    jobs_left_to_iterate = job_count - 1 - job
    current_job_max_time_to_spend = whatever_dist_int(min_job_time_to_spend, max_job_time_to_spend, 1, wdist1)
    current_job_busyness_section_count = whatever_dist_int(1, current_job_max_time_to_spend / 2, 1, wdist2)
    current_job_busyness_values = dependant_dist_float(0, 1, current_job_busyness_section_count)
    current_job_busyness_times = get_random_int(1, current_job_max_time_to_spend / current_job_busyness_section_count, current_job_busyness_section_count)
    current_job_ancestors_count = whatever_dist_int(0, jobs_left_to_iterate, 1, wdist3)
    current_job_ancestors = [(i + job + 1) for i in rng.choice(jobs_left_to_iterate, size=current_job_ancestors_count, replace=False)]
    current_job_group = whatever_dist_int(0, max_job_group_count - 1, 1, wdist4)
    if current_job_group in job_groups_dict:
        job_groups_dict[current_job_group].append(job)
    else:
        job_groups_dict[current_job_group] = list()
        job_groups_dict[current_job_group].append(job)
    generated += "job;" + str(job) + ";"
    if current_job_busyness_section_count == 1:
        generated += str(current_job_busyness_times) + ";" + str(current_job_busyness_values) + ";"
    else:
        for i in range(current_job_busyness_section_count):
            generated += str(current_job_busyness_times[i]) + ";" + str(current_job_busyness_values[i]) + ";"
    generated += "];"
    for ancestor in current_job_ancestors:
        generated += str(ancestor) + ";"
    generated += "\n"
    f.write(generated)
    generated = ""

# Clamp job_group ids:
current_job_group = 0
new_job_groups_dict = dict()
for key in job_groups_dict:
    new_job_groups_dict[current_job_group] = job_groups_dict[key]
    current_job_group += 1
job_groups_dict = new_job_groups_dict
job_group_count = len(job_groups_dict)

max_worker_group_count = get_random_int(1, job_group_count)
worker_group_ids = whatever_dist_int(1, max_worker_group_count, worker_count)

# Clamp worker_group ids:
worker_groups_dict = dict() # id with skips -> worker ids
for worker in range(len(worker_group_ids)):
    group_id = worker_group_ids[worker]
    if group_id not in worker_groups_dict:
        worker_groups_dict[group_id] = list()
    worker_groups_dict[group_id].append(worker)
worker_group_count = len(worker_groups_dict)

# Now we have full worker_group and job_group dicts, merging:
job_group_to_worker_groups_dict = dict()
for job_group in job_groups_dict:
    current_job_group_worker_group_count = min(rng.poisson(1) + 1, worker_group_count)
    current_job_group_worker_groups = rng.choice(worker_group_count, size=current_job_group_worker_group_count, replace=False)
    if current_job_group_worker_group_count == 1:
        current_job_group_worker_groups = list(current_job_group_worker_groups)
    job_group_to_worker_groups_dict[job_group] = current_job_group_worker_groups

# Generate remaining data and write workers, job_groups, worker_groups:
wdist = get_random_whatever_dist()
for worker in range(worker_count):
    current_plan = whatever_dist_int(0, plan_count, 1, wdist)
    generated += "worker;" + str(worker) + ";" + str(current_plan) + "\n"

JOB_GROUP_END_BEFORE = 2147483647 - 1 # __INT_MAX__ - 1
wdist = get_random_whatever_dist()
for job_group in job_groups_dict:
    current_job_group_start_at = whatever_dist_int(0, max_plan_loop, 1, wdist)
    generated += "job_group;" + str(job_group) + ";" + str(current_job_group_start_at) + ";" + str(JOB_GROUP_END_BEFORE) + ";" + str(job_group_to_worker_groups_dict[job_group][0]) + ";"
    for job in job_groups_dict[job_group]:
        generated += str(job) + ";"
    if len(job_group_to_worker_groups_dict[job_group]) > 1:
        generated += "];"
        for i in range(1, len(job_group_to_worker_groups_dict[job_group])):
            generated += str(job_group_to_worker_groups_dict[job_group][i]) + ";"
    generated += "\n"

for worker_group in worker_groups_dict:
    generated += "worker_group;" + str(worker_group) + ";"
    for worker in worker_groups_dict[worker_group]:
        generated += str(worker) + ";"
    generated += "\n"

f.write(generated)
f.close()