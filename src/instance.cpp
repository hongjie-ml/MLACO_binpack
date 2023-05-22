#include "instance.h"

using std::endl;
using std::cout;
using std::ifstream;
using std::stringstream;


namespace Bin{


    Instance::Instance(string file_name, string input_dir, bool _solve) : 
        file_name{file_name}, input_dir{input_dir} {
    
        cout << file_name << endl;
        read_bpa();
    
    }


    void Instance::read_bpa(){

        string input_file = input_dir + file_name + ".bpa";
        ifstream file(input_file);
        string line;
        int w0;
        int line_num = 0;
        int v1 = 0;
        int idx;
        cout << "Reading " << input_file << endl;
        while (!file.eof()){
            getline(file, line);
            if (line_num == 1){
                stringstream stream(line);
                stream >> capacity >> nitems >> bestsolvalue;
                cout << "Bin Capacity is " << capacity << "; number of items is " << 
                nitems << "; the best solution value is " << bestsolvalue << endl;
                // weight.resize(nitems);
            }


            if (line_num > 1 && isdigit(line[0])){
                stringstream stream(line);
                stream >> w0;
                weight.push_back(w0);
            }
            line_num++;

        }   

    }



}




