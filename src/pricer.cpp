#include "pricer.h"
#include <limits>
#include <sys/time.h>
#include <cmath>
#include <numeric>
#include <algorithm>
using namespace std;
using namespace chrono;


namespace Bin{


    long Pricer::current_time_for_seeding(){
        long now = duration_cast< nanoseconds >(
        system_clock::now().time_since_epoch()).count();
        return now;
    }

    vector<long> Pricer::sort_indexes_inc(const vector<double> &v) {
        // initialize original index locations
        vector<long> idx(v.size());
        iota(idx.begin(), idx.end(), 0);
        stable_sort(idx.begin(), idx.end(),
            [&v](long i1, long i2) {return v[i1] < v[i2];});

        return idx;
    }

    double Pricer::get_wall_time(){
        struct timeval time;
        if (gettimeofday(&time,NULL)){
            return 0;
        }
        return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }


    
    void Pricer::compute_statistics(){
        // calculate statistics



        num_neg_rc_col = neg_rc_vals.size();
        if (num_neg_rc_col == 0){
            best_rc = 0.; mean_rc=0.; median_rc=0.; stdev_rc=0.;
        }else{
            sorted_indices = sort_indexes_inc(neg_rc_vals);
            best_rc = neg_rc_vals[sorted_indices[0]]; 
            median_rc = neg_rc_vals[sorted_indices[long(num_neg_rc_col/2)]];
            mean_rc = 0.;
            for (auto i = 0; i < num_neg_rc_col; ++i)
                mean_rc+=neg_rc_vals[i];
            mean_rc /= num_neg_rc_col;
            for (auto i = 0; i < num_neg_rc_col; ++i)
                stdev_rc +=  (neg_rc_vals[i]-mean_rc) * (neg_rc_vals[i]-mean_rc);
            stdev_rc = sqrt(stdev_rc/num_neg_rc_col);
        }
    }


    void Pricer::include_new_cols(vector<vector<int>>& basic_cols, vector<int>& lp_basis){

        if (column_selection == 1){
            include_new_cols_all(basic_cols);
        }
        else if (column_selection == 2){
            // 
            include_best_nrc_col(basic_cols);
        }


    }


    void Pricer::include_best_nrc_col(vector<vector<int>>& basic_cols){
        
    };





    void Pricer::include_new_cols_all(vector<vector<int>>& basic_cols){
        cout << "column selection method: include_new_cols_all\n";
        basic_cols.insert(basic_cols.end(), neg_rc_cols.begin(), neg_rc_cols.end());
        
        // cout << "Debugging ..." <<endl;
        // for (int x=0; x < neg_rc_cols.size(); x++){
        //     for (int y = 0; y < neg_rc_cols[x].size(); y++){
        //         cout << neg_rc_cols[x][y] << " ";
        //     }
        //     cout << endl;
        // }
        // cout << "Debugging ..." <<endl;
        // for (int x=0; x < neg_rc_vals.size(); x++){
        //     cout << neg_rc_vals[x] << " ";
        // }
        // cout << endl;


    }


    
    
    vector<double> Pricer::compute_nrc(vector<vector<int>>& basic_cols){
        vector<double> nrcs(basic_cols.size(), 1);

        for (auto i = 0; i < basic_cols.size(); ++i){
            for (auto v : basic_cols[i]){
                nrcs[i] -= dual_values[v];
            }
        }
        return nrcs;
    }
    

    


}

