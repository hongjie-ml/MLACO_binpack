#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using std::string;
using std::vector;

namespace Bin{


    class Instance {

        
        int bestsolvalue;
        const string file_name;
        const string input_dir;

        void read_bpa();


        public:
            vector<int> weight;
            int nitems;
            int capacity;

            explicit Instance(string file_name, string input_dir, bool _solve);

            


    };



}

#endif