 #ifndef CG_H
#define CG_H

#include "instance.h"
#include "knapsack_solver.h"
#include <random>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;
using std::mt19937;
using std::uniform_int_distribution;



namespace Bin{



    class CG{


        const Instance& bin;
        const string training_data_dir = "../train_data/";
        const string test_data_dir = "../test_data/";
        const string training_model_name = "train_model";
        const string test_data_name = "_test_data_";
        const string output_file_name = "_predicted_value_";
        const int pattern_factor = 1;
        int capacity;
        vector<int> weight;
        vector<int> pattern;
        vector<int> lp_vbasis;
        vector<int> lp_cbasis;
        
        

        void initializing_parameters();


        
    public:

        vector<int> optimal_pattern;
        int num_pattern = 0;
        double lp_bound = 0.0;
        double min_reduced_cost;
        int cg_iters =0;

        vector<double> dual_values;
        vector<vector<int>> pattern_set;
        explicit CG(const Instance& bin);


        void initializing_pattern();
        void solve_restricted_master_problem();

        bool solve_knapsack_dp(double cutoff, double& min_reduced_cost);

        void collect_training_data(vector<vector<double>>& obj_coef, vector<vector<bool>>& solution);

    };
















}













#endif