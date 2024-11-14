#1) job | worker | plan | job_group | worker_group | preference | look_ahead
#2) id (from 0 without skips) | preference: "SPT | LPT | FLS" | look_ahead: number
#3) job: time, busyness... | worker: plan_id | plan: start_at | job_group: start_after | worker_group: workers...
#4) job: ] | | plan: work, rest... | job_group: end_before |
#5) job: ancestors... | | | job_group: worker_group
#6) | | | job_group: jobs...

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
# current_job_max_time_to_spend = whatever dist min_job_time_to_spend ~ max_job_time_to_spend
# current_job_busyness_section_count = whatever dist 1 ~ current_job_max_time_to_spend / 2
# current_job_current_busyness_value = dependant dist 0 ~ 1
# current_job_current_busyness_time = uniform dist 1 ~ current_job_max_time_to_spend / current_job_busyness_section_count
# current_job_ancestors_count = whatever dist 0 ~ possible_ancestors_left
# max_job_group_count = uniform dist 1 ~ job_count
# current_job_group = whatever dist 1 ~ max_job_group_count
# job_group_count = clamp unused job_groups [Leave all unused job_groups in generated file]
# max_worker_group_count = uniform dist 1 ~ job_group_count
# current_worker's_group_id = whatever dist 1 ~ max_worker_group_count
# worker_group_count = clamp unused worker_groups
# current_job_group_corresponds_to_N_worker_groups: N = poisson dist (LIMIT 0 < N <= worker_group_count)
# current_job_group_to_id_N_worker_group: N = uniform dist 1 ~ worker_group_count

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
job_groups = set()

import timeit

f = open("generated_sample.txt", "w+")
f.write(generated)
generated = ""

for job in range(job_count):
    jobs_left_to_iterate = job_count - 1 - job
    current_job_max_time_to_spend = whatever_dist_int(min_job_time_to_spend, max_job_time_to_spend, 1, wdist1)
    #print(timeit.timeit(lambda: whatever_dist_int(min_job_time_to_spend, max_job_time_to_spend, 1, wdist1), number = 1000))
    current_job_busyness_section_count = whatever_dist_int(1, current_job_max_time_to_spend / 2, 1, wdist2)
    #print(timeit.timeit(lambda: whatever_dist_int(1, current_job_max_time_to_spend / 2, 1, wdist2), number = 1000))
    current_job_busyness_values = dependant_dist_float(0, 1, current_job_busyness_section_count)
    #print(timeit.timeit(lambda: dependant_dist_float(0, 1, current_job_busyness_section_count), number = 1000))
    current_job_busyness_times = get_random_int(1, current_job_max_time_to_spend / current_job_busyness_section_count, current_job_busyness_section_count)
    #print(timeit.timeit(lambda: get_random_int(1, current_job_max_time_to_spend / current_job_busyness_section_count, current_job_busyness_section_count), number = 1000))
    current_job_ancestors_count = whatever_dist_int(0, jobs_left_to_iterate, 1, wdist4)
    #print(timeit.timeit(lambda: whatever_dist_int(0, jobs_left_to_iterate, 1, wdist4), number = 1000))
    current_job_ancestors = [(i + job + 1) for i in rng.choice(jobs_left_to_iterate, size=current_job_ancestors_count, replace=False)]
    #print(timeit.timeit(lambda: [(i + job + 1) for i in rng.choice(jobs_left_to_iterate, size=current_job_ancestors_count, replace=False)], number = 1000))
    current_job_group = whatever_dist_int(0, max_job_group_count - 1, 1, wdist3)
    #print(timeit.timeit(lambda: whatever_dist_int(0, max_job_group_count - 1, 1, wdist3), number = 1000))
    job_groups.add(current_job_group)
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
    if job % 50 == 0:
        f.write(generated)
        generated = ""
    print(job / job_count * 100, "%")
job_group_count = len(job_groups)
print(job_groups)

print(generated)
#f = open("generated_sample.txt", "w+")
f.write(generated)
f.close()