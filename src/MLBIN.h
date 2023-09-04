#ifndef MLBIN_H
#define MLBIN_H


#include <iostream>
#include <vector>
#include "pricer.h"
#include <set>
#include <numeric>

#include <cmath>
#include <random>
#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>

using std::vector;
using std::cout;
using std::endl;
using std::mt19937;
using std::uniform_int_distribution;

namespace Bin{

    class MLBIN: public Pricer{
        double THRESHOLD = -0.000001;
        std::vector<float> predicted_value;
        vector<vector<int>> pattern_set;
        std::vector<double> objs;
        const vector<vector<int>> adj_list;
        const vector<vector<bool>> adj_matrix;
        const vector<double> degree_norm;
        double best_obj = 0.0;



        int method;
        int nitems;
        
        double b0;
        double b1;



        
        

        public:
            int capacity;
            vector<int> weight;
            double max_dual;
            double start_time;
            std::set<std::string> identites;
            std::vector<double> weight_ratio;
            vector<double> dual_values;
            float min_cbm, max_cbm;
            float min_rbm, max_rbm;
            vector<float> corr_xy;
            vector<float> ranking_scores; 
            MLBIN(int _method, double b0, double b1, int _n, int _sample_size, int _capacity, vector<int> _weight,
                const vector<double>& _dual_values, const vector<double>& _degree_norm, 
                const vector<vector<bool>>& _adj_matrix, const vector<vector<int>>& _adj_list, int _upper_col_limit);

            MLBIN(int _method, double _cutoff, int _n, int _sample_size, int _capacity, vector<int> _weight, 
                const vector<double>& _dual_values, const vector<double>& _degree_norm, 
            const vector<vector<bool>>& _adj_matrix, const vector<vector<int>>& _adj_list, int _upper_col_limit);

            void random_sampling(); // sampling columns
            // void compute_weight_ratio();
            void compute_correlation_based_measure();
            void compute_ranking_based_measure();
            void compute_adjacent_weight();
            void make_prediction(int ith_iteration);

            void run_iteration(int ith_iteration);
            void run();
    };



}












#endif