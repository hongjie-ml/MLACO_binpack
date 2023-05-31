#include "cg.h"

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c++.h"
#include "knapsack_solver.h"
#include <algorithm>


namespace Bin{


    CG::CG(const Instance& bin) : bin{bin} {
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

    // initialize one pattern
    void CG::initializing_pattern(){
        mt19937 mt(1e5);
        uniform_int_distribution<int> dist(0,RAND_MAX);
        int idx, item_i, num;

        // candidate_pattern is a column
        // num_candidates is the available candidate item to be added into the column

        vector<int> candidate_pattern(bin.nitems);
        int num_candidates;

        for (int i = 0; i < num_pattern; ++i){
            candidate_pattern.resize(bin.nitems);
            num_candidates = bin.nitems;

            int aggregated_weight = 0;
            // cout << "pattern id " << i <<endl;
            for (int j = 0; j < num_candidates; ++j){
                candidate_pattern[j] = j;
            }
            // cout << candidate_pattern.size()<<endl;
            while (num_candidates > 0){
                if (num_candidates == bin.nitems){
                    idx = i;
                    item_i = candidate_pattern[idx];
                    cout << "First picked item is " << idx << endl;
                }
                else {
                    idx = dist(mt) % candidate_pattern.size();
                    item_i = candidate_pattern[idx];
                }
                pattern_set[i].push_back(item_i);
                aggregated_weight += bin.weight[item_i];
                // cout << "Added item " << item_i << " Weight is " << bin.weight[item_i] << endl;

                candidate_pattern.erase(candidate_pattern.begin() + idx);
                // cout << idx << " is removed" << " the candidate size is " << candidate_pattern.size() << endl;
                int remaining_capacity = bin.capacity - aggregated_weight;
                // cout << "Aggregated item weight is " << aggregated_weight<< endl;
                // cout << "Remaining capacity is " << remaining_capacity << endl;
                
                // cout << "current candidate pattern size " <<candidate_pattern.size() << endl;
                // for (int n=0;n<candidate_pattern.size();++n){
                //     cout << candidate_pattern[n] << ": " << bin.weight[candidate_pattern[n]] <<endl;

                // }
                vector<int> invalid_item;
                for (int j = 0; j < candidate_pattern.size(); ++j){
                    // cout << "item " << candidate_pattern[j] << ":" << bin.weight[candidate_pattern[j]] << "remaining capcity" << remaining_capacity << endl;
                    if (bin.weight[candidate_pattern[j]] > remaining_capacity ){
                        // cout << "removing invalid item " << candidate_pattern[j] << endl;
                        invalid_item.push_back(candidate_pattern[j]);
                    }
                }
                // remove idx in invalid item from candidate pattern
                for (int m = 0; m < invalid_item.size(); ++m){
                    for (int n = 0; n < candidate_pattern.size(); ++n){
                        if (invalid_item[m] == candidate_pattern[n]){
                            candidate_pattern.erase(candidate_pattern.begin() + n);
                        } 
                    }
                }

                num_candidates = candidate_pattern.size();
                // cout << "Current candidate:..."<<endl;

            }


        }

        // remove duplicated pattern after generalizing random pattern
        // loop the pattern set, sort them first
        for (int i = 0; i < pattern_set.size(); ++i){
            std::stable_sort(pattern_set[i].begin(), pattern_set[i].end());
        }
        std::stable_sort(pattern_set.begin(), pattern_set.end());
        // cout << pattern_set.size();
        pattern_set.erase(std::unique(pattern_set.begin(), pattern_set.end()), pattern_set.end());
        // cout << pattern_set.size();
        // find duplicated pattern (same vector)
        for (int i=0; i < pattern_set.size(); ++i ){
            cout << "randomly initializing pattern: " << endl;
            int weights = 0;
            for (int n = 0; n < pattern_set[i].size(); ++n){
                cout << pattern_set[i][n] << " " << "weight: " << bin.weight[pattern_set[i][n]] << " " << endl;
                weights += bin.weight[pattern_set[i][n]];
            }
            cout << "Total weight is " << weights << endl;
        } 
        cout << "Number of randomly generated pattern is " << pattern_set.size() << endl;

    }
        // loop each initialized pattern
        // for (int i = 0; i < bin.nitems; ++i){
        //     num_candidates = bin.nitems;
        //     int aggregated_weight = 0;
        //     // vector<int> possible_bin_id;
        //     for (int j = 0; j < num_candidates; ++j){
        //             candidate_pattern[j] = j;
        //         }
        //     while (num_candidates > 0){
                
        //         if (num_candidates == bin.nitems){
        //             idx = i % bin.nitems;
        //             // item_i = candidate_pattern[idx];
        //         }
        //         else {
        //             idx = dist(mt) % candidate_pattern.size();
        //             // item_i = candidate_pattern[idx];
        //         }
                
        //         aggregated_weight += bin.weight[idx];
        //         pattern_set[i].push_back(idx);
        //         num = 0;
        //         // candidate_pattern.clear();
        //         // cout << "Added item " << item_i << " Weight is " << bin.weight[item_i] << endl;
        //         int remaining_capacity = bin.capacity - aggregated_weight;
        //         cout << "Remaining capacity is "<< remaining_capacity << endl;

        //         // cout << "Lets see what items are available in next round" << endl;
        //         for (int j = 0; j < num_candidates; ++j){
        //             if (bin.weight[j] < remaining_capacity && j != idx){
        //                 // cout << "Possible bin ID " << j << " weight is " << bin.weight[j] << endl;
        //                 candidate_pattern[num] = candidate_pattern[j];
        //                 num++;
        //             } 
        //         }
        //         num_candidates = num;

        //     }
        //     cout << "randomly initializing pattern: " << endl;
        //     int weights = 0;
        //     for (int n = 0; n < pattern_set[i].size(); ++n){
        //         cout << pattern_set[i][n] << " " << "weight: " << bin.weight[pattern_set[i][n]] << " " << endl;
        //         weights += bin.weight[pattern_set[i][n]];
        //     } 
        // }    


 
    void CG::solve_restricted_master_problem(){

        
        num_pattern = pattern_set.size();

        vector<vector<bool>> pattern_set_binary(num_pattern, vector<bool>(bin.nitems, 0));
        long item_i;

        cout << "Solving RMP ";
        cout << "  The number of column is " << pattern_set.size() << endl;

        for (int i = 0; i < num_pattern; ++i){
            for (int j = 0; j < pattern_set[i].size(); ++j){
                item_i = pattern_set[i][j];
                pattern_set_binary[i][item_i] = 1;
            }
        }


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

            model.set(GRB_IntParam_Presolve, 0);
            model.optimize();
            lp_bound = model.get(GRB_DoubleAttr_ObjVal);
            cout << "LP bound is " << lp_bound << endl;
            // save pi value
            cout << "Dual value: ";
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
        if (cutoff <= 0)
            return false;
        cout << "dp working" << endl;
        Bin::knapsack_solver dp(cutoff, dual_values, capacity, weight, bin.nitems, 1e8);
        optimal_pattern=dp.optimal_pattern;
        min_reduced_cost = dp.exact_rc;

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


    // }
    
