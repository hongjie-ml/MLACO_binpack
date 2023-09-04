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
        
        vector<double> times;
        vector<vector<int>> sols;
        vector<double> reduced_costs;
        vector<vector<bool>> adj_matrix;
        vector<vector<int>> adj_list;
        int method;
        
        int capacity;
        int nitems;
        vector<int> weight;

        vector<int> optimal_pattern;
        bool isOptimal=false;
        double best_obj;
        explicit knapsack_solver(int method, double _cutoff, const vector<double>& _dual_values, 
            int capacity, const vector<int>& _weight, int _nitems, 
            std::vector<std::vector<bool>> _adj_matrix, std::vector<std::vector<int>> _adj_list, long long _upper_col_limit);


        // void solve_knapsack_dp();
        void solve_knapsackGraph_gurobi();
        void solve_knapsackGraph_ACO();
        
        // MLBIN
        
        
        // More ML methods, like ML + ACO
        void solve_knapsackGraph_ACO_MLenhanced();

        
        void run();
        void postprocessing();
        ~knapsack_solver(){};




    };



}




#endif 
