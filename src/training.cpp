#include "training.h"
#include "MLBIN.h"




namespace Bin{

    Training::Training(std::vector<std::string> training_files, 
                    std::string input_dir, double alpha, int kernel_type): 
                    training_files{training_files}, input_dir{input_dir},alpha{alpha}, kernel_type{kernel_type}
    {
        std::cout << "Number of training file is " << training_files.size() << endl;

    }



    void Training::construct_training_set(){
        std::string train_path = train_data_dir + train_file_name;
        char train_data[train_path.size() + 1];
        strcpy(train_data, train_path.c_str());

        std::ofstream train_file(train_data, std::ios::trunc);
        if (! train_file.is_open()){
            std::cout << "Cannot open the output file " <<  train_data << "\n";
            return;
        }
        train_file.close();

        std::uint64_t num0 = 0;
        std::uint64_t num1 = 0;

        for (auto d = 0u; d < training_files.size(); ++d){
            auto bin = Instance(training_files[d], input_dir, true);
            auto n = bin.nitems;


            std::cout << "number of pricing problems: " << bin.num_pricing;
            for (auto inst_idx=0; inst_idx< bin.num_pricing; ++inst_idx){
                std::vector<double>& cur_pattern_obj_coef = bin.knapsack_obj_coefs[inst_idx];
                std::vector<bool>& cur_pattern_sol = bin.knapsack_sol[inst_idx];

                MLBIN mlbin(0);
                mlbin.random_sampling();
                mlbin.compute_features1();
                mlbin.compute_features2();
                train_file.open(train_data, std::ios::app);
                for (auto i = 0; i < n; ++i){
                    if (cur_pattern_sol[i]){
                        num1++;
                    }
                    else
                        num0++;
                    float val = cur_pattern_sol[i];
                    
                    train_file << val << " ";
                    train_file << "1:" << std::fixed << std::setprecision(6) << mlb
                }


                 
            }


        }


    }




}





