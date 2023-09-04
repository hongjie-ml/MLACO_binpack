#include "training.h"
#include "MLBIN.h"

namespace Bin{

    Training::Training(std::vector<std::string> training_files, std::string dataset_input_dir, 
                    std::string conflict_input_dir , double alpha, int kernel_type) :
        training_files{training_files}, dataset_input_dir{dataset_input_dir},  conflict_input_dir{conflict_input_dir},
        alpha{alpha}, kernel_type{kernel_type} {
        std::cout << "Number of training file is " << training_files.size() << endl;
        construct_training_set();

    }



    void Training::construct_training_set(){
        std::string train_path = train_data_dir + train_file_name;
        char train_data[train_path.size() + 1];
        std::strcpy(train_data, train_path.c_str());
        std::ofstream train_file(train_data, std::ios::trunc);
        if (! train_file.is_open()){
            std::cout << "Cannot open the output file " <<  train_data << "\n";
            return;
        }
        train_file.close();

        std::uint64_t num0 = 0;
        std::uint64_t num1 = 0;

        for (auto d = 0; d < training_files.size(); ++d){
            auto bin = Instance(training_files[d], dataset_input_dir, conflict_input_dir, true);
            auto n = bin.nitems;


            const vector<vector<int>>& adj_list = bin.adj_list;
            vector<vector<bool>> adj_matrix(n, vector<bool>(n, 0));
            for (int i = 0; i < n; ++i){
                for (int j = 0; j < adj_list[i].size(); ++j){
                    adj_matrix[i][adj_list[i][j]] = 1;
                    adj_matrix[adj_list[i][j]][i] = 1;
                }
            } 

            std::cout << "number of pricing problems: " << bin.num_pricing << endl;
            


            cout << bin.knapsack_obj_coefs.size() << endl;
            for (int inst_idx = 0; inst_idx < bin.num_pricing; ++inst_idx){
                
                std::vector<double>& cur_pattern_obj_coef = bin.knapsack_obj_coefs[inst_idx];
                std::vector<bool>& cur_pattern_sol = bin.knapsack_sol[inst_idx];


                MLBIN mlbin(0, 0., 0., bin.nitems, bin.nitems, bin.capacity, bin.weight, cur_pattern_obj_coef, bin.degree_norm, adj_matrix, adj_list, 1e8);
                
                mlbin.random_sampling();
                mlbin.compute_correlation_based_measure();
                mlbin.compute_ranking_based_measure();
                train_file.open(train_data, std::ios::app);
                for (auto i = 0; i < n; ++i){
                    if (cur_pattern_sol[i]){
                        num1++;
                    }
                    else
                        num0++;
                    float val = cur_pattern_sol[i];
                    float corr_norm = (mlbin.max_cbm == 0) ? mlbin.corr_xy[i] : mlbin.corr_xy[i] / mlbin.max_cbm;
                    vector<int> adjacent_item;
                    int adjacent_item_min_weight;
                    int adjacent_item_max_weight;
                    adjacent_item_min_weight = bin.weight[bin.adj_list[i][0]];
                    adjacent_item_max_weight = bin.weight[bin.adj_list[i][0]];
                    for (int j = 0; j < bin.adj_list[i].size(); j++){
                        if (bin.weight[bin.adj_list[i][j]] < adjacent_item_min_weight){
                            adjacent_item_min_weight = bin.weight[bin.adj_list[i][j]];
                        }
                        if (bin.weight[bin.adj_list[i][j]] > adjacent_item_max_weight){
                            adjacent_item_max_weight = bin.weight[bin.adj_list[i][j]];
                        }
                    }

                    train_file << std::fixed << std::setprecision(0) << val << " ";
                    train_file << "1:" << std::fixed << std::setprecision(6) << mlbin.dual_values[i]/mlbin.max_dual << " ";
                    train_file << "2:" << std::fixed << std::setprecision(6) << (float)bin.weight[i]/(float)bin.capacity << " ";
                    train_file << "3:" << std::fixed << std::setprecision(6) << corr_norm << " ";
                    train_file << "4:" << std::fixed << std::setprecision(6) << mlbin.ranking_scores[i] << " ";
                    train_file << "5:" << std::fixed << std::setprecision(6) << bin.degree_norm[i] << endl;
                    // train_file << "6:" << std::fixed << std::setprecision(6) << adjacent_item_min_weight/(float)bin.capacity << " ";
                    // train_file << "7:" << std::fixed << std::setprecision(6) << adjacent_item_max_weight/(float)bin.capacity << endl;

                }
                train_file.close();
            }
            
        }
        std::cout << "num0 is " << num0 << "; " << "num1 is " << num1 << std::endl; 
        weight = alpha * num0/num1; 

    }


    void Training::generate_training_model_svm(){
        std::string train_s = train_data_dir  + train_file_name;
        std::string model_s = train_data_dir + svm_train_model_name;
        char train_data[train_s.size()+1];
        char model_file[model_s.size()+1];
        strcpy(train_data, train_s.c_str());
        strcpy(model_file, model_s.c_str());
        std::cout << "weight of class 1 is " << weight << std::endl;
        std::cout << "kernel type is " << kernel_type << std::endl;
        std::cout << "output probability is " << prob << std::endl;
        svm_train_model(train_data, model_file, 50, kernel_type, prob);
    }





}





