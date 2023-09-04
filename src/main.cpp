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
using namespace Bin;

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

    "Falkenauer_u250_00","Falkenauer_u250_01", "Falkenauer_u250_02", "Falkenauer_u250_03", 
    "Falkenauer_u250_04", "Falkenauer_t249_00", "Falkenauer_t249_01", "Falkenauer_t249_02", 
    "Falkenauer_t249_03", "Falkenauer_t249_04"
// 
// "Falkenauer_t120_01", 
    // "Falkenauer_t249_19", 
// "Falkenauer_t249_04", "Falkenauer_t501_05", "Falkenauer_t249_06", "Falkenauer_t249_07",
// "Falkenauer_t249_08", "Falkenauer_t249_09"
// "Falkenauer_t501_08"
// "Falkenauer_t249_17", "Falkenauer_t249_08",
//  "Falkenauer_t249_12", "Falkenauer_t249_16", , "Falkenauer_t249_18", 
//  "Falkenauer_t249_11", "Falkenauer_t249_05", "Falkenauer_t249_14", "Falkenauer_t249_09"
//  "Falkenauer_t249_02", "Falkenauer_t249_03", "Falkenauer_t249_01", "Falkenauer_t249_13", 
//  "Falkenauer_t249_06",  "Falkenauer_t249_07", "Falkenauer_t249_10", "Falkenauer_t249_15",
//   "Falkenauer_t501_05",  "Falkenauer_t501_01", "Falkenauer_t501_12", "Falkenauer_t501_08",
//  "Falkenauer_t501_11", "Falkenauer_t501_18", "Falkenauer_t501_13", "Falkenauer_t501_02", 
//  "Falkenauer_t501_00",
//   "Falkenauer_t501_16", "Falkenauer_t501_03", "Falkenauer_t501_17", "Falkenauer_t501_06",
//  "Falkenauer_t501_10", "Falkenauer_t501_09", "Falkenauer_t501_04", "Falkenauer_t501_14",
//  "Falkenauer_t501_19", "Falkenauer_t501_07", "Falkenauer_t501_15", "Falkenauer_u250_01",
//  "Falkenauer_u250_13", "Falkenauer_u250_16", "Falkenauer_u250_09", "Falkenauer_u250_05",
//  "Falkenauer_u250_03", "Falkenauer_u250_17", "Falkenauer_u250_14", "Falkenauer_u250_08",
//  "Falkenauer_u250_18", "Falkenauer_u250_06", "Falkenauer_u250_04", "Falkenauer_u250_15",
//  "Falkenauer_u250_11", "Falkenauer_u250_10", "Falkenauer_u250_19", "Falkenauer_u250_12",
//  "Falkenauer_u250_02", "Falkenauer_u250_07", "Falkenauer_u250_00", "Falkenauer_u500_13",
//  "Falkenauer_u500_14", "Falkenauer_u500_18", "Falkenauer_u500_03", "Falkenauer_u500_15",
//  "Falkenauer_u500_04", "Falkenauer_u500_08", "Falkenauer_u500_17", "Falkenauer_u500_16",
//  "Falkenauer_u500_06", "Falkenauer_u500_01", "Falkenauer_u500_09", "Falkenauer_u500_12",
//  "Falkenauer_u500_05", "Falkenauer_u500_10", "Falkenauer_u500_02", "Falkenauer_u500_19",
//  "Falkenauer_u500_11", "Falkenauer_u500_00", "Falkenauer_u500_07"
};


vector<string> train_file{


    "Falkenauer_t60_00" , "Falkenauer_t60_01" , "Falkenauer_t60_02", "Falkenauer_t60_03", "Falkenauer_t60_04", 
    "Falkenauer_t60_05", "Falkenauer_t60_06", "Falkenauer_t60_07", "Falkenauer_t60_08", "Falkenauer_t60_09",
    "Falkenauer_u120_00", "Falkenauer_u120_01", "Falkenauer_u120_02", "Falkenauer_u120_03", "Falkenauer_u120_04", 
    "Falkenauer_u120_05", "Falkenauer_u120_06", "Falkenauer_u120_07", "Falkenauer_u120_08", "Falkenauer_u120_09"
    //  "Falkenauer_t60_10" , "Falkenauer_t60_11" , "Falkenauer_t60_12", "Falkenauer_t60_13", "Falkenauer_t60_14", 
    // "Falkenauer_t60_15", "Falkenauer_t60_16", "Falkenauer_t60_17", "Falkenauer_t60_18", "Falkenauer_t60_19",
    };



void train_svm(){

    const string dataset_input_dir = "../binpacking_train/";
    const string conflict_input_dir = "../Falkenauer_conflict/0.6_seed_1/";
    auto training = Training(train_file, dataset_input_dir, conflict_input_dir, 1, 0);  // linear kernel type
    training.generate_training_model_svm();
}


void test(int method, string density, int column_selection, int seed, string output_dir){

    for (int i = 0; i < file_name.size(); ++i){
        const string dataset_input_dir = "../Falkenauer/";
        cout << density << endl;
        const string conflict_input_dir = "../Falkenauer_conflict/" + density + "_seed_1/";
        string input_file_name = file_name[i];
        cout << input_file_name << endl;

        // read the test file into Instance object
        const auto instance = Instance(input_file_name, dataset_input_dir, conflict_input_dir, false);
        
        // output file setup
        string output_cg_filename, output_solving_filename;
        output_cg_filename = output_dir + input_file_name + "_cg_stats.csv"; 
        output_solving_filename = output_dir + input_file_name + "_solving_stats.csv";
        ofstream output_file_cg_stats (output_cg_filename);
        ofstream output_file_solving_stats (output_solving_filename);

        if (output_file_cg_stats.is_open()){
                output_file_cg_stats <<"ith_CG_iter,current_time,lp_obj,nnrc_cols,min_rc,mean_rc,median_rc,stdev_rc,Lagrangian_bound\n";
        } 
        else{
                cout << "Cannot open the output file " + output_cg_filename << endl;}

        if (output_file_solving_stats.is_open()){
            output_file_solving_stats << "optimality,lp_obj,tot_time,tot_cpu_time,master_duration,heur_pricing_duration,exact_pricing_duration,#CG_iter,#added_columns,#heur_success" << endl;
            } else{
                cout << "Cannot open the output file " + output_solving_filename << endl;
        }

        ofstream output_file_sampling_stats;
        ofstream* output_file_sampling_stats_ptr;

        if (method == 1){
            output_file_sampling_stats_ptr = nullptr;  // method gurobi has no sampling method
        }
        else if(method >= 2){
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
        
        // Solving stage
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

        if (method >= 2){
            assert(output_file_sampling_stats_ptr != nullptr);
            output_file_sampling_stats.close();
            output_file_sampling_stats_ptr=nullptr;
        }

        output_file_cg_stats.close();
        output_file_solving_stats.close();

    }


}

       


int main(int argc, char* argv[]) {


    int mode = stoi(argv[1]);


    if (mode == 0){
        train_svm();
    }
    else {
        int method = mode;
        string output_dir, seed, density;
        seed = argv[2];
        density = argv[3];
        output_dir = "../results/";
        int column_selection = 0;

        if (method == 1){
            output_dir += "gurobi/density_" + density + "/seed_" + seed + "/";
        }
        else if (method == 2){
            output_dir += "mlbin/density_"  + density + "/seed_" + seed + "/";
        }
        else if (method == 3){
            output_dir += "aco/density_" + density + "/seed_" + seed + "/";
        }

        
        boost::filesystem::create_directories(output_dir);
        test(method, density, column_selection, stoi(seed), output_dir);
    }

   
    // vector<vector<double>> obj_coef;
    // vector<vector<bool>> solution;    


    // cg.collect_training_data(obj_coef, solution);
    

    // for (int i = 0; i < inst.nitems; i++){
    //     for (int j = 0; j < inst.adj_list[i].size(); j++){
    //         cout << inst.adj_list[i][j] << " ,";
    //     }
    //     cout << endl;
    // }


}   