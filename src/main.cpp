#include "instance.h"
#include "cg.h"


using Bin::Instance;

int main(void) {


    const string input_dir = "../data/";

    const auto instance = Instance("u250_00", input_dir, false);
    cout << instance.num_pricing << endl;
    
    // auto bin = Bin::CG(instance);
    // bin.initializing_pattern();
    // double min_reduced_cost = -1.0;
    // cout << bin.pattern_set.size() << endl;
    // while (min_reduced_cost < -0.0000001){

        // cout << "Solving restricted master problem" << endl;
        // bin.solve_restricted_master_problem();
        // cout << "Solving pricing problem" << endl;
        // bin.solve_knapsack_dp(1e8, min_reduced_cost);
        // cout << "---" << endl;
        // for (int i = 0; i < bin.optimal_pattern.size(); ++i){
        //     cout << bin.optimal_pattern[i];
        // }
        // cout << endl;
        // cout << bin.optimal_pattern.size() << endl;
        // bin.pattern_set.push_back(bin.optimal_pattern);
        // cout << "Added optiaml pattern found in pricing problem, new size is " <<bin.pattern_set.size() << endl;

    //     min_reduced_cost = 1;
    // }
    
    

    
    // cout << 
    // bin.solve_knapsack_dp(1e8, min_reduced_cost);
    // bin.pattern_set.push_back(bin.optimal_pattern);
    // cout << "--------" << endl;
    // cout << bin.pattern_set.size() << endl;
    // cout << bin.min_reduced_cost <<endl;
    // bin.solve_restricted_master_problem();
    // bin.solve_knapsack_dp(1e8, bin.min_reduced_cost);
    // bin.pattern_set.push_back(bin.optimal_pattern);
    // cout << "--------" << endl;
    // cout << bin.pattern_set.size() << endl;
    // cout << bin.min_reduced_cost <<endl;
    // bin.solve_restricted_master_problem();
    // vector<vector<double>> obj_coef;
    // vector<vector<bool>> solution;
    // bin.collect_training_data(obj_coef, solution);





    // cout << obj_coef.size() << endl;
    // for (int i = 0; i < obj_coef.size(); i++){
    //     for (int j = 0; j < obj_coef[i].size(); j++){
    //         cout << obj_coef[i][j] << " ";
    //     }
    //     cout << endl;
    //     cout << "---" << endl;
    // }

}   