#include "ACO.h"
#include <cmath>
#include <limits>
#include <sys/time.h>
#include <cassert>
#include <math.h> 
#include <sstream>

using std::mt19937;
using std::cout;
using std::endl;
using std::vector;


namespace Bin{


    ACO::ACO(int _method, double _cutoff, int _n, int _nitems, int _sample_size, 
                vector<int> _weight, int _capacity, const vector<double>& _dual_values,
                const vector<vector<bool>>& _adj_matrix, const vector<vector<int>>& _adj_list
                ,int _upper_col_limit): nitems{_nitems}, adj_list{_adj_list}, adj_matrix{_adj_matrix}
                {
                    dual_values = _dual_values;
                    method = _method;
                    capacity = _capacity;
                    weight = _weight;
                    sample_size = _sample_size;
                    upper_col_limit = _upper_col_limit;
                    niterations = floor(nitems*5/3);
                    // cout << "ACO starting.. iteration:" << niterations << endl;
                    best_rc_current_iteration = vector<double>(niterations);
                    num_neg_rc_current_iteration = vector<long>(niterations);
                    heur_best_reduced_cost = 1;
               
                
                }


    void ACO::run(){
        
        
        start_time=get_wall_time();
        objs = vector<double> (sample_size);
        pattern_set = vector<vector<int>> (sample_size);
        tau = vector<double>(nitems, 1.);

        
        for (auto i = 0; i < nitems; i++)
            tau[i] += dual_values[i];
        
        
        for (auto i = 0; i < niterations; ++i){
            
            this->run_iteration(i);
            
            num_neg_rc_col = 0;
            for (auto idx = 0; idx < sample_size; idx++){
                if (1 - objs[idx] < -1e-6){
                    num_neg_rc_col++;
                }
            }
            best_rc_current_iteration[i] = heur_best_reduced_cost;
            num_neg_rc_current_iteration[i] = num_neg_rc_col;
            if (get_wall_time() - start_time > cutoff)
                break;

            // update dynamic parameters
            if (i < 100) alpha = 1;
            else if (i < 400) alpha = 2;
            else if (i < 800) alpha = 3;
            else alpha = 4;

            T = (1-beta) * T;
            if (rho > 0.95)
                rho = (1-delta_rho) * rho;
            else
                rho = 0.95;
        }
        
        compute_statistics();

    }

 

    void ACO::run_iteration(int ith_iteration){
        long time_seed = current_time_for_seeding();    
        vector<double> delta_tau(nitems, 0.0);

        mt19937 mt(time_seed);
        uniform_real_distribution<> dist(0., 1.);
        std::vector<double> distribution (nitems, 0);
        int aggregated_weight;
        
        
        for (auto idx = 0; idx < sample_size; ++idx){
            if (get_wall_time() - start_time > cutoff){
                idx=sample_size; continue;
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
            while (nb_candidates > 0){
                r = dist(mt);
                if (nb_candidates == nitems){
                    sel_idx = idx % nitems;
                }
                else{
                    sum = 0.;
                    for (int i = 0; i < nb_candidates; ++i){
                        sum += pow(tau[candidates[i]], alpha);
                    }
                    if (sum < 1e-8){
                        for (int i = 0; i < nitems; ++i){
                            distribution[candidates[i]] = 1. / nb_candidates;
                     }   
                    }
                    else {
                        for (int i = 0; i < nitems; ++i){
                            distribution[candidates[i]] = pow(tau[candidates[i]], alpha) / sum;
                        }
                    }

                    for (j = 0; j < nb_candidates; ++j){
                        prob = distribution[candidates[j]];
                        if (r > prob)
                            r -= prob;
                        else
                            break;
                    }
                    sel_idx = (j==nb_candidates) ? 0 : j;
                    // cout << "sel idx" << sel_idx << endl;
                }
                
                auto sel_item = candidates[sel_idx];
                sample.push_back(sel_item);
                obj += dual_values[sel_item];
                
                aggregated_weight += weight[sel_item];

                int remaining_capacity = capacity - aggregated_weight;
                int num = 0; 
                
                for (int j = 0; j < nb_candidates; ++ j){
                    if (remaining_capacity > weight[candidates[j]] && j != sel_idx && adj_matrix[sel_item][candidates[j]] == 1){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }

            objs[idx] = obj;
            pattern_set[idx].resize(sample.size());
            std::copy(sample.begin(), sample.end(), pattern_set[idx].begin());
            if (1 - obj < -1e-9){
                std::sort (sample.begin(), sample.end());
                std::stringstream ss;

                for (auto k = 0; k < sample.size(); ++k){
                    ss << sample[k] << " ";
                }
                
                std::string identity = ss.str();


                if (identites.find(identity) == identites.end()){
                    identites.insert(identity);
                    neg_rc_cols.push_back(sample);
                    neg_rc_vals.push_back(1-obj);
                }
            
            }
            
            if (best_obj < obj){
                best_obj = obj;
            }

            for (auto k = 0; k < sample.size(); ++k){
                delta_tau[sample[k]] += 1/(1+1000 * (best_obj-obj));
            }
        }        


        for (auto k = 0; k < nitems; ++k){
                tau[k] = rho * tau[k] + delta_tau[k];
            }
        heur_best_reduced_cost = 1-best_obj;



    }


    
   

}