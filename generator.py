#1) job | worker | plan | job_group | worker_group | preference | look_ahead
#2) id (from 0 without skips) | preference: "SPT | LPT | FLS" | look_ahead: number
#3) job: time, busyness... | worker: plan_id | plan: start_at | job_group: start_after | worker_group: workers...
#4) job: ] | | plan: work, rest... | job_group: end_before |
#5) job: ancestors... | | | job_group: worker_group
#6) | | | job_group: jobs...

import random

# job count 10 ~ 10000

target_ancestors_distribution = random.uniform(0.0, 1.0)
# loop through every ancestor and add that ancestor if random_change(target_ancestors_distribution) is true
# also we can use normal distribution around that value from time to time

target_job_area_busyness_distribution = random.uniform(0, 1)


file = open('generated_sample.txt', 'w+')
line = " "
file.write(line)
file.close()