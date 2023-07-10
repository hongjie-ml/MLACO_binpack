#include "training.h"
#include "cg.h"
#include "MLBIN.h"
using Bin::Instance;
using Bin::Training;
using Bin::CG;
using Bin::MLBIN;
#include <time.h>
#include <sys/time.h>
#include <fstream>


#include <boost/filesystem.hpp>


using std::ofstream;
using std::stoi;

double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}

static double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

vector<string> file_name{
    "u10_00_0", 
    // "u20_00_0", 
    // "u40_00_0", 
    // "u60_00_0", "u80_00_0", "u100_00_0",
    // "u120_00_0", "u120_00_1", "u120_00_2", "u120_00_3", "u120_00_4", "u120_00_5", "u120_00_6", "u120_00_7", "u120_00_8", "u120_00_9", \
    // "u250_00_0", "u250_00_1", "u250_00_2", "u250_00_3", "u250_00_4", "u250_00_5", "u250_00_6", "u250_00_7", "u250_00_8", "u250_00_9", \
    // "u500_00_0", "u500_00_1", "u500_00_2", "u500_00_3", "u500_00_4", "u500_00_5", "u500_00_6", "u500_00_7", "u500_00_8", "u500_00_9", \
    // "u1000_00_0"
};


vector<string> train_file{
    "u10_00_0", "u20_00_0", "u40_00_0", "u60_00_0", "u80_00_0", "u100_00_0"};


void train_svm(){

    const string input_dir = "../data/";
    auto training = Training(train_file, input_dir, 1, 0);  // linear kernel type
    training.generate_training_model_svm();

}


void test(int method, int column_selection, int seed, string output_dir){

    for (int i = 0; i < file_name.size(); ++i){

        const string input_dir = "../data/";
        string input_file_name = file_name[i];
        cout << input_file_name << endl;
        const auto instance = Instance(input_file_name, input_dir, false);
        string output_cg_filename, output_solving_filename;
        output_cg_filename = output_dir + input_file_name + "_cg_stats.csv";
        output_solving_filename = output_dir + input_file_name + "_solving_stats.csv";
        ofstream output_file_cg_stats (output_cg_filename);
        ofstream output_file_solving_stats (output_solving_filename);

        if (output_file_cg_stats.is_open()){
                output_file_cg_stats <<"ith_CG_iter,current_time,lp_obj,nnrc_cols,min_rc,mean_rc,median_rc,stdev_rc,column_selection_time\n";
            } else{
                cout << "Cannot open the output file " + output_cg_filename << endl;

            }
            if (output_file_solving_stats.is_open()){
                output_file_solving_stats << "optimality,lp_obj,tot_time,tot_cpu_time,master_duration,heur_pricing_duration,exact_pricing_duration,#CG_iter,#added_columns,#heur_success" << endl;
            } else{
                cout << "Cannot open the output file " + output_solving_filename << endl;
        }

        ofstream output_file_sampling_stats;
        ofstream* output_file_sampling_stats_ptr;

        if (method == 1){ 
            output_file_sampling_stats_ptr = nullptr;
        }
        else if(method == 2) {
            string output_sampling_stats_filename;
            output_sampling_stats_filename = output_dir + input_file_name + "_sampling_stats.csv";
            output_file_sampling_stats.open(output_sampling_stats_filename);
            if (output_file_sampling_stats.is_open()){
                output_file_sampling_stats <<"ith_cg_iter,ith_mlbin_iter,#neg_rc,best_rc\n";
            } else{
                cout << "Cannot open the output file " + output_sampling_stats_filename << endl;
            }
            output_file_sampling_stats_ptr = &output_file_sampling_stats;
        }

        auto cg = CG(instance, seed, 1000);
        cout << " --- Problem instance ---" << input_file_name << endl;
        cout << "SOLVING ROOT LP BY CG\n"; 
        auto w0 = get_wall_time(); auto c0 = get_cpu_time();

        cg.test(method, output_file_sampling_stats_ptr, &output_file_cg_stats);

        auto wall_time_cg = get_wall_time() - w0;
        auto cpu_time_cg = get_cpu_time() - c0;
        cout << "WALL/CPU TOTAL TIME: " << wall_time_cg << ", " << cpu_time_cg << "\n";  


        output_file_solving_stats << cg.lp_optimal << ","  << cg.lp_bound << ","
                    << wall_time_cg << "," << cpu_time_cg << "," << cg.time_duration_master << ","
                    << cg.time_duration_pricing_heur <<"," << cg.time_duration_pricing_exact << ","
                    << cg.cg_iters << "," << cg.num_pattern - instance.nitems*1 << "," 
                    << cg.num_heur_runs_success << "\n";

        if (method == 1){
            output_file_sampling_stats.close();
            output_file_sampling_stats_ptr=nullptr;
        }
        output_file_cg_stats.close();
        output_file_sampling_stats.close();

    }


}




int main(int argc, char* argv[]) {


    int mode = stoi(argv[1]);


    if (mode == 5){
        train_svm();
    }
    else {
        int method = mode;
        string output_dir, seed;
        seed = argv[2];

        output_dir = "../results/";
        int column_selection = 0;

        if (method == 1){
            output_dir += "dynamic/seed_" + seed + "/";
        }
        else if (method == 2){
            output_dir += "mlbin/seed_" + seed + "/";
        }

        
        boost::filesystem::create_directories(output_dir);
        test(method, column_selection, stoi(seed), output_dir);
    }





}   