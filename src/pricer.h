#ifndef PRICER_H
#define PRICER_H
#include <vector>
#include <iostream>


using std::cout;
using std::endl;
using std::vector;

namespace Bin{
    
    class Pricer {


    public:
        int method;
        int column_selection = 0;
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

        virtual  ~Pricer();
        virtual void run() {std::cout << "error, in pricer!!!\n\n";};

        vector<long> sort_indexes_inc(const vector<double> &v);
        long current_time_for_seeding();
        vector<long> sort_indexes_dec(const vector<double> &v); 


        double get_wall_time();

        void compute_statistics();

        



    }


}









#endif
