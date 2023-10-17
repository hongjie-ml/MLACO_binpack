#include "mlaco.h"
#include <cmath>
#include <limits>
#include <sys/time.h>
#include <cassert>
#include <math.h>
#include <sstream>
#include <fstream>
using std::cout;
using std::endl;
using std::mt19937;
using std::vector;

namespace Bin
{

    MLACO::MLACO(int _method, int _method_type, int _seed, double _cutoff, int _n, int _nitems, int _sample_size,
                 vector<int> _weight, int _capacity, const vector<double> &_dual_values, const vector<double> &_degree_norm,
                 const vector<vector<bool>> &_adj_matrix, const vector<vector<int>> &_adj_list, int _upper_col_limit) : degree_norm{_degree_norm}, nitems{_nitems}, adj_list{_adj_list}, adj_matrix{_adj_matrix}
    {
        dual_values = _dual_values;
        method = _method;
        method_type = _method_type;
        seed = _seed;
        cutoff = _cutoff;
        capacity = _capacity;
        weight = _weight;
        sample_size = _sample_size;
        upper_col_limit = _upper_col_limit;
        niterations = 10;
        cout << "ACO starting.. iteration:" << niterations << endl;

        best_rc_current_iteration = vector<double>(niterations);
        num_neg_rc_current_iteration = vector<long>(niterations);
        heur_best_reduced_cost = 1;

        // read svm param files
        cout << "reading svm param..." << endl;
        std::ifstream svm_param_file("../svm.param");

        if (svm_param_file.good())
        {
            string line;

            getline(svm_param_file, line);
            c0 = std::stod(line);
            getline(svm_param_file, line);
            c1 = std::stod(line);
            getline(svm_param_file, line);
            c2 = std::stod(line);
            getline(svm_param_file, line);
            c3 = std::stod(line);
            getline(svm_param_file, line);
            b = std::stod(line);
            svm_param_file.close();
            cout << c0 << " " << c1 << " " << c2 << " " << c3 << " " << c4 << " " << b << endl;
            ;
        }

        max_dual = 0.;
        for (int i = 0; i < nitems; i++)
        {
            if (dual_values[i] > max_dual)
            {
                max_dual = dual_values[i];
            }
        }
        identites = std::set<std::string>();
    }

    void MLACO::make_prediction(int ith_iteration)
    {

        compute_correlation_based_measure();

        compute_ranking_based_measure();

        predicted_value = std::vector<float>(nitems, 0);
        float projection;
        float min_cbm, max_cbm;
        float min_rbm, max_rbm;
        if (max_cbm == 0)
            max_cbm = 1;

        for (int i = 0; i < nitems; ++i)
        {

            projection =
                c0 * dual_values[i] / max_dual +
                c1 * (float)weight[i] / (float)capacity +
                c2 * corr_xy[i] / max_cbm +
                c3 * ranking_scores[i] / max_rbm +
                c4 * degree_norm[i] + b;

            predicted_value[i] = 1.0 / (1 + exp(-projection));
        }
    }

    void MLACO::random_sampling()
    {

        objs = vector<double>(sample_size);
        sample_set = vector<vector<int>>(sample_size);
        long time_seed = current_time_for_seeding();
        mt19937 mt(time_seed);
        uniform_int_distribution<int> dist(0, RAND_MAX);

        vector<int> candidates(nitems);
        int aggregated_weight;
        int nb_candidates, idx, item, num;

        for (int i = 0; i < sample_size; ++i)
        {
            objs[i] = 0;
            nb_candidates = nitems;
            aggregated_weight = 0;
            for (int j = 0; j < nb_candidates; ++j)
            {
                candidates[j] = j;
            }

            while (nb_candidates > 0)
            {
                if (nb_candidates == nitems)
                {
                    idx = i % nitems;
                }
                else
                {
                    idx = dist(mt) % nb_candidates;
                }
                item = candidates[idx]; // item number_id
                // cout << "dual value _size:" << dual_values.size() <<endl;
                // cout << "item_id:" << item << endl;
                sample_set[i].push_back(item);
                objs[i] += dual_values[item]; // number_id is the item idx in dual_values
                // cout << "dual values:" <<dual_values[item] << endl;
                aggregated_weight += weight[item];
                int remaining_capacity = capacity - aggregated_weight;
                num = 0;
                for (int j = 0; j < nb_candidates; ++j)
                {
                    if (remaining_capacity > weight[candidates[j]] && j != idx && adj_matrix[item][candidates[j]] == 0)
                    {
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }

            std::sort(sample_set[i].begin(), sample_set[i].end());

            // std::stringstream ss;
            // for (int k = 0; k < pattern_set[i].size(); ++k)
            // {
            //     ss << pattern_set[i][k] << " ";
            // }
            // std::string identity = ss.str();
            // if (identites.find(identity) == identites.end())
            // {
            //     identites.insert(identity);
            // }
        }
    }

    void MLACO::run()
    {

        start_time = get_wall_time();
        objs = vector<double>(sample_size);
        pattern_set = vector<vector<int>>(sample_size);

        if (method_type == 0)
        {
        }
        else if (method_type == 1)
        {
            eta = vector<float>(nitems, 1.);
        }
        else if (method_type == 2)
        {
        }
        else if (method_type == 3)
        {
            eta = vector<float>(nitems, 1.);
            for (auto k = 0; k < nitems; k++)
            {
                eta[k] = dual_values[k] / weight[k];
            }
        }

        random_sampling();

        for (auto i = 0; i < niterations; ++i)
        {
            if (i == 0)
            {
                this->make_prediction(i);
                if (method_type == 0)
                {
                    eta = predicted_value;
                    tau = vector<float>(nitems, 1.);
                }
                else if (method_type == 1)
                {
                    tau = predicted_value;
                }
                else if (method_type == 2)
                {
                    eta = predicted_value;
                    tau = predicted_value;
                }
                else if (method_type == 3)
                {
                    tau = predicted_value;
                }
            }

            
            this->run_iteration(i);
            // cout << "number of iter" << i << " done " << endl;
            // cout << sample_size << endl;
            int num_neg_rc_col_iter = 0;
            for (auto idx = 0; idx < sample_size; idx++)
            {
                if (1 - objs[idx] < -1e-6)
                {
                    num_neg_rc_col_iter++;
                }
            }

            best_rc_current_iteration[i] = heur_best_reduced_cost;
            num_neg_rc_current_iteration[i] = num_neg_rc_col_iter;
            // if (get_wall_time() - start_time > cutoff)
            //     break;

            // update dynamic parameters
            if (i < 100)
                alpha = 1;
            else if (i < 400)
                alpha = 2;
            else if (i < 800)
                alpha = 3;
            else
                alpha = 4;

            T = (1 - beta) * T;
            if (rho > 0.95)
                rho = (1 - delta_rho) * rho;
            else
                rho = 0.95;
        }

        compute_statistics();
    }

    void MLACO::run_iteration(int ith_iteration)
    {

        // cout << "run iteration: " << ith_iteration << endl;
        // cout << "sample size " << sample_size << endl;
        vector<double> delta_tau(nitems, 0.0);

        mt19937 mt(seed);
        uniform_real_distribution<> dist(0., 1.);
        std::vector<double> distribution(nitems, 0);
        int aggregated_weight;

        for (auto idx = 0; idx < sample_size; ++idx)
        {
            if (get_wall_time() - start_time > cutoff)
            {
                cout << cutoff  << endl;
                idx = sample_size;
                continue;
            }

            int nb_candidates = nitems;
            std::vector<int> candidates(nb_candidates);
            std::iota(candidates.begin(), candidates.end(), 0);
            int j, sel_idx;
            std::vector<int> sample;
            double sum = 0.;
            double obj = 0.;
            double r;
            double prob;
            aggregated_weight = 0;
            while (nb_candidates > 0)
            {
                r = dist(mt);
                if (nb_candidates == nitems)
                {
                    sel_idx = idx % nitems;
                }
                else
                {
                    sum = 0.;
                    for (int i = 0; i < nb_candidates; ++i)
                    {
                        sum += pow(tau[candidates[i]], alpha) * pow(eta[candidates[i]], beta);
                    }
                    if (sum < 1e-8)
                    {
                        for (int i = 0; i < nitems; ++i)
                        {
                            distribution[candidates[i]] = 1. / nb_candidates;
                        }
                    }
                    else
                    {
                        for (int i = 0; i < nitems; ++i)
                        {
                            distribution[candidates[i]] = pow(tau[candidates[i]], alpha) * pow(eta[candidates[i]], beta) / sum;
                            // distribution[candidates[i]] = pow(tau[candidates[i]], alpha) / sum;
                        }
                    }

                    for (j = 0; j < nb_candidates; ++j)
                    {
                        prob = distribution[candidates[j]];
                        if (r > prob)
                            r -= prob;
                        else
                            break;
                    }
                    sel_idx = (j == nb_candidates) ? 0 : j;
                }

                auto sel_item = candidates[sel_idx];
                sample.push_back(sel_item);
                obj += dual_values[sel_item];

                aggregated_weight += weight[sel_item];

                int remaining_capacity = capacity - aggregated_weight;
                int num = 0;

                for (int j = 0; j < nb_candidates; ++j)
                {
                    if (remaining_capacity > weight[candidates[j]] && j != sel_idx && adj_matrix[sel_item][candidates[j]] == 0)
                    {
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }

            objs[idx] = obj;
            cout <<" obj:" << obj << endl;
            pattern_set[idx].resize(sample.size());
            std::copy(sample.begin(), sample.end(), pattern_set[idx].begin());
            

            if (1 - obj < -1e-9)
            {
                std::sort(sample.begin(), sample.end());
                std::stringstream ss;

                for (auto k = 0; k < sample.size(); ++k)
                {
                    ss << sample[k] << " ";
                }

                std::string identity = ss.str();

                if (identites.find(identity) == identites.end())
                {
                    identites.insert(identity);
                    neg_rc_cols.push_back(sample);
                    neg_rc_vals.push_back(1 - obj);
                }
            }

            if (best_obj > obj)
            {
                best_obj = obj;
                best_sample = sample;
            }

            for (auto k = 0; k < sample.size(); ++k)
            {
                for (auto j = 0; j < best_sample.size(); ++j)
                {
                    if (sample[k] != best_sample[j])
                    {
                        // sample[k] selected item index
                        delta_tau[sample[k]] += 1 / (best_obj - obj);
                    }
                    else
                    {
                        delta_tau[best_sample[j]] += 1 / best_obj;
                    }
                }
            }
        }


        for (auto k = 0; k < nitems; ++k)
        {
            tau[k] = rho * tau[k] + delta_tau[k];
        }
        heur_best_reduced_cost = 1 - best_obj;
    }

    void MLACO::compute_ranking_based_measure()
    {
        std::vector<int> sort_idx(sample_size);
        std::iota(sort_idx.begin(), sort_idx.end(), 0);
        std::vector<double> objs_copy(objs);
        std::sort(sort_idx.begin(), sort_idx.end(), [&objs_copy](int i1, int i2)
                  { return objs_copy[i1] > objs_copy[i2]; });
        std::vector<int> rank = std::vector<int>(sample_size);

        for (int i = 0; i < sample_size; ++i)
        {
            rank[sort_idx[i]] = i;
        }

        // cout << "sample_set" << endl;
        // for (auto i = 0; i < sample_set.size(); ++i)
        // {
        //     for (auto j = 0; j < sample_set[i].size(); ++j)
        //     {
        //         cout << sample_set[i][j] << " ";
        //     }
        //     cout << endl;
        // }

        std::vector<int> sampling_count(nitems, 0);
        ranking_scores = std::vector<float>(sample_size, 0);
        for (auto i = 0; i < sample_size; ++i)
        {
            for (auto j = 0; j < sample_set[i].size(); ++j)
            {
                sampling_count[sample_set[i][j]]++;
                ranking_scores[sample_set[i][j]] += 1.0 / (rank[i] + 1);
            }
        }

        float min_rmb = 1.0;
        float max_rmb = 0.;

        for (auto i = 0; i < nitems; ++i)
        {
            if (ranking_scores[i] > max_rbm)
            {
                max_rbm = ranking_scores[i];
            }
            if (ranking_scores[i] < min_rbm)
            {
                min_rbm = ranking_scores[i];
            }
        }
    }

    void MLACO::compute_correlation_based_measure()
    {

        float sum_y = 0.0;
        for (auto i = 0; i < sample_size; ++i)
        {
            sum_y += objs[i];
        }
        // cout << "sum_y: " << sum_y <<  endl;
        float mean_y = sum_y / sample_size;

        vector<double> diff_y;

        float variance_y = 0.0, sum_diff_y = 0.0;
        for (auto i = 0; i < sample_size; ++i)
        {

            diff_y.push_back(objs[i] - mean_y);
            variance_y += diff_y[i] * diff_y[i];
            sum_diff_y += diff_y[i];
        }

        vector<float> mean_x(nitems, 0.0);
        vector<float> S1(nitems, 0.0);

        for (auto i = 0; i < sample_size; ++i)
        {
            for (auto j = 0; j < sample_set[i].size(); ++j)
            {
                mean_x[sample_set[i][j]] += 1.0 / sample_size;
                S1[sample_set[i][j]] += diff_y[i];
            }
        }

        vector<float> variance_x(nitems, 0.0);
        vector<float> variance_xy(nitems, 0.0);
        corr_xy = vector<float>(nitems, 0.0);
        min_cbm = 1.0;
        max_cbm = -1.0;

        for (auto i = 0; i < nitems; ++i)
        {
            variance_x[i] = mean_x[i] * (1 - mean_x[i]) * sample_size;
            variance_xy[i] = (1 - mean_x[i]) * S1[i] - mean_x[i] * (sum_diff_y - S1[i]);
            if (mean_x[i] == 0)
            {
                corr_xy[i] = -1;
            }
            if (mean_x[i] == 1)
            {
                corr_xy[i] = 1;
            }
            if (variance_x[i] > 0)
            {

                auto tmp = variance_x[i] * variance_y;
                if (tmp > 0)
                {
                    corr_xy[i] = variance_xy[i] / sqrt(tmp);
                }
                else
                    corr_xy[i] = 0;
            }
            if (corr_xy[i] < min_cbm)
                min_cbm = corr_xy[i];
            if (corr_xy[i] > max_cbm)
                max_cbm = corr_xy[i];
        }
    }

}