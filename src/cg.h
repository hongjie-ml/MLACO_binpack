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
        double min_reduced_cost_exact;
        vector<vector<int>> adj_list;
        vector<vector<bool>> adj_matrix;
        bool kanpsack_conflict_optimal = false;
        bool isOptimal = false;
        void initializing_parameters();
        

        
    public:
        int seed;
        int k;
        vector<int> optimal_pattern;
        int num_pattern = 0;
        double lp_bound = 0.0;
        double lg_bound = 0.0;
        double min_reduced_cost;
        int cg_iters =0;
        double cutoff;
        double time_duration_master=0;
        bool lp_optimal=false;
        vector<double> dual_values;
        vector<vector<int>> pattern_set;
        double time_duration_pricing_exact=0;
        double time_duration_pricing_heur=0;
        int num_heur_runs_success = 0;

        
        explicit CG(const Instance& bin, int _seed, double cutoff);

        double pricer_cutoff;
        
        vector<vector<int>> optimal_pattern_set;
        // double optimize_LM(int method, double b0, double b1);
        
        
        void initializing_pattern();
        void solve_restricted_master_problem();
        bool solve_knapsack_conflicted_gurobi(double cutoff, double& min_reduced_cost);
        
        
        bool solve_knapsack_conflicted_gurobi_set(double cutoff, double& min_reduced_cost);


        void collect_training_data(vector<vector<double>>& obj_coef, vector<vector<bool>>& solution);
        void test(int method, int method_type, std::ofstream* output_file_sampling_stats, std::ofstream* output_file_cg_stats);


        bool solve_knapsack_dp(double cutoff, double& min_reduced_cost);
        

    };
















}













#endif