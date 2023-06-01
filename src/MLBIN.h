#ifndef MLBIN_H
#define MLBIN_H


#include <iostream>
#include <vector>
#include "pricer.h"
#include <set>
#include <numeric>
#include <cmath>
using std::vector;
using std::cout;
using std::endl;


namespace Bin{

    class MLBIN: public Pricer{

        std::vector<float> predicted_value;
        vector<vector<int>> pattern_set;
        std::vector<double> objs;
        double best_obj = 0.0;

        int method;
        int nitems;
        double max_dual;
        double b0;
        double b1;
        vector<double> dual_values;
        std::vector<double> weight_ratio;
        int capacity;
        vector<int> weight;
        vector<float> corr_xy;
        float min_cbm, max_cbm;

        public:

            std::set<std::string> identites;


            MLBIN(int _method, double b0, double b1, int _n, int _sample_size, int _capacity, vector<int> _weight,
                const vector<double>& _dual_values, int _upper_col_limit);

            void random_sampling(); // sampling columns
            void compute_weight_ratio();
            void compute_correlation_based_measure();
            void compute_ranking_based_measure();
            void make_prediction(int ith_iteration);

            void run_iteration(int ith_iteration);
    };



}












#endif