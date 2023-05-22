#include <knapsack_solver.h>



namespace Bin{


    knapsack_solver::knapsack_solver(int _method, double _cutoff, 
            const vector<double>& _dual_values, int _capacity, vector<double> _weight, int nitems, long long _upper_col_limit) 
    {   
        dual_values = _dual_values;
        method = _method;
        cutoff = cutoff;
        capacity=_capacity;
        weight=_weight;
        nitems=nitems;
    }

    int max(int a, int b)
    {
        return (a > b) ? a : b;
    }


    // solve knapsack problem using dp
    void knapsack_solver::solve_knapsack_dp(){
        // dual value is regarded as (value of item)
        int i,w_idx,n;
        vector<double> value = dual_values;
        int K[nitems+1][capacity+1];
        for (i = 0; i <= n; i++){
        
            for (w_idx = 0; w_idx <= weight.size(); w_idx++){
                int w = weight[w_idx];
                if (i == 0 || w == 0)
                    K[i][w_idx] = 0;
                else if (weight[i - 1] <= w)
                    K[i][w_idx]
                            = max(dual_values[i - 1] + K[i - 1][w - w[i - 1]], K[i - 1][w]);
                else
                    K[i][w_idx] = K[i - 1][w_idx];
        }
    }



        



    }

    void knapsack_solver::solve_knapsack_greedy(){
        // solve knapsack prolem using greedy
        



    }



    


}