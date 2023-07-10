#include "cg.h"

#include "MLBIN.h"
#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c++.h"
#include "knapsack_solver.h"
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <sys/time.h>

namespace Bin{

    static double get_wall_time(){
        struct timeval time;
        if (gettimeofday(&time,NULL)){
            return 0;
        }
        return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }

    CG::CG(const Instance& bin, int _seed, double _cutoff) : bin{bin}, seed{_seed}, cutoff{_cutoff}{
        cout << "CG object is initialized..." << endl;
        initializing_parameters();

    }




    void CG::initializing_parameters(){


        num_pattern = pattern_factor * bin.nitems;
        cout << "Pattern is generating...  "  << endl;
        capacity = bin.capacity;
        weight = bin.weight;
        pattern_set.resize(num_pattern);
        dual_values.resize(bin.nitems);

    }
    
    void CG::initializing_pattern(){
        mt19937 mt(seed);
        uniform_int_distribution<int> dist(0,RAND_MAX);
        vector<int> candidates(bin.nitems);
        int aggregated_weight;
        int nb_candidates, idx, item, num;

        for (int i = 0; i < num_pattern; ++i){

            nb_candidates = bin.nitems;
            aggregated_weight = 0;
            for (int j = 0; j < nb_candidates; ++j){
                candidates[j] = j;
            }

            while (nb_candidates > 0){
                if (nb_candidates == bin.nitems){
                    idx = i % bin.nitems;
                } else{
                    idx = dist(mt) % nb_candidates;
                }
                item = candidates[idx]; // item number_id
                pattern_set[i].push_back(item);
                aggregated_weight += weight[item];
                int remaining_capacity = capacity - aggregated_weight;
                num = 0;
                for (int j = 0; j < nb_candidates; ++ j){
                    if (remaining_capacity > weight[candidates[j]] && j != idx){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }
        }
        cout << "----" << endl;
        for (int i = 0; i < pattern_set.size(); ++i){
            for (int j = 0; j < pattern_set[i].size(); ++j){
                cout << pattern_set[i][j] << " ";
            }
            cout << endl;
        }

        // remove duplicated pattern after generalizing random pattern
        // loop the pattern set, sort them first
        // for (int i = 0; i < pattern_set.size(); ++i){
        //     std::stable_sort(pattern_set[i].begin(), pattern_set[i].end());
        // }
        // std::stable_sort(pattern_set.begin(), pattern_set.end());
        // // cout << pattern_set.size();
        // pattern_set.erase(std::unique(pattern_set.begin(), pattern_set.end()), pattern_set.end());

        // for (int i=0; i < pattern_set.size(); ++i ){
        //     cout << "randomly initializing pattern: " << endl;
        //     int weights = 0;
        //     for (int n = 0; n < pattern_set[i].size(); ++n){
        //         cout << pattern_set[i][n] << " " << "weight: " << bin.weight[pattern_set[i][n]] << " " << endl;
        //         weights += bin.weight[pattern_set[i][n]];
        //     }
        //     cout << "Total weight is " << weights << endl;
        // } 
        // cout << "Number of randomly generated pattern is " << pattern_set.size() << endl;

    }

    // initialize one pattern
    // void CG::initializing_pattern(){
    //     mt19937 mt(1e5);
    //     uniform_int_distribution<int> dist(0,RAND_MAX);
    //     int idx, item_i, num;

    //     // candidate_pattern is a column
    //     // num_candidates is the available candidate item to be added into the column

    //     vector<int> candidate_pattern(bin.nitems);
    //     int num_candidates;

    //     for (int i = 0; i < num_pattern; ++i){
    //         candidate_pattern.resize(bin.nitems);
    //         num_candidates = bin.nitems;

    //         int aggregated_weight = 0;
    //         // cout << "pattern id " << i <<endl;
    //         for (int j = 0; j < num_candidates; ++j){
    //             candidate_pattern[j] = j;
    //         }
    //         // cout << candidate_pattern.size()<<endl;
    //         while (num_candidates > 0){
    //             if (num_candidates == bin.nitems){
    //                 idx = i;
    //                 item_i = candidate_pattern[idx];
    //                 cout << "First picked item is " << idx << endl;
    //             }
    //             else {
    //                 idx = dist(mt) % candidate_pattern.size();
    //                 item_i = candidate_pattern[idx];
    //             }
    //             pattern_set[i].push_back(item_i);
    //             aggregated_weight += bin.weight[item_i];
    //             // cout << "Added item " << item_i << " Weight is " << bin.weight[item_i] << endl;

    //             candidate_pattern.erase(candidate_pattern.begin() + idx);
    //             // cout << idx << " is removed" << " the candidate size is " << candidate_pattern.size() << endl;
    //             int remaining_capacity = bin.capacity - aggregated_weight;
    //             // cout << "Aggregated item weight is " << aggregated_weight<< endl;
    //             // cout << "Remaining capacity is " << remaining_capacity << endl;
                
    //             // cout << "current candidate pattern size " <<candidate_pattern.size() << endl;
    //             // for (int n=0;n<candidate_pattern.size();++n){
    //             //     cout << candidate_pattern[n] << ": " << bin.weight[candidate_pattern[n]] <<endl;

    //             // }
    //             vector<int> invalid_item;
    //             for (int j = 0; j < candidate_pattern.size(); ++j){
    //                 // cout << "item " << candidate_pattern[j] << ":" << bin.weight[candidate_pattern[j]] << "remaining capcity" << remaining_capacity << endl;
    //                 if (bin.weight[candidate_pattern[j]] > remaining_capacity ){
    //                     // cout << "removing invalid item " << candidate_pattern[j] << endl;
    //                     invalid_item.push_back(candidate_pattern[j]);
    //                 }
    //             }
    //             // remove idx in invalid item from candidate pattern
    //             for (int m = 0; m < invalid_item.size(); ++m){
    //                 for (int n = 0; n < candidate_pattern.size(); ++n){
    //                     if (invalid_item[m] == candidate_pattern[n]){
    //                         candidate_pattern.erase(candidate_pattern.begin() + n);
    //                     } 
    //                 }
    //             }

    //             num_candidates = candidate_pattern.size();
    //             // cout << "Current candidate:..."<<endl;

    //         }


    //     }




 
    void CG::solve_restricted_master_problem(){

        // input columns are index of item
        num_pattern = pattern_set.size();

        vector<vector<bool>> pattern_set_binary(num_pattern, vector<bool>(bin.nitems, 0));
        long item_i;

        

        for (int i = 0; i < num_pattern; ++i){
            for (int j = 0; j < pattern_set[i].size(); ++j){
                item_i = pattern_set[i][j];
                pattern_set_binary[i][item_i] = 1;
            }
        }

        for (int i = 0; i < pattern_set_binary.size(); ++i){
            cout <<  "[";
            for (int j = 0; j < pattern_set_binary[i].size(); ++j){
                cout << pattern_set_binary[i][j] << ",";
            }
            cout << "]" << endl;
        }


        cout << "Solving RMP ";
        cout << "The number of column is " << pattern_set_binary.size() << endl;
        try{
            GRBEnv *env;
            vector<GRBVar> x;
            env = new GRBEnv();
            GRBModel model = GRBModel(*env);
            // model.getEnv().set(GRB_IntParam_OutputFlag, 0);
            // model.set(GRB_IntParam_Threads, thread_limit);
            model.set(GRB_StringAttr_ModelName, "RMP_GCP");

            x.resize(num_pattern);

            for (long i = 0; i < num_pattern; ++i){
                x[i] = model.addVar(0, 1, 0, GRB_CONTINUOUS);
            }
            model.update();

            // set covering problem, each item is covered by at least one set
            vector<GRBConstr> orders;
            orders.resize(bin.nitems);
            for (long j = 0; j < bin.nitems; ++j){
                GRBLinExpr rtot = 0;
                for (long i = 0; i < num_pattern; ++i){
                    rtot += pattern_set_binary[i][j] * x[i];
                }
                orders[j] = model.addConstr(rtot >= 1, "");
            }

            // objective
            GRBLinExpr objtot = 0;
            for (long i = 0; i < num_pattern; ++i){
                objtot += x[i];
            }
            
            model.setObjective(objtot, GRB_MINIMIZE);
            model.update();

            // model.set(GRB_IntParam_Presolve, 0);
            model.optimize();
            lp_bound = model.get(GRB_DoubleAttr_ObjVal);
            cout << "LP bound is " << lp_bound << endl;

            for (long j = 0; j < bin.nitems; ++j){
                dual_values[j] = orders[j].get(GRB_DoubleAttr_Pi);
                cout << dual_values[j] <<" ";
            }

            int cols = model.get(GRB_IntAttr_NumVars);

            long k = 0;
            lp_vbasis.resize(num_pattern);
            for (long i = 0; i < num_pattern; ++i){
                lp_vbasis[i] = x[i].get(GRB_IntAttr_VBasis);
                k+=(lp_vbasis[i]==0);
            }
            cout << endl;
            cout << "lp solution # basic variables: " << k << "/" << lp_vbasis.size() << "\n";
            
            delete env;
        }     
        catch (GRBException e) {
        cout << "Error code " << endl;
        cout <<  e.getErrorCode() << e.getMessage() << endl;
        }




    }


    bool CG::solve_knapsack_dp(double cutoff, double& min_reduced_cost){
        // if (cutoff <= 0)
        //     return false;
        // cout << "dp working with dual values" << endl;
        // cout << dual_values.size() << endl;
        Bin::knapsack_solver dp(cutoff, dual_values, capacity, weight, bin.nitems, 1e8);
        dp.run();
        optimal_pattern=dp.optimal_pattern;
        min_reduced_cost =dp.exact_rc;

        return dp.isOptimal;
        
    }



    void CG::collect_training_data(vector<vector<double>>& obj_coef, vector<vector<bool>>& solution){
        initializing_pattern();
        cout << "Pattern is initialized!" << endl;

        min_reduced_cost = -1.0;
        
        while (min_reduced_cost < 0){
            cout << "Num of iter " << cg_iters << endl; 
            cout << pattern_set.size() << endl;
            cout << "Solving restricted master problem" << endl;
            solve_restricted_master_problem();
            cout << "Solving pricing problem" << endl;
            solve_knapsack_dp(1e8, min_reduced_cost);
            if (cg_iters % 5 == 0){
                vector<bool> opt_sol(bin.nitems, false);

                for (auto item : optimal_pattern){
                    opt_sol[item]=true;
                }
                obj_coef.push_back(dual_values);
                solution.push_back(opt_sol);
            }
            // add new columns
            pattern_set.push_back(optimal_pattern);
            // cout << pattern_set.size() << endl;
            cout << "minimum reduced cost is " << min_reduced_cost << endl;
 
            if (cg_iters ++>=25) break;
        }

    }


    void CG::test(int method, std::ofstream* output_file_sampling_stats, std::ofstream* output_file_cg_stats){
        
        double start_time = get_wall_time();
        int upper_col_limit = bin.nitems;
        initializing_pattern();
        min_reduced_cost_exact = -1.0;
        cg_iters = 0;

        double t0;
        double duration;
        bool knapsack_optimal;

        while (true) {
            cout << "iteration " << cg_iters++ << " : " <<endl;
            t0 = get_wall_time();
            solve_restricted_master_problem();
            duration = get_wall_time()-t0;
            time_duration_master += duration;
            cout << "time used: " << duration << "\n";


            cout << "---------" << endl;
            cout << "current cg iter" << cg_iters << endl;
            if (output_file_cg_stats!=nullptr){
                cout << "testing ..." <<endl;
                (*output_file_cg_stats) << cg_iters << ","
                    << get_wall_time() - start_time << ","
                    << lp_bound << ",";
            }
 
            
            cout << "solve pricing knapsack by heuristic \n";
            t0 = get_wall_time();
            Pricer* pricer = nullptr;
    
            
            if (method == 1){
                pricer = new knapsack_solver(cutoff, dual_values, capacity,weight,bin.nitems, 0);
            }else if (method == 2){
                pricer = new MLBIN(2, 300, bin.nitems, bin.nitems, bin.capacity, bin.weight, dual_values, 0);
            }


            pricer->run();

            if (output_file_sampling_stats!=nullptr){
                for (auto i = 0; i < pricer->niterations; ++i){
                    (*output_file_sampling_stats) << cg_iters << "," << i+1 << ","
                                        << pricer->num_neg_rc_current_iteration[i] << ","
                                        << pricer->best_rc_current_iteration[i] << "\n";
                }
            }

            duration = get_wall_time() - t0;
            cout << "time used: " <<duration << "\n";
            time_duration_pricing_heur += duration;
            cout << "minimum reduced cost by pricer is: " << pricer->best_rc << endl;

            if (pricer->num_neg_rc_col > 0 ){
                num_heur_runs_success++;
                cout << "# columns with negative reduced cost found by heuristic pricer: " << pricer->num_neg_rc_col << "\n";
                pricer->include_new_cols_all(pattern_set);
                num_pattern = pattern_set.size();
            }
            else{
                cout << "heuristic pricer does not find any column with negative reduced cost!\n";
                t0 = get_wall_time();

                cout << "solve pricing problem by knapsack solver: \n";
                cout << "time budget used so far: " << (t0 - start_time) << "\n";
                auto knapsack_cutoff = cutoff - (t0 - start_time);
                knapsack_optimal = solve_knapsack_dp(knapsack_cutoff, min_reduced_cost_exact);

                if (knapsack_optimal){
                    cout << "minimum reduced cost by exact solver is: " << min_reduced_cost_exact << endl;
                }
                duration = get_wall_time() - t0;
                cout << "time used: " << duration << "\n";
                time_duration_pricing_heur += duration;
            }

            if (output_file_cg_stats!=nullptr){
                (*output_file_cg_stats) << pricer->num_neg_rc_col << ","
                                        << pricer->best_rc << ","
                                        << pricer->mean_rc << ","
                                        << pricer->median_rc << ","
                                        << pricer->stdev_rc << ","
                                        << pricer->column_selection << endl;
            }
            // cout << "checking..." << pricer->neg_rc_vals.size() << endl;
            if (pricer!=nullptr) {
                pricer = nullptr;}
            if (min_reduced_cost_exact>-0.0000000001) break;
            if (cg_iters == 2) break;
        }

        lp_optimal = knapsack_optimal && min_reduced_cost_exact>-0.0000000001;
    }
    
}
    // bool CG::solve_knapsack_gurobi(){
        
    //     GRBEnv *env;
    //     vector<GRBVar> x;
    //     env = new GRBEnv();
    //     GRBModel model = GRBModel(*env);
    //     model.getEnv().set(GRB_IntParam_OutputFlag, 0);
    //     // model.set(GRB_IntParam_Threads, thread_limit);
    //     model.set(GRB_StringAttr_ModelName, "RMP_GCP");

    //     //subproblem solver

    // }



    // void CG::collect_training_data(){

    //     min_reduced_cost = -1.0;

    //     while (min_reduced_cost < -0.0000001){

    //         cout << 'Solve restricted master problem' << endl;
    //         solve_restricted_master_problem();
    //         cout << "solve the Pricing problem" << endl;
    //         solve_knapsack_gurobi();
    //         // training data recording
    //         if (cg_iters % 5==0){
    //             vector<int> opt_sol(bin.nitems, false);
    //             for (int i = 0; i < optimal_pattern.size(); i++){
    //                 opt_sol[i] = 
    //             }    
    //             dual_values.push_back(dual_values);
    //             soluion.push_back(opt_sol);
    //         }

    //     }

