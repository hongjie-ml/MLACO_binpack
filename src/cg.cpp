#include "cg.h"

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c++.h"


namespace Bin{


    CG::CG(const Instance& bin) : bin{bin} {
        cout << "CG object is initialized..." << endl;
        initializing_parameters();
    }




    void CG::initializing_parameters(){


        num_pattern = pattern_factor * bin.nitems;
        cout << "number of pattern generated is " << num_pattern << endl;
        capacity = bin.capacity;
        weight = bin.weight;
        pattern_set.resize(num_pattern);
        dual_values.resize(bin.nitems);
        

    }


    void CG::initializing_pattern(){
        mt19937 mt(1e5);
        uniform_int_distribution<int> dist(0,RAND_MAX);
        int idx, item_i, num;

        // candidate_pattern is a column
        // num_candidates is the available candidate item to be added into the column

        vector<int> candidate_pattern(bin.nitems);
        int num_candidates;

        // loop each initialized pattern
        for (int i = 0; i < bin.nitems; ++i){
            num_candidates = bin.nitems;
            int aggregated_weight = 0;
            vector<int> possible_bin_id;
            for (int j = 0; j < num_candidates; ++j){
                    candidate_pattern[j] = j;
                }
            while (num_candidates > 0){
                
                if (num_candidates == bin.nitems){
                    idx = i % bin.nitems;
                    item_i = candidate_pattern[idx];
                }
                else {
                    idx = dist(mt) % num_candidates;
                    item_i = possible_bin_id[idx];
                }
                possible_bin_id.clear();
                aggregated_weight += bin.weight[item_i];
                pattern_set[i].push_back(item_i);
                num = 0;

                // cout << "Added item " << item_i << " Weight is " << bin.weight[item_i] << endl;
                int remaining_capacity = bin.capacity - aggregated_weight;
                // cout << "Remaining capacity is "<< remaining_capacity << endl;

                for (int j = 0; j < num_candidates; ++j){

                    if (bin.weight[j] < remaining_capacity && j != idx){
                        // cout << "Possible bin ID " << j << " weight is " << bin.weight[j] << endl;
                        possible_bin_id.push_back(j);
                        num++;
                    } 
                }
                num_candidates = num;

            }
            // cout << "randomly initializing pattern: " << endl;
            // int weights = 0;
            // for (int n = 0; n < pattern_set[i].size(); ++n){
            //     cout << pattern_set[i][n] << " " << "weight: " << bin.weight[pattern_set[i][n]] << " " << endl;
            //     weights += bin.weight[pattern_set[i][n]];
            // }
        }    
    }

 
    void CG::solve_restricted_master_problem(){

        cout << "Computing restricted master problem" << endl;
        num_pattern = pattern_set.size();
        vector<vector<bool>> pattern_set_binary(num_pattern, vector<bool>(bin.nitems, 0));
        long item_i;
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

            // set covering problem
            vector<GRBConstr> orders;
            orders.resize(bin.nitems);
            for (long j = 0; j < bin.nitems; ++j){
                GRBLinExpr rtot = 0;
                for (long i = 0; i < num_pattern; ++i){
                    rtot += pattern_set_binary[i][j] * x[i];
                }
                orders[j] = model.addConstr(rtot >= 1, "");
            }

            GRBLinExpr objtot = 0;
            for (long i = 0; i < num_pattern; ++i){
                objtot += x[i];
            }
            
            model.setObjective(objtot, GRB_MINIMIZE);
            model.update();
            model.optimize();
            lp_bound = model.get(GRB_DoubleAttr_ObjVal);
            cout << "LP bound is " << lp_bound << endl;
            // save pi value
            dual_values.resize(bin.nitems);

         
            for (long j = 0; j < bin.nitems; ++j){
                dual_values[j] = orders[j].get(GRB_DoubleAttr_Pi);
                // cout << dual_values[j] << endl;
            }
            delete env;
        }      
        catch (GRBException e) {
        cout << "Error code " << endl;
        cout <<  e.getErrorCode() << e.getMessage() << endl;
        }
    }


    bool CG::solve_knapsack_gurobi(double cutoff, double& min_reduced_cost){
        if (cutoff <= 0)
            return false;
        knapsack_solver knapsack_solver(1, cutoff, dual_values, capacity, weight, 1e8);
        knapsack_solver.run();
        optimal_pattern=knapsack_solver.optimal_pattern;
        min_reduced_cost = knapsack_solver.exact_rc;

        return knapsack_solver.isOptimal;
        
    }



    void CG::collect_training_data(vector<vector<double>>& obj_coef, vector<vector<bool>>& solution){
        initializing_pattern();
        min_reduced_cost = -1.0;

        while (min_reduced_cost < -0.0000001){


            cout << "Solving restricted master problem" << endl;
            solve_restricted_master_problem();
            cout << "Solving pricing problem" << endl;
            solve_knapsack_gurobi(1e8, min_reduced_cost);
            if (cg_iters % 5 == 0){
                
                vector<bool> opt_sol(bin.nitems, false);
                for (auto v : optimal_pattern){
                    opt_sol[v]=true;
                }
                obj_coef.push_back(dual_values);
                solution.push_back(opt_sol);
            }


            // add new columns
            pattern_set.push_back(optimal_pattern);
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
    
