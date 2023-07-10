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


        
    }

    double cal_max(double a, double b)
    {
        return (a > b) ? a : b;
    }


    // solve knapsack problem using dp
    void knapsack_solver::solve_knapsack_dp(){
        // dual value is regarded as (value of item)

        // for (int i=0; i < dual_values.size(); ++i) {
        //     cout << dual_values[i] << ",";
        // }
        // cout << endl;

        // for (int i=0; i < weight.size(); ++i) {
        //     cout << weight[i] << " ";
        // }


        vector<vector<double>> table(nitems+1, vector<double>(capacity+1, 0));
        for (int i = 1; i < nitems+1; i++){
            for (int w = 1; w < capacity+1; w++){
                if (w < weight[i - 1]){
                    table[i][w] = table[i - 1] [w];
                }
                else{
                    table[i][w] = cal_max(table[i-1][w-weight[i-1]] + dual_values[i-1], table[i-1][w]);
                }

            }
        }
        best_obj = table[nitems][capacity];

        double res = table[nitems][capacity];
        int w = capacity;
        vector<int> optimal_pattern_binary;
        optimal_pattern_binary = vector<int>(nitems, 0);
        
        vector<int> selected_item;
        int total_weight = 0;
        int c = capacity;
        for (int i = nitems; i > 0; --i){
            if (table[i][c] > table[i - 1][c]){
                optimal_pattern_binary[i-1] = 1;
                total_weight += weight[i-1];
                c = c - weight[i - 1];
            }




            // if (res == table[i-1][w]){
            //     continue;
            // }
            // else if (res <=0){
            //     break;
            // }
            // else if (res != table[i-1][w]){
            //     total_weight += weight[i-1];
            //     optimal_pattern_binary[i-1] = 1;
            //     res = res - dual_values[i - 1];
            //     w = w - weight[i - 1];
            // }

        }

        cout << "new column weight is " << total_weight;
        // cout << "selected_item  is " << endl;
        // for (int i = 0; i < selected_item.size(); ++i){
        //     cout << selected_item[i] << " ";
        // }
        // cout << endl;


        for (int i = 0; i < optimal_pattern_binary.size(); ++i){
            if (optimal_pattern_binary[i] != 0){
                optimal_pattern.push_back(i);
            }

        }

        cout << "optimal pattern is " << endl;
        for (int i = 0; i < optimal_pattern_binary.size(); ++i){
            cout << optimal_pattern_binary[i] << ",";
        }
        cout << endl;

        
        cout << "best obj is " << best_obj << endl;
        exact_rc = 1 - best_obj;
        cout << "exact rc is " << exact_rc << endl;


        if (exact_rc > -0.0000000000001){
            isOptimal = true;
        }
        else {
            neg_rc_cols.push_back(optimal_pattern);
            neg_rc_vals.push_back(exact_rc);
        }
        }

        

        
        



    // void knapsack_solver::solve_knapsack_greedy(){
    //     // solve knapsack prolem using greedy
        



    // }



    void knapsack_solver::run(){


        solve_knapsack_dp();
        // postprocessing();
        compute_statistics();
    }

    


    // void knapsack_solver::postprocessing(){

    //     // cout << "size is " << optimal_pattern.size() << endl;

    // }


}