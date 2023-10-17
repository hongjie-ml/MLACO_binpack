#include "knapsack_solver.h"
#include "gurobi_c++.h"

namespace Bin
{

    knapsack_solver::knapsack_solver(int _method, int _method_type, double _cutoff, const vector<double> &_dual_values,
                                     int _capacity, const vector<int> &_weight, int _nitems, vector<vector<bool>> _adj_matrix, vector<vector<int>> _adj_list,
                                     long long _upper_col_limit)
    {
        dual_values = _dual_values;
        method_type = _method_type;
        cutoff = _cutoff;
        capacity = _capacity;
        weight = _weight;
        nitems = _nitems;
        adj_matrix = _adj_matrix;
        adj_list = _adj_list;
    }

    double cal_max(double a, double b)
    {
        return (a > b) ? a : b;
    }

    // solve knapsack problem using dp
    // void knapsack_solver::solve_knapsack_dp(){
    //     // // dual value is regarded as (value of item)

    //     // vector<vector<double>> table(nitems+1, vector<double>(capacity+1, 0));
    //     // for (int i = 1; i < nitems+1; i++){
    //     //     for (int w = 1; w < capacity+1; w++){
    //     //         if (w < weight[i - 1]){
    //     //             table[i][w] = table[i - 1] [w];
    //     //         }
    //     //         else{
    //     //             table[i][w] = cal_max(table[i-1][w-weight[i-1]] + dual_values[i-1], table[i-1][w]);
    //     //         }

    //     //     }
    //     // }
    //     // best_obj = table[nitems][capacity];

    //     // double res = table[nitems][capacity];
    //     // int w = capacity;
    //     // vector<int> optimal_pattern_binary;
    //     // optimal_pattern_binary = vector<int>(nitems, 0);

    //     // vector<int> selected_item;
    //     // int total_weight = 0;
    //     // int c = capacity;
    //     // for (int i = nitems; i > 0; --i){
    //     //     if (table[i][c] > table[i - 1][c]){
    //     //         optimal_pattern_binary[i-1] = 1;
    //     //         total_weight += weight[i-1];
    //     //         c = c - weight[i - 1];
    //     //     }
    //     // }

    //     // for (int i = 0; i < optimal_pattern_binary.size(); ++i){
    //     //     if (optimal_pattern_binary[i] != 0){
    //     //         optimal_pattern.push_back(i);
    //     //     }
    //     // }
    //     // isOptimal = true;

    //     // cout << "best obj is " << best_obj << endl;
    //     // exact_rc = 1 - best_obj;
    //     // cout << "exact rc is " << exact_rc << endl;

    //     // if (exact_rc > -0.0000000000001){
    //     // }
    //     // else {
    //     //     neg_rc_cols.push_back(optimal_pattern);
    //     //     neg_rc_vals.push_back(exact_rc);
    //     // }
    //     }

    void knapsack_solver::run()
    {

        solve_knapsackGraph_gurobi();
        compute_statistics();
    }

    void knapsack_solver::solve_knapsackGraph_gurobi()
    {

        cout << "Solving Pricing problem ... " << endl;

        GRBEnv *env;
        vector<GRBVar> x;
        env = new GRBEnv();
        GRBModel model = GRBModel(*env);
        model.set(GRB_DoubleParam_TimeLimit, cutoff);
        model.getEnv().set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_StringAttr_ModelName, "MIN_KNAPSACK_CONFLICTED");

        x.resize(nitems);

        for (long i = 0; i < nitems; ++i)
        {
            x[i] = model.addVar(0, 1, 0, GRB_BINARY);
        }
        model.update();

        // no-adjacent vertices cannot be selected simultaneously
        for (int i = 0; i < nitems; ++i)
        {
            for (int j = i + 1; j < nitems; ++j)
            {
                if (adj_matrix[i][j] == 0)
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
        model.addConstr(totalweight <= capacity);

        model.update();
        model.set(GRB_IntParam_Presolve, 0);

        // the objective
        GRBLinExpr tot = 0;
        for (int i = 0; i < nitems; ++i)
        {
            tot += x[i] * dual_values[i];
        }

        model.setObjective(tot, GRB_MAXIMIZE);
        model.optimize();

        cout << "Pricing problem obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
        isOptimal = (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL);
        exact_rc = 1 - model.get(GRB_DoubleAttr_ObjVal);
        cout << "exact_rc: " << exact_rc << endl;

        // gurobi classic
        vector<int> sol;
        if (method_type == 0)
        {
            if (isOptimal)
            {
                neg_rc_vals.push_back(exact_rc);
                for (int i = 0; i < nitems; i++)
                {
                    if (abs(x[i].get(GRB_DoubleAttr_X)) > 0.5)
                    {
                        sol.push_back(i);
                    }
                }
                neg_rc_cols.push_back(sol);
            }
        }
        else if (method_type == 1)
        {
            auto nsol = model.get(GRB_IntAttr_SolCount);
            for (int i = 0; i < nsol; i++)
            {
                model.set(GRB_IntParam_SolutionNumber, i);

                auto rc = 1 - model.get(GRB_DoubleAttr_PoolObjVal);
                if (rc < -0.000001)
                {

                    neg_rc_vals.push_back(rc);
                    for (int j = 0; j < nitems; ++j)
                    {
                        if (x[j].get(GRB_DoubleAttr_Xn) > 0.5)
                        {
                            sol.push_back(j);
                        }
                    }
                    neg_rc_cols.push_back(sol);
                }
            }
        }

        // gurobi heuristic

        delete env;
    }

    // void knapsack_solver::postprocessing(){

    //     // cout << "size is " << optimal_pattern.size() << endl;

    // }

}