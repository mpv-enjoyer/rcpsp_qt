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
bool Weights::are_valid(AlgorithmWeights& weights)
{
    static constexpr double EPSILON = 0.0001;
    for (auto name : WeightsNames)
    {
        if (!weights.count(name)) return false;
        auto& value = weights.at(name);
        if (std::abs(value) > 1 + EPSILON) return false;
        if (std::abs(value) > 1) value = 1;
    }
    return true;
    //double sum = 0;
    //for (auto key : WeightsNames) sum += std::abs(weights.at(key));
    //if (sum != 1) return false;
    //return true;
}
//AlgorithmWeights Weights::fix(AlgorithmWeights weights)
//{
//    static constexpr double EPSILON = 0.0001;
//    for (auto name : WeightsNames)
//    {
//        auto value = weights.at(name);
//        if (std::abs(value))
//    }
//    //double sum = 0;
//    //for (auto key : WeightsNames) sum += std::abs(weights.at(key));
//    //for (auto key : WeightsNames)
//    //{
//    //    weights[key] /= sum;
//    //}
//    //sum = 0; // further fix for double inaccuracy
//    //for (auto key : WeightsNames) sum += std::abs(weights.at(key));
//    //double diff = sum - 1;
//    //if (diff == 0) return weights;
//    //if (diff < 0)
//    //{
//    //    weights.at(*WeightsNames.begin()) -= diff;
//    //    return weights;
//    //}
//    //for (auto key : WeightsNames)
//    //{
//    //    if (weights.at(key) >= -1 + diff)
//    //    {
//    //        weights.at(key) -= diff;
//    //        return weights;
//    //    }
//    //}
//    //assert(false);
//}
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
    std::size_t worker_count = 0;
    for (const auto& worker_group : worker_groups)
    {
        worker_count += worker_group->get_size();
    }
    for (int i = 0; i < jobs->get_size(); i++)
    {
        int start = jobs->get_start();
        int end = jobs->get_end();
        JobPair new_pair = {start, end, jobs->get_job(i), worker_groups, static_cast<int>(_pending_jobs.size()), worker_count};
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

AlgorithmWeights Algorithm::get_weights()
{
    return _weights;
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

std::string Algorithm::get_string_result(const std::vector<ResultPair> &completed) const
{
    std::stringstream output;
    for (const auto resultpair : completed)
    {
        output <<
        "Job ID " << resultpair.job_id <<
        " min " << resultpair.job->get_start_after() <<
        " critical " << resultpair.job->get_critical_time() <<
        " max " << resultpair.job->get_end_before() <<
        " done by Worker ID " << resultpair.worker_internal_id <<
        " group ID " << resultpair.worker_group_id <<
        " start " << resultpair.start;
        if (resultpair.start + resultpair.job->get_time_to_spend() <= resultpair.job->get_end_before())
        {
            output << " [GOOD]";
        }
        else
        {
            output << " [BAD]";
        }
        output << "\n";
    }
    return output.str();
}

std::unordered_set<WorkerGroup *> Algorithm::get_worker_groups()
{
    std::unordered_set<WorkerGroup *> output;
    for (auto pending_job : _pending_jobs)
    {
        for (auto worker_group : pending_job.worker_groups)
        {
            if (output.count(worker_group) == 0)
            {
                output.insert(worker_group);
            }
        }
    }
    return output;
}

int Algorithm::get_pass_max_count() const
{
    return pass_max_count;
}

void Algorithm::set_pass_max_count(int count)
{
    if (count <= 0) throw std::invalid_argument("Invalid pass max count");
    pass_max_count = count;
}

#include <unordered_map>
#include <random>

void Algorithm::shuffle_pending_jobs()
{
    std::shuffle(std::begin(_pending_jobs), std::end(_pending_jobs), std::default_random_engine{});
}

int Algorithm::run()
{
    int best_failed_jobs = __INT_MAX__;
    std::vector<ResultPair> best_completed_jobs;
    int current_failed_jobs = 0;
    int current_equal_failed = 0;
    std::vector<ResultPair> current_completed_jobs;
    std::size_t worker_max_count = 0;

    for (const auto& pending_job : _pending_jobs)
    {
        worker_max_count = std::max(static_cast<double>(worker_max_count), pending_job.worker_count);
    }
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
            .max_critical_time = pending_jobs.get_max_critical_time(),
            .worker_max_count = worker_max_count
            };

            must_continue = false;
            must_continue |= pending_jobs.tick();
            must_continue |= pending_fronts.tick(for_weights);
            must_continue |= assigned_jobs.tick();
        }

        current_completed_jobs = completed_jobs.result();
        if (current_completed_jobs.size() != _pending_jobs.size()) throw std::invalid_argument("Lost some jobs");

        current_failed_jobs = completed_jobs.failed_count();
        if (current_failed_jobs == 0 || i + 1 >= pass_max_count)
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

    // Calculate penalty
    _penalty = 0;
    std::unordered_map<int, int> failed_groups; // job_group_id -> time
    for (auto& resultpair : _completed_jobs)
    {
        auto id = resultpair.job->get_global_group_id();
        if (!(resultpair.job->is_failed(resultpair.start))) continue;
        int failed_time = resultpair.start + resultpair.job->get_time_to_spend() - resultpair.job->get_end_before(); 
        failed_groups[id] = std::max(failed_groups[id], failed_time);
    }
    for (auto failed_group : failed_groups)
    {
        _penalty += failed_group.second;
    }
    return time_used;
}

std::vector<ResultPair> Algorithm::get_completed()
{
    return _completed_jobs;
}

std::size_t Algorithm::get_penalty() const
{
    return _penalty;
}

#include <unordered_set>

Algorithm::~Algorithm()
{
    //std::unordered_set<WorkerGroup*> worker_groups_deleted;
    //for (auto pending_job : _pending_jobs)
    //{
    //    delete pending_job.job;
    //    //for (auto worker_group : pending_job.worker_groups)
    //    //{
    //    //    if (worker_groups_deleted.count(worker_group) == 0)
    //    //    {
    //    //        worker_groups_deleted.insert(worker_group);
    //    //        delete worker_group;
    //    //    }
    //    //}        
    //}
    //for (auto completed_job : _completed_jobs)
    //{
    //    delete completed_job.job;
    //}
}

#include <cmath>

Stats::Stats(std::vector<ResultPair> completed, double precision, bool print_raw)
{
    init_coeff(wait_coeff, [](ResultPair r) -> double {
        double dedicated_time = r.job->get_end_before() - r.job->get_start_after();
        return double(r.start + r.job->get_time_to_spend() - r.job->get_start_after()) / double(dedicated_time);
    }, completed, precision);
    init_coeff(work_coeff, [](ResultPair r) -> double {
        double dedicated_time = r.job->get_end_before() - r.job->get_start_after();
        return double(r.job->get_time_to_spend()) / double(dedicated_time);
    }, completed, precision);
    for (auto pair : completed)
    {
        if (pair.job->get_critical_time() < pair.job->get_start_after())
        {
            leads_to_impossible_jobs_counter++;
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

void Stats::init_coeff(std::map<double, double> &coeff, std::function<double (ResultPair)> calculate_coeff, std::vector<ResultPair>& completed, double precision)
{
    std::map<int, int> unfiltered;
    for (auto job : completed)
    {
        // Ignore those jobs with infinite time to complete
        if (job.job->get_end_before() >= __INT_MAX__ - 1) continue;
        int id = (calculate_coeff(job) + precision / 2.0) / precision;
        unfiltered[id]++;
    }
    auto minmax = std::minmax_element(unfiltered.begin(), unfiltered.end());
    int min_x = minmax.first->first;
    int max_x = minmax.second->first;
    for (int i = min_x + 1; i <= max_x + 1; i++)
    {
        if (unfiltered.count(i) == 0) unfiltered[i] = 0;
    }

    for (auto current : unfiltered)
    {
        coeff[current.first * precision] = current.second;
    }
}



AlgorithmWeights Weights::create_empty()
{
    AlgorithmWeights w;
    for (const auto& name : Weights::WeightsNames)
    {
        w[name] = 0;
    }
    return w;
}
