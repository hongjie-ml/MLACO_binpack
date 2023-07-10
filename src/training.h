#ifndef TRAINING_H
#define TRAINING_H
#include "instance.h"
#include <cstring>
#include <iomanip>


extern "C" {
#include "svm/svm_predict_model.h"
#include "svm/svm_train_model.h"
}

namespace Bin {
    class Training{

        std::vector<std::string> training_files;
        std::string input_dir;
        std::string train_data_dir = "../train_data/";
        std::string svm_train_model_name = "svm_train_model";
        std::string train_file_name = "train_data.txt";
        double alpha; //penalty of miss-classifying positive training instances
        int kernel_type;
        int prob = 0;
        double weight;

        public:

            explicit Training(std::vector<std::string> training_files, std::string input_dir, double alpha, int kernel_type);
            void construct_training_set();
            void generate_training_model_svm();



    };
}












#endif