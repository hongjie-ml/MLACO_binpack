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
#include "ACO.h"




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

        adj_list = bin.adj_list;
        adj_matrix = vector<vector<bool>>(bin.nitems, vector<bool>(bin.nitems, 0));
        for (int i = 0; i < bin.nitems; ++i){
            for (int j = 0; j < adj_list[i].size(); ++j){
                adj_matrix[i][adj_list[i][j]] = 1;
                adj_matrix[adj_list[i][j]][i] = 1;
            }
         }     
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
                    if (remaining_capacity > weight[candidates[j]] && j != idx && adj_matrix[item][candidates[j]] == 1){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }
        }
        // cout << "----" << endl;
        // for (int i = 0; i < pattern_set.size(); ++i){
        //     for (int j = 0; j < pattern_set[i].size(); ++j){
        //         cout << pattern_set[i][j] << " ";
        //     }
        //     cout << endl;
        // }

    }

  
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

        cout << "Solving RMP ";
        cout << "The number of column binary is " << pattern_set_binary.size() << endl;


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
            }

            cout << "dual value in LP: " << dual_values[0] << endl;

            // int cols = model.get(GRB_IntAttr_NumVars);

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

    // no-adjacent vertices cannot be selected simultaneously
    bool CG::solve_knapsack_conflicted_gurobi(double cutoff, double& min_reduced_cost){

        // if (cutoff <=0 )
        //     return false;

        cout << "Solving Pricing problem ... " << endl;
        try{
        GRBEnv *env;
        vector<GRBVar> x;
        env = new GRBEnv();
        GRBModel model = GRBModel(*env);
        model.set(GRB_DoubleParam_TimeLimit, cutoff);
        model.getEnv().set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_StringAttr_ModelName, "MIN_KNAPSACK_CONFLICTED");

        x.resize(bin.nitems);
        for (int i = 0; i < bin.nitems; ++i){
            x[i] = model.addVar(0, 1, 0, GRB_BINARY);
        }
        model.update();

        // no-adjacent vertices cannot be selected simultaneously
        for (int i = 0; i < bin.nitems; ++i){
            for (int j = i+1; j < bin.nitems; ++j){
                if (adj_matrix[i][j] == 0){
                    model.addConstr(x[i] + x[j] <= 1, " ");
                }
            }
        };

        // weight constraints
        GRBLinExpr totalweight = 0;
        for (long i = 0; i < x.size(); ++i){
                totalweight += weight[i] * x[i];
            }
        model.addConstr(totalweight <= bin.capacity);


        model.update();
        model.set(GRB_IntParam_Presolve, 0);

        // the objective
        GRBLinExpr tot = 0;
        for(int i = 0; i < bin.nitems; ++i){
            tot += x[i] * dual_values[i];
        }
        
        model.setObjective(tot, GRB_MAXIMIZE);
        model.optimize();
        cout << "Pricing problem obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
        min_reduced_cost = 1 - model.get(GRB_DoubleAttr_ObjVal);
        int status = model.get(GRB_IntAttr_Status);

        kanpsack_conflict_optimal = (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL);

        optimal_pattern.clear();        
        if (kanpsack_conflict_optimal){
            for (int i=0; i < bin.nitems; i++){
                if (abs(x[i].get(GRB_DoubleAttr_X)) > 0.5){
                    optimal_pattern.push_back(i);
                }
            }
        }
        delete env;
        
        }
        catch (GRBException e) {
        cout << "Error code " << endl;
        cout <<  e.getErrorCode() << e.getMessage() << endl;
        }

        cout << "kanpsack_conflict_optimal " << kanpsack_conflict_optimal << endl;
        return kanpsack_conflict_optimal;
    }


    bool CG::solve_knapsack_dp(double cutoff, double& min_reduced_cost){


        // Bin::knapsack_solver dp(cutoff, dual_values, capacity, weight, bin.nitems, 1e8);
        // dp.run();
        // optimal_pattern=dp.optimal_pattern;
        // min_reduced_cost =dp.exact_rc;

        // return dp.isOptimal;
        return false;
    }


    void CG::collect_training_data(vector<vector<double>>& obj_coef, vector<vector<bool>>& solution){
        initializing_pattern();
        cout << "Pattern is initialized!" << endl;

        min_reduced_cost = -1.0;
        
        while (min_reduced_cost < -0.000001){
            cout << "Num of iter " << cg_iters << endl; 
            cout << pattern_set.size() << endl;
            cout << "Solving restricted master problem" << endl;
            solve_restricted_master_problem();
            cout << "Solving pricing problem" << endl;
            solve_knapsack_conflicted_gurobi(1e8, min_reduced_cost);
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

            if (cg_iters++>=25) break;
        }

    }


    bool CG::solve_knapsack_conflicted_gurobi_set(double cutoff, double& min_reduced_cost){

        cout << "Solving Pricing problem ... " << endl;
        try{
        GRBEnv *env;
        vector<GRBVar> x;
        env = new GRBEnv();
        GRBModel model = GRBModel(*env);
        model.set(GRB_DoubleParam_TimeLimit, cutoff);
        model.getEnv().set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_StringAttr_ModelName, "MIN_KNAPSACK_CONFLICTED");

        x.resize(bin.nitems);

        for (int i = 0; i < bin.nitems; ++i){
            x[i] = model.addVar(0, 1, 0, GRB_BINARY);
        }
        model.update();

        // no-adjacent vertices cannot be selected simultaneously
        for (int i = 0; i < bin.nitems; ++i){
            for (int j = i+1; j < bin.nitems; ++j){
                if (adj_matrix[i][j] == 0){
                    model.addConstr(x[i] + x[j] <= 1, " ");
                }
            }
        };

        // weight constraints
        GRBLinExpr totalweight = 0;
        for (long i = 0; i < x.size(); ++i){
                totalweight += weight[i] * x[i];
            }
        model.addConstr(totalweight <= bin.capacity);


        model.update();
        model.set(GRB_IntParam_Presolve, 0);

        // the objective
        GRBLinExpr tot = 0;
        for(int i = 0; i < bin.nitems; ++i){
            tot += x[i] * dual_values[i];
        }
        
        model.setObjective(tot, GRB_MAXIMIZE);
        model.optimize();
        
        cout << "Pricing problem obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
        isOptimal = (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL);
        min_reduced_cost = 1 - model.get(GRB_DoubleAttr_ObjVal);

        auto nsol = model.get(GRB_IntAttr_SolCount);
        cout << "nsol in exact solver: " << nsol << endl;
        optimal_pattern_set.clear();

        for (int i = 0; i < nsol; i++){
            vector<int> sol;
            model.set(GRB_IntParam_SolutionNumber, i);
            auto rc = 1 - model.get(GRB_DoubleAttr_PoolObjVal);

            if (rc < -0.000001){
                for (int j=0; j < bin.nitems; j++){
                    if (x[j].get(GRB_DoubleAttr_Xn) > 0.5){
                        sol.push_back(j);
                    }
                }
                optimal_pattern_set.push_back(sol);
            }
        }
        delete env;
        
        }
        catch (GRBException e) {
        cout << "Error code " << endl;
        cout <<  e.getErrorCode() << e.getMessage() << endl;
        }

        lg_bound = lp_bound + min_reduced_cost;
        cout << "knapsack_conflict_optimal " << isOptimal << endl;
        return isOptimal;


    }


    void CG::test(int method, std::ofstream* output_file_sampling_stats, std::ofstream* output_file_cg_stats){
        
        double start_time = get_wall_time();
        int upper_col_limit = bin.nitems;
        initializing_pattern();

        min_reduced_cost_exact = -1.0;
        cg_iters = 0;

        int cutoff = 600;
        double t0;
        double duration;
        bool knapsack_optimal;
        int num_ml_sampling_fail = 0;
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
                (*output_file_cg_stats) << cg_iters << ","
                    << get_wall_time() - start_time << ","
                    << lp_bound << ",";
            }
 
            
            cout << "solve knapsack by heuristic \n";

            t0 = get_wall_time();
            Pricer* pricer = nullptr;

            
            if (method == 1){
                // using gurobi solving directly
                pricer = new knapsack_solver(method, cutoff, dual_values, bin.capacity, bin.weight, bin.nitems, adj_matrix, adj_list, 0);

            }
            else if (method == 2){
                // if (cg_iters < 90){
                pricer = new MLBIN(method, cutoff, bin.nitems, bin.nitems, bin.capacity, bin.weight, dual_values, bin.degree_norm, adj_matrix, adj_list, 0);
                // }
                // else{
                // pricer = new knapsack_solver(method, cutoff, dual_values, bin.capacity, bin.weight, bin.nitems, adj_matrix, adj_list, 0);
                // }
            }
            else if (method == 3){
                pricer = new ACO(method, cutoff, bin.nitems, bin.nitems, bin.nitems, bin.weight, bin.capacity, dual_values, adj_matrix, adj_list, 0);
            }

            cout << "Pricer set up " << endl;
            pricer->run();

            if (output_file_sampling_stats!=nullptr){
                for (auto i = 0; i < pricer->niterations; ++i){
                    (*output_file_sampling_stats) << cg_iters << "," << i+1 << ","
                                        << pricer->num_neg_rc_current_iteration[i] << ","
                                        << pricer->best_rc_current_iteration[i] << "\n";
                }
            }
            
            duration = get_wall_time() - t0;
            cout << "time used: " << duration << "\n";
            time_duration_pricing_heur += duration;
            cout << "minimum reduced cost by pricer is: " << pricer->best_rc << endl;
            
            if (pricer->num_neg_rc_col > 0 ){
                num_heur_runs_success++;
                cout << "# columns with negative reduced cost found by pricer: " << pricer->num_neg_rc_col << "\n";
                
                pricer->include_new_cols_all(pattern_set);
                num_pattern = pattern_set.size();

            }
            else{
                num_ml_sampling_fail++;
                cout << "heuristic pricer does not find any column with negative reduced cost!\n";
                t0 = get_wall_time();
                cout << "solve pricing problem by Gurobi solver: \n";
                cout << "time budget used so far: " << (t0 - start_time) << "\n";
                auto knapsack_cutoff = cutoff - (t0 - start_time);


                //*** using exact Pattern set  ***

                knapsack_optimal = solve_knapsack_conflicted_gurobi_set(knapsack_cutoff, min_reduced_cost_exact);
                if (knapsack_optimal){
                    cout << "minimum reduced cost by exact solver is: " << min_reduced_cost_exact << endl;
                    cout << "Current column size: " << pattern_set.size() << endl;
                    cout << "Adding column..." << pattern_set.size() << endl;
                    pattern_set.insert(pattern_set.end(), optimal_pattern_set.begin(), optimal_pattern_set.end());
                    cout << "Current column size: " << pattern_set.size() << endl;
                    num_pattern = pattern_set.size();

                }



                // **** Original method ****
                // knapsack_optimal = solve_knapsack_conflicted_gurobi(knapsack_cutoff, min_reduced_cost_exact);

                
                // if (knapsack_optimal){
                    
                //     cout << "minimum reduced cost by exact solver is: " << min_reduced_cost_exact << endl;
                //     if (min_reduced_cost_exact < -0.000001){
                //         pattern_set.push_back(optimal_pattern);
                        
                //         cout << "Debugging ..." <<endl;
                //         for (int j=0; j < optimal_pattern.size(); j++){
                //             cout << optimal_pattern[j] << " ";
                //         }
                //         cout << endl;
                //         num_pattern++;
                //     }
                // }
                // **** Original method ****
                

                duration = get_wall_time() - t0;
                cout << "time used: " << duration << "\n";
                time_duration_pricing_heur += duration;
            }

            lg_bound = lp_bound + pricer->best_rc;

            if (output_file_cg_stats!=nullptr){
                (*output_file_cg_stats) << pricer->num_neg_rc_col << ","
                                        << pricer->best_rc << ","
                                        << pricer->mean_rc << ","
                                        << pricer->median_rc << ","
                                        << pricer->stdev_rc << ","
                                        << lg_bound << endl;
            }

        
            cout << "lp_bound: "<< lp_bound << "-----" << "lg_bound: " << lg_bound << endl;
            if (pricer != nullptr) {
                pricer = nullptr;}
            if (std::ceil(lp_bound) == std::ceil(lg_bound)) break;
            if (min_reduced_cost_exact>-0.000001) break;
            if (get_wall_time()-start_time > cutoff) break;
            cout << "min_reduced_cost_exact " << min_reduced_cost_exact << endl;

        }
        lp_optimal = knapsack_optimal && min_reduced_cost_exact>-0.000001;
        if (std::ceil(lp_bound) == std::ceil(lg_bound)){
            lp_optimal = true;
        } 
        cout << "lp_optimal: " << lp_optimal << endl;
    }
    
}
