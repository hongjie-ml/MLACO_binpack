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

#include <filesystem>
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




std::string dataset_input_dir = "../train_data/binpacking_data";
std::string easy_test_dir = "../test_data/easy_test_data/";
std::string hard_test_dir = "../test_data/hard_test_data/";

vector<string> train_file_name;
vector<string> easy_test_file_name;
vector<string> hard_test_file_name;


void get_train_file_name(){   
    for (const auto & entry : std::filesystem::directory_iterator(dataset_input_dir)){
        const auto filenameStr = entry.path().filename().string();
        train_file_name.push_back(filenameStr.substr(0, filenameStr.size() - 4));
    }
}

void get_easytest_file_name(){
    for (const auto & entry : std::filesystem::directory_iterator(easy_test_dir)){
        const auto filenameStr = entry.path().filename().string();
        if (filenameStr.find("Schwerin") != std::string::npos ){
            easy_test_file_name.push_back(filenameStr.substr(0, filenameStr.size() - 4));
        }
    }
}



void get_hardtest_file_name(){
    for (const auto & entry : std::filesystem::directory_iterator(hard_test_dir)){
        const auto filenameStr = entry.path().filename().string();
        hard_test_file_name.push_back(filenameStr.substr(0, filenameStr.size() - 4));
        cout << filenameStr << endl;
    }
}





void train_svm(){

    const string dataset_input_dir = "../train_data/binpacking_data/";
    const string conflict_input_dir = "../train_data/conflict_data/0.6_seed_1/";
    
    cout << dataset_input_dir << endl;
    cout << conflict_input_dir << endl;

    get_train_file_name();
    auto training = Training(train_file_name, dataset_input_dir, conflict_input_dir, 1, 0);  // linear kernel type
    training.generate_training_model_svm();
}



void test(int method, string density, int column_selection, int seed, string output_dir){

    for (int i = 0; i < hard_test_file_name.size(); ++i){

        const string dataset_input_dir = "../test_data/hard_test_data/";
        cout << density << endl;
        const string conflict_input_dir = "../test_data/hard_conflict_data/" + density + "_seed_1/";
        string input_file_name = hard_test_file_name[i];
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
            output_file_solving_stats << "nitems,capacity,optimality,lp_obj,tot_time,tot_cpu_time,master_duration,heur_pricing_duration,exact_pricing_duration,#CG_iter,#added_columns,#heur_success" << endl;
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

        output_file_solving_stats << instance.nitems << "," << instance.capacity << "," << cg.lp_optimal << ","  << cg.lp_bound << ","
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
        bool iseasy = stoi(argv[4]);
        if (iseasy){
            output_dir = "../easy_result/";
            get_easytest_file_name();
        }
        else if (!iseasy){
            output_dir = "../hard_result/";
            get_hardtest_file_name();
        }

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
        // cout << output_dir << endl;
        boost::filesystem::create_directories(output_dir);
        // cout << hard_test_file_name.size() << endl;
        test(method, density, column_selection, stoi(seed), output_dir);
    }

   

}   