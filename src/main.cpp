#include "instance.h"
#include "cg.h"


using Bin::Instance;

int main(void) {


    const string input_dir = "../data/";

    const auto instance = Instance("u40_00", input_dir, false);

    
    auto bin = Bin::CG(instance);
    
    cout << bin.dual_values.size() << endl;
    

    bin.initializing_pattern();

    bin.solve_restricted_master_problem();
    double min_reduced_cost = -1.0;
    cout << 
    bin.solve_knapsack_dp(1e8, min_reduced_cost);
    bin.pattern_set.push_back(bin.optimal_pattern);
    cout << "--------" << endl;
    cout << bin.pattern_set.size() << endl;
    cout << bin.min_reduced_cost <<endl;
    bin.solve_restricted_master_problem();
    bin.solve_knapsack_dp(1e8, bin.min_reduced_cost);
    bin.pattern_set.push_back(bin.optimal_pattern);
    cout << "--------" << endl;
    cout << bin.pattern_set.size() << endl;
    cout << bin.min_reduced_cost <<endl;
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