#ifndef PRICER_H
#define PRICER_H
#include <vector>
#include <iostream>
#include <chrono>

using std::cout;
using std::endl;
using std::vector;

namespace Bin{
    
    class Pricer {


    public:
        int column_selection;
        double cutoff;
        int sample_size;
        int niterations;
        int basis_factor;
        int upper_col_limit;

        vector<double> dual_values;
        vector<double> best_rc_current_iteration;
        vector<long> num_neg_rc_current_iteration;
        double heur_best_reduced_cost;
        const double INF = 1e8;

        int num_bin;
        vector<vector<int>> neg_rc_cols;
        vector<double> neg_rc_vals;
        std::vector<long> sorted_indices;
        long num_neg_rc_col=0;
        double best_rc = 0.;
        double mean_rc = 0.;
        double stdev_rc = 0.;
        double median_rc = 0.;
        double b0;
        double b1;
        double c0;
        double c1; 
        double c2; 
        double c3; 
        double c4;
        double b;

            
        vector<double> compute_nrc(vector<vector<int>>& basic_cols);




        virtual  ~Pricer(){};
        double get_wall_time();



        // sorting methods
        vector<long> sort_indexes_inc(const vector<double> &v);
        vector<long> sort_indexes_dec(const vector<double> &v);


        long current_time_for_seeding();
        // vector<long> sort_indexes_dec(const vector<double> &v); 
        

        void include_new_cols(vector<vector<int>>& basic_cols, vector<int>& lp_basis);    
    

        void compute_statistics();
        void include_new_cols_all(vector<vector<int>>& basic_cols);
        
        virtual void run() {std::cout << "error, in pricer!!!\n\n";};

    };


}









#endif
