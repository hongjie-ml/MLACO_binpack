#include "MLBIN.h"

using std::vector;
using std::string;
#include <numeric>  


namespace Bin {

    // constructor for training
    MLBIN::MLBIN(int _method, double b0, double b1, int _n, int _sample_size, int _capacity, vector<int> _weight,
                const vector<double>& _dual_values, const vector<double>& _degree_norm, 
            const vector<vector<bool>>& _adj_matrix, const vector<vector<int>>& _adj_list,  int _upper_col_limit):  
            degree_norm{_degree_norm}, adj_matrix{_adj_matrix}, adj_list{_adj_list}, b0{b0}, b1{b1}, dual_values{_dual_values}
        {
        

        capacity = _capacity;
        weight = _weight;
        method = _method;
        nitems = _n;
        sample_size=_sample_size;
        upper_col_limit = _upper_col_limit;
        niterations = 10;
        best_rc_current_iteration = vector<double>(niterations);
        num_neg_rc_current_iteration = vector<long>(niterations);
        heur_best_reduced_cost = 1;

        max_dual = 0.;

        for (auto i = 0; i < nitems; ++i){
            if (dual_values[i] > max_dual){
                max_dual = dual_values[i];

            }
        }



        identites = std::set<std::string>();



 

    }

    // constructor for testing
    MLBIN::MLBIN(int _method, double _cutoff, int _n, int _sample_size, int _capacity, vector<int> _weight, 
            const vector<double>& _dual_values, const vector<double>& _degree_norm, 
            const vector<vector<bool>>& _adj_matrix, const vector<vector<int>>& _adj_list, int _upper_col_limit):  
            degree_norm{_degree_norm}, adj_matrix{_adj_matrix}, adj_list{_adj_list}, weight{_weight}{
            
        method = _method;
        cutoff = _cutoff;
        nitems = _n;
        dual_values = _dual_values;
        sample_size = _sample_size;
        capacity = _capacity;
        upper_col_limit = _upper_col_limit;
        niterations = 50;
        best_rc_current_iteration = vector<double>(niterations);
        num_neg_rc_current_iteration = vector<long>(niterations);
        heur_best_reduced_cost = 1;


        //read svm param files
        cout << "reading svm param..." << endl;
        std::ifstream svm_param_file("../svm.param");

        if (svm_param_file.good()){
            string line;
            
            getline(svm_param_file, line);
            // std::cout << line << endl;
            c0 = std::stod(line);
            // cout << c0 << endl;
            getline(svm_param_file, line);
            c1 = std::stod(line);
            getline(svm_param_file, line);
            c2 = std::stod(line);
            getline(svm_param_file, line);
            c3 = std::stod(line);
            getline(svm_param_file, line);
            c4 = std::stod(line);
            // getline(svm_param_file, line);
            // c5 = std::stod(line);
            // getline(svm_param_file, line);
            // c6 = std::stod(line);
            getline(svm_param_file, line);
            b = std::stod(line);
            svm_param_file.close();
            cout << c0 << " " << c1 << " " << c2 << " " << c3 << " " << c4 << " "<< b;
        }

        cutoff = 1e10;
        max_dual = 0.;
        for (int i = 0; i < nitems; i++){
            if (dual_values[i] > max_dual){
                max_dual = dual_values[i];
            }
        }
        identites = std::set<std::string>();

    
    }


    void MLBIN::random_sampling(){
        
        objs = vector<double> (sample_size);
        pattern_set = vector<vector<int>> (sample_size);
        long time_seed = current_time_for_seeding();    
        mt19937 mt(time_seed);
        uniform_int_distribution<int> dist(0,RAND_MAX);

        vector<int> candidates(nitems);
        int aggregated_weight;
        int nb_candidates, idx, item, num;

        for (int i = 0; i < sample_size; ++i){
            objs[i] = 0;
            nb_candidates = nitems;
            aggregated_weight = 0;
            for (int j = 0; j < nb_candidates; ++j){
                candidates[j] = j;
            }

            while (nb_candidates > 0){
                if (nb_candidates == nitems){
                    idx = i % nitems;
                } else{
                    idx = dist(mt) % nb_candidates;
                }
                item = candidates[idx]; // item number_id
                // cout << "dual value _size:" << dual_values.size() <<endl;
                // cout << "item_id:" << item << endl; 
                pattern_set[i].push_back(item);
                objs[i] += dual_values[item]; // number_id is the item idx in dual_values
                // cout << "dual values:" <<dual_values[item] << endl;
                aggregated_weight += weight[item];
                int remaining_capacity = capacity - aggregated_weight;
                num = 0;
                for (int j = 0; j < nb_candidates; ++ j){
                    if (remaining_capacity > weight[candidates[j]] && j != idx && adj_matrix[item][candidates[j]] == 1){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }
            
            std::sort(pattern_set[i].begin(), pattern_set[i].end());
            std::stringstream ss;
            for (int k = 0; k < pattern_set[i].size(); ++k){
                ss << pattern_set[i][k] << " ";
            }
            std::string identity = ss.str();
            if (identites.find(identity) == identites.end()){
                if (1 - objs[i] < THRESHOLD){
                    neg_rc_cols.push_back(pattern_set[i]);
                    neg_rc_vals.push_back(1-objs[i]);
                }
                identites.insert(identity);
            }
        }
    
        // cout << "negative rc col size :" << neg_rc_cols.size() << endl;
        // cout << "-------" << endl;
        

        


    }



    void MLBIN::compute_ranking_based_measure(){
        std::vector<int> sort_idx(sample_size);
        std::iota(sort_idx.begin(), sort_idx.end(), 0);
        std::vector<double> objs_copy(objs);
        std::sort(sort_idx.begin(), sort_idx.end(), [&objs_copy](int i1, int i2) {return objs_copy[i1] > objs_copy[i2];});
        std::vector<int> rank = std::vector<int>(sample_size);

        for (int i = 0; i < sample_size; ++i){
            rank[sort_idx[i]] = i;
        }

        std::vector<int> sampling_count(nitems,0);
        ranking_scores = std::vector<float>(sample_size, 0);
        for (auto i = 0; i < sample_size; ++i){
            for (auto j = 0; j < pattern_set[i].size(); ++j){
                sampling_count[pattern_set[i][j]]++;
                ranking_scores[pattern_set[i][j]] += 1.0/(rank[i]+1);
            }
        }


        float min_rmb=1.0;
        float max_rmb=0.;

        for (auto i = 0; i < nitems; ++i){
            if (ranking_scores[i] > max_rbm){
                max_rbm = ranking_scores[i];
            }
            if (ranking_scores[i] < min_rbm){
                min_rbm = ranking_scores[i];
            }
        }
        


    }


    void MLBIN::compute_correlation_based_measure(){
        
        float sum_y = 0.0;
        for (auto i = 0; i < sample_size; ++i){
            sum_y += objs[i];
        }
        float mean_y = sum_y/sample_size;

        vector<double> diff_y;
        
        float variance_y = 0.0, sum_diff_y = 0.0;
        for (auto i = 0; i < sample_size; ++i){
            
            diff_y.push_back(objs[i] - mean_y);
            variance_y += diff_y[i] * diff_y[i];
            sum_diff_y += diff_y[i];

        }
        
        vector<float> mean_x(nitems, 0.0);
        vector<float> S1(nitems, 0.0);
        
        for (auto i = 0; i < sample_size; ++i){
            for (auto j = 0; j < pattern_set[i].size(); ++j){
                mean_x[pattern_set[i][j]] += 1.0/sample_size;
                S1[pattern_set[i][j]] += diff_y[i];
            }

        }

        vector<float> variance_x(nitems, 0.0);
        vector<float> variance_xy(nitems, 0.0);
        corr_xy = vector<float> (nitems, 0.0);
        min_cbm = 1.0; max_cbm = -1.0;

        for (auto i = 0; i < nitems; ++i){
            variance_x[i] = mean_x[i]*(1 - mean_x[i])*sample_size;
            variance_xy[i] = (1-mean_x[i]) * S1[i] - mean_x[i] * (sum_diff_y - S1[i]);
                if (mean_x[i] == 0){
                    corr_xy[i] = -1;
                }
                if (mean_x[i] == 1){
                    corr_xy[i] = 1;
                }
                if (variance_x[i] > 0){
                    // cout << "variance_x:" << variance_x[i]<< endl;
                    // cout << "variance_y:" << variance_y << endl;

                    auto tmp = variance_x[i]*variance_y;
                    if (tmp > 0){
                        corr_xy[i] = variance_xy[i]/sqrt(tmp);              
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





    void MLBIN::make_prediction(int ith_iteration){

        
        compute_correlation_based_measure();
        
        compute_ranking_based_measure();
        
        predicted_value = std::vector<float>(nitems, 0);
        float projection;
        
        
        if (max_cbm == 0) max_cbm == 1;
        
        for (int i = 0; i < nitems; ++i){


            projection =
            c0 * dual_values[i]/max_dual +
            c1 * (float)weight[i]/(float)capacity +
            c2 * corr_xy[i]/max_cbm +
            c3 * ranking_scores[i]/max_rbm +
            c4 * degree_norm[i] + b;

            predicted_value[i] = 1.0/(1 + exp(-projection));


        }
   

    }


    void MLBIN::run_iteration(int ith_iteraion){
        
        long time_seed = current_time_for_seeding();
        
        mt19937 mt(time_seed);
        std::uniform_real_distribution<> dist(0., 1.);
        std::vector<double> distribution (nitems, 0);

        for (int idx=0; idx < sample_size; ++idx){
            if (get_wall_time() - start_time > cutoff){
                idx = sample_size; continue;

            }
            
            int nb_candidates = nitems;
            std::vector<int> candidates(nb_candidates);
            std::iota(candidates.begin(), candidates.end(), 0);
            int j, sel_idx;
            std::vector<int> sample;
            double sum=0;
            double obj=0;
            double r;
            double prob;
            int aggregated_weight = 0;

            while (nb_candidates >0){
                r = dist(mt);

                if (nb_candidates == nitems){
                    sel_idx = idx % nitems;

                } else
                {   
                    // calculate total predicted_value for each column
                    sum = 0.;
                    for (int i = 0; i < nb_candidates; i++){
                        sum += predicted_value[candidates[i]];
                    }

                    if (sum < 1e-8)
                    {
                        for (int i = 0; i < nitems; i++)
                            distribution[candidates[i]] = 1. / nb_candidates;
                    }
                    else{ 

                        // assign probability to each item in candidate
                        for (int i = 0; i < nitems; i++){
                            distribution[candidates[i]] = predicted_value[candidates[i]]/sum;
                            }
                    }
                    // cout << "prob: " << endl;
                    double max_prob = 0;
                    for (j = 0; j < nb_candidates; j++){
                        prob = distribution[candidates[j]];
                        // cout << prob << endl;
                        
                        if (prob > max_prob){
                            max_prob = prob;
                            sel_idx = j;
                        }

                        // if (prob > r){
                        //     r -= prob;
                        // }
                        // else
                        //     break;
                    }
                    // cout << "max prob " << max_prob << endl;
                    sel_idx = (j==nb_candidates) ? 0 : j;

                }

                auto sel_item = candidates[sel_idx];
                sample.push_back(sel_item);
                obj += dual_values[sel_item];
                aggregated_weight += weight[sel_item];
                int remaining_capacity = capacity - aggregated_weight;
                int num = 0;
                // const std::vector<bool>& neighbors = adj_matrix[sel_item];
                for (int j = 0; j < nb_candidates; ++ j){
                    if (remaining_capacity > weight[candidates[j]] && j != sel_idx && adj_matrix[sel_item][candidates[j]]==1){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }
            

            std::sort(sample.begin(), sample.end());
            std::stringstream ss;
            for (auto k = 0; k < sample.size(); k++){
                ss << sample[k] << " ";
                
            }
            std::string identity = ss.str();  
 
            if (best_obj < obj)
                best_obj = obj;
            
        
            bool duplicate = identites.find(identity) != identites.end();
            if (!duplicate && 1 - obj < THRESHOLD ){
                    neg_rc_cols.push_back(sample);
                    neg_rc_vals.push_back(1-obj);
                    identites.insert(identity);
            }

        }
        
        heur_best_reduced_cost = 1 - best_obj;

    }




    void MLBIN::run(){

        start_time = get_wall_time();
        random_sampling();
        
        cout << "Running MLBIN after random sampling...." << endl;
        
        for (auto i = 0; i < niterations; ++i){
            // cout << "the iter " << i << endl;
            if (i == 0){
                
                this->make_prediction(i);
                
            }
            
            this->run_iteration(i);
            int nrc_cols_cur_iteration = 0;
            for (auto idx = 0; idx < sample_size; ++idx){
                if (1 - objs[idx] < THRESHOLD){
                    nrc_cols_cur_iteration++;
                }
            }

            best_rc_current_iteration[i] = heur_best_reduced_cost;
            num_neg_rc_current_iteration[i] = nrc_cols_cur_iteration;

            if (get_wall_time() - start_time > cutoff)
                break;
            
        }
        compute_statistics();
        // cout << "dominate_selection_time: " << dominate_selection_time << "\n";
 
    }






}