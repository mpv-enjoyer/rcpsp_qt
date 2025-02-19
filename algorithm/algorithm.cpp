#include "algorithm.h"
#include <iostream>

double Weights::get(AlgorithmWeights weights, std::string name)
{
    if (WeightsNames.count(name) == 0) throw std::invalid_argument("Invalid weight name.");
    return weights.at(name);
}
bool Weights::set(AlgorithmWeights& weights, std::string name, double value)
{
    if (WeightsNames.count(name) == 0) return false;
    weights[name] = value;
    return true;
}
bool Weights::are_valid(AlgorithmWeights weights)
{
    double sum = 0;
    for (auto key : WeightsNames) sum += weights.at(key);
    if (sum != 1) return false;
    return true;
}
AlgorithmWeights Weights::fix(AlgorithmWeights weights)
{
    double sum = 0;
    for (auto key : WeightsNames) sum += weights.at(key);
    double diff = sum - 1;
    if (diff == 0) return weights;
    if (diff < 0)
    {
        weights.at(*WeightsNames.begin()) -= diff;
        return weights;
    }
    for (auto key : WeightsNames)
    {
        if (weights.at(key) >= diff)
        {
            weights.at(key) -= diff;
            return weights;
        }
    }
    assert(false);
}
std::string Weights::to_string(AlgorithmWeights weights)
{
    std::string output = "Weights: ";
    for (auto key : WeightsNames)
    {
        output.append("[" + key + "]=");
        output.append(std::to_string(weights.at(key)));
        output.append(" ");
    }
    return output;
}

AlgorithmWeights Weights::create_equal()
{
    AlgorithmWeights output;
    for (auto name : WeightsNames)
    {
        output[name] = 1.0 / SIZE;
    }
    return output;
}

Algorithm::Algorithm()
{

}

void Algorithm::add_job_group(JobGroup* jobs, std::vector<WorkerGroup*> worker_groups)
{
    for (int i = 0; i < jobs->get_size(); i++)
    {
        int start = jobs->get_start();
        int end = jobs->get_end();
        JobPair new_pair = {start, end, jobs->get_job(i), worker_groups, static_cast<int>(_pending_jobs.size())};
        _pending_jobs.push_back(new_pair);
    }
    for (auto workers : worker_groups)
    {
        for (int i = 0; i < workers->get_size(); i++)
        {
            if (workers->get_worker(i)->get_plan().get_time_loop() > longest_plan_loop)
            {
                longest_plan_loop = workers->get_worker(i)->get_plan().get_time_loop();
            }
        }
    }
}

void Algorithm::set_preference(Preference new_preference)
{
    preference = new_preference;
}

bool compare_result(ResultPair& lhs, ResultPair& rhs)
{
    return (lhs.job->get_global_id() > rhs.job->get_global_id());
}

int Algorithm::get_look_ahead_time() const
{
    return look_ahead_time;
}

Preference Algorithm::get_preference() const
{
    return preference;
}

void Algorithm::set_look_ahead_time(int newLook_ahead_time)
{
    look_ahead_time = newLook_ahead_time;
}

void Algorithm::set_weights(AlgorithmWeights weights)
{
    _weights = weights;
}

void Algorithm::reset()
{
    for (auto job : _completed_jobs)
    {
        job.job->undone();
        job.worker->undone();
        job.job->set_preference_coefficient(0);
    }
}

int Algorithm::get_failed_jobs_count()
{
    return _failed_jobs_count;
}

int Algorithm::run()
{
    int best_failed_jobs = __INT_MAX__;
    std::vector<ResultPair> best_completed_jobs;
    int current_failed_jobs = 0;
    int current_equal_failed = 0;
    std::vector<ResultPair> current_completed_jobs;
    for (int i = 0; ; i++)
    {
        int current_time = 0;
        CompletedJobs completed_jobs;
        AssignedJobs assigned_jobs = AssignedJobs(&current_time, &completed_jobs);
        PendingFronts pending_fronts = PendingFronts(&current_time, &assigned_jobs, preference, look_ahead_time, _weights);
        PendingJobs pending_jobs = PendingJobs(&current_time, &pending_fronts, look_ahead_time, _pending_jobs);
        pending_fronts.update_time_to_front();

        bool must_continue = true;
        while (must_continue)
        {
            AlgorithmDataForWeights for_weights =
            {
            .job_count_not_assigned = pending_jobs.data_size(),
            .job_count_overall = _pending_jobs.size(),
            .max_critical_time = pending_jobs.get_max_critical_time()
            };

            must_continue = false;
            must_continue |= pending_jobs.tick();
            must_continue |= pending_fronts.tick(for_weights);
            must_continue |= assigned_jobs.tick();
        }

        current_completed_jobs = completed_jobs.result();
        if (current_completed_jobs.size() != _pending_jobs.size()) throw std::invalid_argument("Lost some jobs"); // Lost some jobs

        current_failed_jobs = completed_jobs.failed_count();
        if (current_failed_jobs == 0 || i + 1 >= PASS_MAX_COUNT)
        {
            best_failed_jobs = current_failed_jobs;
            best_completed_jobs = current_completed_jobs;
            break;
        }
        if (current_failed_jobs > best_failed_jobs)
        {
            // Don't set to a worse value
            break;
        }
        if (current_failed_jobs == best_failed_jobs)
        {
            current_equal_failed++;
            if (current_equal_failed > CURRENT_EQUAL_MAX)
            {
                best_completed_jobs = current_completed_jobs;
                break;
            }
        }
        current_equal_failed = 0;
        best_failed_jobs = current_failed_jobs;
        completed_jobs.prepare_for_next_iteration();
        std::cout << " [REITERATE] current failed job count:" << best_failed_jobs << "with" << completed_jobs.result().size() << "completed\n";
    }
    if (best_failed_jobs != __INT_MAX__) // Does it ever occur?
        _completed_jobs = best_completed_jobs;
    std::sort(_completed_jobs.begin(), _completed_jobs.end(), compare_result);
    int time_used = 0;
    for (int i = 0; i < _completed_jobs.size(); i++)
    {
        _completed_jobs[i].job_id = _completed_jobs[i].job->get_global_id();
        time_used = std::max(time_used, _completed_jobs[i].start + _completed_jobs[i].job->get_time_to_spend());
    }
    qDebug() << "final failed job count:" << best_failed_jobs << "with" << _completed_jobs.size() << "completed";
    _failed_jobs_count = best_failed_jobs;
    return time_used;
}

std::vector<ResultPair> Algorithm::get_completed()
{
    return _completed_jobs;
}

Stats::Stats(std::vector<ResultPair> completed, double precision, bool print_raw)
{
    //for (double x_id = 0; x_id * precision < 1; x_id++)
    //{
    //    double x = x_id * precision;
    //    wait_coeff[x] = 0;
    //    work_coeff[x] = 0;
    //}
    std::map<double, double> unfiltered_wait_coeff;
    std::map<double, double> unfiltered_work_coeff;
    for (auto job : completed)
    {
        double dedicated_time = job.job->get_end_before() - job.job->get_start_after();
        double wait_coeff = double(job.start - job.job->get_start_after()) / double(dedicated_time);
        double work_coeff = double(job.job->get_time_to_spend()) / double(dedicated_time);
        unfiltered_wait_coeff[wait_coeff]++;
        unfiltered_work_coeff[work_coeff]++;
    }

    qDebug() << "UWC calculated\n";

    double last_wait_x = (--(unfiltered_wait_coeff.end()))->first;
    for (double x_id = 0; (x_id - 1) * precision < last_wait_x; x_id++)
    {
        double x = x_id * precision;
        wait_coeff[x] = 0;
    }

    double last_work_x = (--(unfiltered_work_coeff.end()))->first;
    for (double x_id = 0; (x_id - 1) * precision < last_work_x; x_id++)
    {
        double x = x_id * precision;
        work_coeff[x] = 0;
    }

    qDebug() << "coeff calculated " << last_wait_x << last_work_x << wait_coeff.size() << work_coeff.size() << " \n";

    auto unfiltered_wait_coeff_iter = unfiltered_wait_coeff.begin();
    auto unfiltered_wait_coeff_size = unfiltered_wait_coeff.size();
    for (auto& pair : wait_coeff)
    {
        double x = pair.first;
        for (; unfiltered_wait_coeff_iter != unfiltered_wait_coeff.end() && std::abs(unfiltered_wait_coeff_iter->first - x) < (precision / 2.0); ++unfiltered_wait_coeff_iter)
        {
            pair.second++;
            unfiltered_wait_coeff_size--;
        }
    }
    if (unfiltered_wait_coeff_size != 0)
    {
        throw std::logic_error("unfiltered wait coeff size != 0");
        //FIXME: this is bugged for 1MB example
    }
    auto unfiltered_work_coeff_iter = unfiltered_work_coeff.begin();
    auto unfiltered_work_coeff_size = unfiltered_work_coeff.size();
    for (auto& pair : work_coeff)
    {
        double x = pair.first;
        for (; unfiltered_work_coeff_iter != unfiltered_work_coeff.end() && std::abs(unfiltered_work_coeff_iter->first - x) < (precision / 2.0); ++unfiltered_work_coeff_iter)
        {
            pair.second++;
            unfiltered_work_coeff_size--;
        }
    }
    if (unfiltered_work_coeff_size != 0)
    {
        throw std::logic_error("unfiltered work coeff size != 0");
    }

    qDebug() << "Stats calculated";
    if (print_raw)
    {
        qDebug() << "STATS RAW (wait_coeff): \n";
        for (auto point : unfiltered_wait_coeff)
        {
            qDebug() << "X = " << point.first << ", Y = " << point.second << "\n";
        }
        qDebug() << "STATS RAW (work_coeff): \n";
        for (auto point : unfiltered_work_coeff)
        {
            qDebug() << "X = " << point.first << ", Y = " << point.second << "\n";
        }
    }
}

void Stats::print()
{
    qDebug() << "STATS (wait_coeff): \n";
    for (auto point : wait_coeff)
    {
        qDebug() << "X = " << point.first << ", Y = " << point.second << "\n";
    }
    qDebug() << "STATS (work_coeff): \n";
    for (auto point : work_coeff)
    {
        qDebug() << "X = " << point.first << ", Y = " << point.second << "\n";
    }
}
