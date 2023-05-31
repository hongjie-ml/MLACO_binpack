#include "instance.h"
#include "cg.h"
using std::endl;
using std::cout;
using std::ifstream;
using std::stringstream;
using std::string;
using std::ofstream;

namespace Bin{
    static bool is_file_exist(const char *fileName)
    {
        std::ifstream infile(fileName);
        return infile.good();
    };

    Instance::Instance(string file_name, string input_dir, bool _solve) : 
        file_name{file_name}, input_dir{input_dir} {
        string opt_file_name = input_dir + file_name + ".sol";
        cout << file_name << endl;
        read_bpa();
        if (_solve){
            if(is_file_exist(opt_file_name.c_str()))
                cout << "Collecting train data" << endl;
                collect_train_data(opt_file_name);
        }
        if (is_file_exist(opt_file_name.c_str()))
            cout << "Loading train data"<< endl;
            load_train_data(opt_file_name);
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
            }


            if (line_num > 1 && isdigit(line[0])){
                stringstream stream(line);
                stream >> w0;
                weight.push_back(w0);
            }
            line_num++;

        }   

    }



    void Instance::collect_train_data(string save_to){
        Bin::CG cg(*this);

        vector<vector<double>> obj_coef;
        vector<vector<bool>> solution;
        cg.collect_training_data(obj_coef, solution);
        int num_knapsack = obj_coef.size();

        ofstream opt_file(save_to);
        for (int i = 0; i < num_knapsack; ++i){
            for (int j = 0; j < nitems; ++j){
                opt_file << obj_coef[i][j];
                if (j!=nitems - 1) opt_file << " ";
            }
            opt_file << "\n";
            for (int j = 0; j < nitems; ++j){
                opt_file << solution[i][j];
                if (j!=nitems - 1) opt_file << " ";
            }
            opt_file << "\n";
        }
        opt_file.close();
    }


    void Instance::load_train_data(std::string read_from){
        ifstream opt_file(read_from);
        string line;
        bool sol_val;
        double dual;
        int ctr=0;

        std::vector<double> obj_coef;
        std::vector<bool> solution;

        while(!opt_file.eof()){
            
            getline(opt_file, line);
            if (line == "EOF" || line == "-1" || line.size()==0)
                break;
            
            std::stringstream stream(line);
            if (ctr % 2 == 0){
                while (stream >> dual){
                    obj_coef.push_back(dual);
                knapsack_obj_coefs.push_back(obj_coef);}
            }
            else {
                while (stream >> sol_val)
                    solution.push_back(sol_val);
                knapsack_sol.push_back(solution);
            }
            ctr++;
            }
        num_pricing = ctr/2;
        opt_file.close();
        }

}




