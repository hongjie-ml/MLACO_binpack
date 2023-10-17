#ifndef ACO_H
#define ACO_H

#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <numeric> // iota
#include <vector>
#include <cstring>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <iomanip>
#include <omp.h>
#include <set>
#include "pricer.h"

namespace Bin
{
    using namespace std;

    class ACO : public Pricer
    {

        double T = 1.;
        double alpha = 1;
        double beta = 0.05;
        double rho = 1.;
        double delta_rho = 0.0002;
        double gamma = 0.5;
        double best_obj = 0.;

        int method;
        int method_type;
        int nitems;
        int capacity;
        vector<int> weight;
        const vector<vector<int>> adj_list;
        const vector<vector<bool>> adj_matrix;

        vector<int> best_sample;
        vector<vector<int>> pattern_set;
        vector<float> tau;
        vector<float> eta;

    public:
        double start_time;
        double max_dual;
        set<string> identites;
        double random_sampling_time;
        vector<double> objs;

        // Builds a solver for graph g.
        ACO(int _method, int _method_type, double _cutoff, int _n, int _nitems, int _sample_size, vector<int> weight, int capacity,
            const vector<double> &_dual_values, const vector<vector<bool>> &_adj_matrix, const vector<vector<int>> &_adj_list,
            int _upper_col_limit);
        void run_iteration(int ith_iteration);
        void run() override;
    };
}

#endif
