#ifndef MLBIN_H
#define MLBIN_H


#include <iostream>
#include <vector>
#include "pricer.h"

using std::vector;
using std::cout;
using std::endl;


namespace Bin{

    class MLBIN: public Pricer{

        std::vector<float> predicted_value;
        vector<vector<int>> pattern_set;
        std::vector<double> objs;
        double best_obj = 0.0;


        public:
            MLBIN(int _method);

            void random_sampling(); // sampling columns
            void compute_features1();
            void compute_features2();




    };



}












#endif