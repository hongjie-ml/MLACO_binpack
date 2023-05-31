#include "knapsack_solver.h"



namespace Bin{


    knapsack_solver::knapsack_solver(double _cutoff, const vector<double>& _dual_values, 
                int _capacity, const vector<int>& _weight, int _nitems, long long _upper_col_limit) 
    {   
        dual_values = _dual_values;
        cutoff = _cutoff;
        capacity = _capacity;
        weight = _weight;
        nitems = _nitems;

        solve_knapsack_dp();
        // cout << endl;
        // for (int i = 0; i < optimal_pattern.size(); i++){
        //     cout << optimal_pattern[i];
        // }
        // cout << endl;
    }

    int max(int a, int b)
    {
        return (a > b) ? a : b;
    }


    // solve knapsack problem using dp
    void knapsack_solver::solve_knapsack_dp(){
        // dual value is regarded as (value of item)
        
        vector<vector<int>> table(nitems+1, vector<int>(capacity+1, 0));
        for (int i = 1; i < nitems+1; i++){
            for (int w = 1; w < capacity+1; w++){
                if (w < weight[i - 1]){
                    table[i][w] = table[i - 1] [w];
                }
                else{
                    table[i][w] = max(table[i-1][w-weight[i-1]] + dual_values[i-1], table[i-1][w]);
                }
            }
        }
        best_obj = table[nitems][capacity];
        int res = table[nitems][capacity];
        int w = capacity;
        vector<int> optimal_pattern_binary;
        optimal_pattern_binary = vector<int>(nitems, 0);
        for (int i = nitems; i >> 0 && res > 0; i--){
            if (res == table[i-1][w]){
                continue;
            }
            else{
                optimal_pattern_binary[i-1] = 1;
            }
            res = res - dual_values[i - 1];
            w = w - weight[i - 1];
        }
        
        for (int i = 0; i < optimal_pattern_binary.size(); ++i){
            if (optimal_pattern_binary[i] != 0){
                optimal_pattern.push_back(i);
            }
        }

        // cout << "Best objective in dp is "<<best_obj;
        exact_rc = 1 - best_obj;
        }

        


        



    void knapsack_solver::solve_knapsack_greedy(){
        // solve knapsack prolem using greedy
        



    }



    void run(){
        
    }

    


}