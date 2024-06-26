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
#include "mlaco.h"
#include "mlmmas.h"

namespace Bin
{

    static double get_wall_time()
    {
        struct timeval time;
        if (gettimeofday(&time, NULL))
        {
            return 0;
        }
        return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }

    CG::CG(const Instance &bin, int _seed, double _cutoff) : bin{bin}, seed{_seed}, cutoff{_cutoff}
    {
        cout << "CG object is initialized..." << endl;

        initializing_parameters();
    }

    void CG::initializing_parameters()
    {

        num_pattern = pattern_factor * bin.nitems;
        cout << "Pattern is generating...  " << endl;
        capacity = bin.capacity;
        weight = bin.weight;
        pattern_set.resize(num_pattern);
        dual_values.resize(bin.nitems);

        adj_list = bin.adj_list;
        adj_matrix = vector<vector<bool>>(bin.nitems, vector<bool>(bin.nitems, 0));
        for (int i = 0; i < bin.nitems; ++i)
        {
            for (int j = 0; j < adj_list[i].size(); ++j)
            {
                adj_matrix[i][adj_list[i][j]] = 1;
                adj_matrix[adj_list[i][j]][i] = 1;
            }
        }

        // cout << "check pattern include in RMP " << endl;
        // for (auto x=0; x < adj_matrix.size(); x++)
        // {
        //     for (auto y=0; y < adj_matrix[x].size(); y++)
        //     {
        //         cout << adj_matrix[x][y] << " ";
        //     }
        //     cout << endl;
        // }

    }

    void CG::initializing_pattern()
    {
        mt19937 mt(seed);
        uniform_int_distribution<int> dist(0, RAND_MAX);
        vector<int> candidates(bin.nitems);
        int aggregated_weight;
        int nb_candidates, idx, item, num;

        for (int i = 0; i < num_pattern; ++i)
        {

            nb_candidates = bin.nitems;
            aggregated_weight = 0;
            for (int j = 0; j < nb_candidates; ++j)
            {
                candidates[j] = j;
            }

            while (nb_candidates > 0)
            {
                if (nb_candidates == bin.nitems)
                {
                    idx = i % bin.nitems;
                }
                else
                {
                    idx = dist(mt) % nb_candidates;
                }
                item = candidates[idx]; // item number_id
                pattern_set[i].push_back(item);
                aggregated_weight += weight[item];
                int remaining_capacity = capacity - aggregated_weight;
                num = 0;
                for (int j = 0; j < nb_candidates; ++j)
                {
                    if (remaining_capacity > weight[candidates[j]] && j != idx && adj_matrix[item][candidates[j]] == 0)
                    {
                        candidates[num] = candidates[j];
                        num++;
                    }
                }
                nb_candidates = num;
            }
        }
    }

    void CG::solve_restricted_master_problem()
    {

        // input columns are index of item
        num_pattern = pattern_set.size();

        vector<vector<bool>> pattern_set_binary(num_pattern, vector<bool>(bin.nitems, 0));
        long item_i;

        for (int i = 0; i < num_pattern; ++i)
        {
            for (int j = 0; j < pattern_set[i].size(); ++j)
            {
                item_i = pattern_set[i][j];
                pattern_set_binary[i][item_i] = 1;
            }
        }

        cout << "Solving RMP ";
        cout << "The number of column binary is " << pattern_set_binary.size() << endl;

        try
        {
            GRBEnv *env;
            vector<GRBVar> x;
            env = new GRBEnv();
            GRBModel model = GRBModel(*env);
            // model.getEnv().set(GRB_IntParam_OutputFlag, 0);
            // model.set(GRB_IntParam_Threads, thread_limit);
            model.set(GRB_StringAttr_ModelName, "RMP_BIN");

            x.resize(num_pattern);

            for (long i = 0; i < num_pattern; ++i)
            {
                x[i] = model.addVar(0, 1, 0, GRB_CONTINUOUS);
            }
            model.update();

            // set covering problem, each item is covered by at least one set
            vector<GRBConstr> orders;
            orders.resize(bin.nitems);
            for (long j = 0; j < bin.nitems; ++j)
            {
                GRBLinExpr rtot = 0;
                for (long i = 0; i < num_pattern; ++i)
                {
                    rtot += pattern_set_binary[i][j] * x[i];
                }
                orders[j] = model.addConstr(rtot >= 1, "");
            }

            // objective
            GRBLinExpr objtot = 0;
            for (long i = 0; i < num_pattern; ++i)
            {
                objtot += x[i];
            }

            model.setObjective(objtot, GRB_MINIMIZE);
            model.update();

            // model.set(GRB_IntParam_Presolve, 0);
            model.optimize();
            lp_bound = model.get(GRB_DoubleAttr_ObjVal);
            cout << "LP bound is " << lp_bound << endl;

            for (long j = 0; j < bin.nitems; ++j)
            {
                dual_values[j] = orders[j].get(GRB_DoubleAttr_Pi);
            }



            // int cols = model.get(GRB_IntAttr_NumVars);

            k = 0;
            lp_vbasis.resize(num_pattern);
            for (long i = 0; i < num_pattern; ++i)
            {
                lp_vbasis[i] = x[i].get(GRB_IntAttr_VBasis);
                k += (lp_vbasis[i] == 0);
            }

            cout << k << endl;
            cout << "lp solution # basic variables: " << k << "/" << lp_vbasis.size() << "\n";

            delete env;
        }
        catch (GRBException e)
        {
            cout << "Error code " << endl;
            cout << e.getErrorCode() << e.getMessage() << endl;
        }
    }

    // adjacent vertices cannot be selected simultaneously
    bool CG::solve_knapsack_conflicted_gurobi(double cutoff, double &min_reduced_cost)
    {

        cout << "Solving Pricing problem ... " << endl;
        try
        {
            GRBEnv *env;
            vector<GRBVar> x;
            env = new GRBEnv();
            GRBModel model = GRBModel(*env);
            model.set(GRB_DoubleParam_TimeLimit, cutoff);
            model.getEnv().set(GRB_IntParam_OutputFlag, 0);
            model.set(GRB_StringAttr_ModelName, "MIN_KNAPSACK_CONFLICTED");

            x.resize(bin.nitems);
            for (int i = 0; i < bin.nitems; ++i)
            {
                x[i] = model.addVar(0, 1, 0, GRB_BINARY);
            }
            model.update();

            // adjacent vertices cannot be selected simultaneously
            for (int i = 0; i < bin.nitems; ++i)
            {
                for (int j = i + 1; j < bin.nitems; ++j)
                {
                    if (adj_matrix[i][j] == 1 && i != j)
                    {
                        model.addConstr(x[i] + x[j] <= 1, " ");
                    }
                }
            };

            // weight constraints
            GRBLinExpr totalweight = 0;
            for (long i = 0; i < x.size(); ++i)
            {
                totalweight += weight[i] * x[i];
            }
            model.addConstr(totalweight <= bin.capacity);

            model.update();
            model.set(GRB_IntParam_Presolve, 0);

            // the objective
            GRBLinExpr tot = 0;
            for (int i = 0; i < bin.nitems; ++i)
            {
                tot += x[i] * dual_values[i];
            }

            model.setObjective(tot, GRB_MAXIMIZE);
            model.optimize();
            cout << "Pricing problem obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
            min_reduced_cost = 1 - model.get(GRB_DoubleAttr_ObjVal);
            int status = model.get(GRB_IntAttr_Status);

            kanpsack_conflict_optimal = (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL);

            optimal_pattern.clear();
            if (kanpsack_conflict_optimal)
            {
                for (int i = 0; i < bin.nitems; i++)
                {
                    if (abs(x[i].get(GRB_DoubleAttr_X)) > 0.5)
                    {
                        optimal_pattern.push_back(i);
                    }
                }
            }
            delete env;
        }
        catch (GRBException e)
        {
            cout << "Error code " << endl;
            cout << e.getErrorCode() << e.getMessage() << endl;
        }

        cout << "kanpsack_conflict_optimal " << kanpsack_conflict_optimal << endl;
        return kanpsack_conflict_optimal;
    }

    void CG::collect_training_data(vector<vector<double>> &obj_coef, vector<vector<bool>> &solution)
    {
        initializing_pattern();
        cout << "Pattern is initialized!" << endl;

        min_reduced_cost = -1.0;

        while (min_reduced_cost < -0.000001)
        {
            cout << "Num of iter " << cg_iters << endl;
            cout << pattern_set.size() << endl;
            cout << "Solving restricted master problem" << endl;
            solve_restricted_master_problem();
            cout << "Solving pricing problem" << endl;
            solve_knapsack_conflicted_gurobi(1e8, min_reduced_cost);
            if (cg_iters % 5 == 0)
            {

                vector<bool> opt_sol(bin.nitems, false);
                for (auto item : optimal_pattern)
                {
                    opt_sol[item] = true;
                }
                obj_coef.push_back(dual_values);
                solution.push_back(opt_sol);
            }
            // add new columns
            pattern_set.push_back(optimal_pattern);
            // cout << pattern_set.size() << endl;
            cout << "minimum reduced cost is " << min_reduced_cost << endl;

            if (cg_iters++ >= 25)
                break;
        }
    }

    bool CG::solve_knapsack_conflicted_gurobi_set(double cutoff, double &min_reduced_cost)
    {

        cout << "Solving Pricing problem ... " << endl;
        try
        {
            GRBEnv *env;
            vector<GRBVar> x;
            env = new GRBEnv();
            GRBModel model = GRBModel(*env);
            model.set(GRB_DoubleParam_TimeLimit, cutoff);
            model.getEnv().set(GRB_IntParam_OutputFlag, 0);
            model.set(GRB_StringAttr_ModelName, "MIN_KNAPSACK_CONFLICTED");

            x.resize(bin.nitems);

            for (int i = 0; i < bin.nitems; ++i)
            {
                x[i] = model.addVar(0, 1, 0, GRB_BINARY);
            }
            model.update();

            // no-adjacent vertices cannot be selected simultaneously
            for (int i = 0; i < bin.nitems; ++i)
            {
                for (int j = i + 1; j < bin.nitems; ++j)
                {
                    if (adj_matrix[i][j] == 1 && i != j)
                    {
                        model.addConstr(x[i] + x[j] <= 1, " ");
                    }
                }
            };

            // weight constraints
            GRBLinExpr totalweight = 0;
            for (long i = 0; i < x.size(); ++i)
            {
                totalweight += weight[i] * x[i];
            }
            model.addConstr(totalweight <= bin.capacity);

            model.update();
            model.set(GRB_IntParam_Presolve, 0);

            // the objective
            GRBLinExpr tot = 0;
            for (int i = 0; i < bin.nitems; ++i)
            {
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

            for (int i = 0; i < nsol; i++)
            {
                vector<int> sol;
                model.set(GRB_IntParam_SolutionNumber, i);
                auto rc = 1 - model.get(GRB_DoubleAttr_PoolObjVal);

                if (rc < -0.000001)
                {
                    for (int j = 0; j < bin.nitems; j++)
                    {
                        if (x[j].get(GRB_DoubleAttr_Xn) > 0.5)
                        {
                            sol.push_back(j);
                        }
                    }
                    optimal_pattern_set.push_back(sol);
                }
            }
            delete env;
        }
        catch (GRBException e)
        {
            cout << "Error code " << endl;
            cout << e.getErrorCode() << e.getMessage() << endl;
        }

        return isOptimal;
    }

    void CG::test(int method, int method_type, std::ofstream *output_file_sampling_stats, std::ofstream *output_file_cg_stats)
    {

        double start_time = get_wall_time();

        initializing_pattern();

        min_reduced_cost_exact = -1.0;
        cg_iters = 0;
        double best_nrc = -1.0;
        double cutoff = 10000;
        double t0;
        double duration;
        bool knapsack_optimal;
        int num_ml_sampling_fail = 0;
        while (true)
        {
            cout << "iteration " << cg_iters++ << " : " << endl;
            t0 = get_wall_time();
            solve_restricted_master_problem();

            duration = get_wall_time() - t0;
            time_duration_master += duration;
            cout << "time used: " << duration << "\n";
            cout << "---------" << endl;
            cout << "current cg iter" << cg_iters << endl;
            if (output_file_cg_stats != nullptr)
            {
                (*output_file_cg_stats) << cg_iters << ","
                                        << get_wall_time() - start_time << ","
                                        << lp_bound << ",";
            }

            cout << "solve knapsack by heuristic \n";

            t0 = get_wall_time();
            Pricer *pricer = nullptr;

            if (method == 1) // MLACO
            {

                pricer = new MLACO(method, method_type, seed, cutoff, bin.nitems, bin.nitems, bin.nitems, bin.weight, bin.capacity, dual_values, bin.degree_norm, adj_matrix, adj_list, 0);
            }
            else if (method == 2) // MLMMAS
            {
                pricer = new MLMMAS(method, method_type, cutoff, bin.nitems, bin.nitems, bin.nitems, bin.weight, bin.capacity, dual_values, bin.degree_norm, adj_matrix, adj_list, 0);
            }

            else if (method == 3) // gurobi solver
            {
                pricer = new knapsack_solver(method, method_type, cutoff, dual_values, bin.capacity, bin.weight, bin.nitems, adj_matrix, adj_list, 0);
            }

            else if (method == 4) // classic ACO
            {
                pricer = new ACO(method, method_type, cutoff, bin.nitems, bin.nitems, bin.nitems, bin.weight, bin.capacity, dual_values, adj_matrix, adj_list, 0);
            }
            else if (method == 5) // classic MMAS
            {
                pricer = new ACO(method, method_type, cutoff, bin.nitems, bin.nitems, bin.nitems, bin.weight, bin.capacity, dual_values, adj_matrix, adj_list, 0);
            }
            else if (method == 6) // MLBIN
            {
                pricer = new MLBIN(method, cutoff, bin.nitems, bin.nitems, bin.capacity, bin.weight, dual_values, bin.degree_norm, adj_matrix, adj_list, 0);
            }
            cout << "Pricer set up " << endl;
            pricer->run();

            if (output_file_sampling_stats != nullptr)
            {
                for (auto i = 0; i < pricer->niterations; ++i)
                {
                    (*output_file_sampling_stats) << cg_iters << "," << i + 1 << ","
                                                  << pricer->num_neg_rc_current_iteration[i] << ","
                                                  << pricer->best_rc_current_iteration[i] << "\n";
                }
            }

            duration = get_wall_time() - t0;
            cout << "time used: " << duration << "\n";
            time_duration_pricing_heur += duration;

            if (pricer->num_neg_rc_col > 0)
            {
                num_heur_runs_success++;
                cout << "# columns with negative reduced cost found by pricer: " << pricer->num_neg_rc_col << "\n";
                cout << "minimum reduced cost by pricer is: " << pricer->best_rc << endl;
                // best_nrc = pricer->best_rc;
                pricer->include_new_cols_all(pattern_set);
                num_pattern = pattern_set.size();

                lg_bound = -1000;

            }
            else
            {
                num_ml_sampling_fail++;
                cout << "heuristic pricer does not find any column with negative reduced cost, using exact solver instead !\n";
                t0 = get_wall_time();
                cout << "solve pricing problem by Gurobi solver: \n";
                cout << "time budget used so far: " << (t0 - start_time) << "\n";
                auto knapsack_cutoff = cutoff - (t0 - start_time);

                //*** using exact Pattern set  ***
                knapsack_optimal = solve_knapsack_conflicted_gurobi(knapsack_cutoff, min_reduced_cost_exact);
                if (knapsack_optimal)
                {
                    cout << "minimum reduced cost by exact solver is: " << min_reduced_cost_exact << endl;
                    cout << "Current column size: " << pattern_set.size() << endl;
                    cout << "Adding column..." << pattern_set.size() << endl;
                    pattern_set.insert(pattern_set.end(), optimal_pattern_set.begin(), optimal_pattern_set.end());
                    cout << "Current column size: " << pattern_set.size() << endl;
                    num_pattern = pattern_set.size();
                }

                duration = get_wall_time() - t0;
                cout << "time used: " << duration << "\n";
                time_duration_pricing_heur += duration;

                best_nrc = min_reduced_cost_exact;
                lg_bound = lp_bound / (1 - best_nrc);
            }

            
            if (output_file_cg_stats != nullptr)
            {
                (*output_file_cg_stats) << pricer->num_neg_rc_col << ","
                                        << pricer->best_rc << ","
                                        << pricer->mean_rc << ","
                                        << pricer->median_rc << ","
                                        << pricer->stdev_rc << ","
                                        << lg_bound << endl;
            }
            cout << "lp_bound: " << lp_bound << "-----"
                 << "lg_bound: " << lg_bound << endl;
            cout << "best negative reduced cost " << best_nrc << endl;
            if (pricer != nullptr)
            {
                pricer = nullptr;
            }
            if (std::floor(lp_bound) == std::ceil(lg_bound))
                break;
            if (min_reduced_cost_exact > 0.0)
                break;
            if (get_wall_time() - start_time > cutoff)
                break;
            // cout << "min_reduced_cost_exact " << min_reduced_cost_exact << endl;

        }
        
        // cout << "check pattern include in RMP " << endl;
        // for (auto x=0; x < pattern_set.size(); x++)
        // {
        //     for (auto y=0; y < pattern_set[x].size(); y++)
        //     {
        //         cout << pattern_set[x][y] << " ";
        //     }
        //     cout << endl;
        // }


        lp_optimal = knapsack_optimal && min_reduced_cost_exact > -0.01;
        if (std::floor(lp_bound) == std::ceil(lg_bound))
        {
            lp_optimal = true;
        }
        cout << "lp_optimal: " << lp_optimal << endl;
    }

}
