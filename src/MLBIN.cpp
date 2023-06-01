#include "MLBIN.h"





namespace Bin {


    MLBIN::MLBIN(int _method, double b0, double b1, int _n, int _sample_size, int _capacity, vector<int> _weight,
                const vector<double>& _dual_values, int _upper_col_limit):  b0{b0}, b1{b1}{
        
        dual_values = _dual_values;
        capacity = _capacity;
        weight = _weight;
        method = _method;
        nitems = _n;
        sample_size=_sample_size;
        upper_col_limit = _upper_col_limit;
        niterations = 1;
        best_rc_current_iteration = vector<double>(niterations);
        num_neg_rc_current_iteration = vector<long>(niterations);
        heur_best_reduced_cost = 1;
        
        max_dual = 0.;
        for (auto i = 0; i < nitems; i++){
            if (dual_values[i] > max_dual){
                max_dual = dual_values[i];
            }
        }
        compute_weight_ratio();
        identites = std::set<std::string>();
    }



    void MLBIN::random_sampling(){

    }



    void MLBIN::compute_weight_ratio(){
        weight_ratio = vector<double> (nitems, 0);
        for (auto i = 0; i < nitems; ++i){
            double weigth_r = weight[i]/capacity;
        }


    }

    void MLBIN::compute_ranking_based_measure(){

    }


    void MLBIN::compute_correlation_based_measure(){
        float sum_y = 0.0;
        for (auto i = 0; i < sample_size; ++i){
            sum_y += objs[i];
        }
        float mean_y = sum_y/sample_size;

        vector<float> diff_y = vector<float>(sample_size);

        float variance_y = 0.0, sum_diff_y = 0.0;
        for (auto i = 0; i < sample_size; ++i){
            diff_y[i] == objs[i] - mean_y;
            variance_y += diff_y[i] * diff_y[i];
            sum_diff_y += diff_y[i];
        }

        vector<float> mean_x(nitems, 0.0);
        vector<float> S1(nitems, 0.0);
        
        for (auto i = 0; i < sample_size; ++i){
            for (auto j = 0; j < pattern_set[i].size(); ++j){
                mean_x[pattern_set[i][j] += 1.0/sample_size];
                S1[pattern_set[i][j] += diff_y[i]];
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
                    auto tmp = variance_x[i]*variance_y;
                    if (tmp > 0)
                        corr_xy[i] = variance_xy[i]/sqrt(tmp);
                    else
                        corr_xy[i] = 0;
                }
                if (corr_xy[i] < min_cbm)
                    min_cbm = corr_xy[i];
                if (corr_xy[i] > max_cbm)
                    max_cbm = corr_xy[i];

                if (corr_xy[i]!=corr_xy[i]){
                    std::cout << variance_x[i] << " " << variance_y << "\n";
                }
        }

    }


    void MLBIN::make_prediction(int ith_iteration){

    }


    void MLBIN::run_iteration(int ith_iteraion){

    }











}