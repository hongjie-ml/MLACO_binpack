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
        
        

    }




}





