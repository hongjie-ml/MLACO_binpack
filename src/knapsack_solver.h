#ifndef KNAPSACK_SOLVER_H
#define KNAPSACK_SOLVER_H

#include <vector>
#include <iostream>
#include "pricer.h"


using std::cout;
using std::endl;
using std::vector;


namespace Bin{

    class knapsack_solver : public Pricer{

    
    public:
        double exact_rc;
        // const vector<int>& knapsack_pattern;




        vector<double> objs;
        vector<double> times;
        vector<vector<int>> sols;
        vector<double> reduced_costs;

        int capacity;
        int nitems;
        vector<double> weight;

        vector<int> optimal_pattern;
        bool isOptimal=false;
        double best_obj;
        explicit knapsack_solver(int _method, double _cutoff, const vector<double>& _dual_values, 
            int capacity, vector<int> weight, int nitems, long long _upper_col_limit);

        void solve_knapsack_greedy();
        void solve_knapsack_dp();
        void postprocessing();





    };



}










#endif 
