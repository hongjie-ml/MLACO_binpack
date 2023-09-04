#include "instance.h"
#include "cg.h"
using std::endl;
using std::cout;
using std::ifstream;
using std::stringstream;
using std::string;
using std::ofstream;
using std::vector;
namespace Bin{
    static bool is_file_exist(const char *fileName)
    {
        std::ifstream infile(fileName);
        return infile.good();
    };

    Instance::Instance(string bin_file_name, std::string dataset_input_dir, 
                    std::string conflict_input_dir, bool _solve) : 
        bin_file_name{bin_file_name}, dataset_input_dir{dataset_input_dir},  conflict_input_dir{conflict_input_dir} {
        string opt_file_name = dataset_input_dir + bin_file_name + ".sol";

        cout << bin_file_name << endl;
        
        read_bintxt();
        read_conflict();
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

        // string input_file = input_dir + bin_file_name + ".bpa";
        // ifstream file(input_file);
        // string line;
        // int w0;
        // int line_num = 0;
        // int v1 = 0;
        // int idx;
        // cout << "Reading ... " << input_file << endl;
        // while (!file.eof()){
        //     getline(file, line);
        //     if (line_num == 1){
        //         stringstream stream(line);
        //         stream >> capacity >> nitems >> bestsolvalue;
        //         cout << "Bin Capacity is " << capacity << "; number of items is " << 
        //         nitems << "; the best solution value is " << bestsolvalue << endl;
        //     }

        //     if (line_num > 1 && isdigit(line[0])){
        //         stringstream stream(line);
        //         stream >> w0;
        //         weight.push_back(w0);
        //     }
        //     line_num++;
        // }   
    }
    
    // read bin txt benchmark dataset
    void Instance::read_bintxt(){
        string input_file = dataset_input_dir + bin_file_name + ".txt";
        cout << dataset_input_dir << "---" << bin_file_name << endl;
        ifstream file(input_file);
        string line;
        int line_num = 0;
        int w0;
        cout << "reading bin txt file ..." << "--" << input_file << endl;
        while (!file.eof()){
            getline(file, line);
            
            if (line_num == 0){
                stringstream stream(line);
                stream >> nitems;
                cout << "number of items" << nitems << " ";
            }
            if (line_num == 1){
                stringstream stream(line);
                stream >> capacity;
                cout << "capacity is : " << capacity << endl;
            }
            if (line_num > 1 && isdigit(line[0])){
                stringstream stream(line);
                stream >> w0;
                weight.push_back(w0);
            }
            line_num++;
        }
    }


    // if the binpack problem starts with 
    void Instance::read_conflict(){
        string input_file = conflict_input_dir + bin_file_name + ".adjlist";
        ifstream file(input_file);
        string line;
        char ch;
        int line_num = 0;
        cout << "Reading conflict file :" << input_file << endl;
        while (!file.eof()){
            getline(file, line);
            if (line_num > 2 && isdigit(line[0])){
                std::istringstream ss(line);
                adj_list.push_back({});
                int x;
                int node_num = 0;
                while (ss >> x){
                    if (node_num != 0){
                        adj_list.back().push_back(x);
                    }
                    node_num++;
                };
   
            }
            line_num++;
        }

        cout << "reading is done" << endl;

        for (int i = 0; i < adj_list.size(); i++){
            for (int j = 0; j < adj_list[i].size(); j++){
                adj_list[adj_list[i][j]].push_back(i);
            }
        }

        cout << "adj_list is constructed" << endl;
        // for (int i = 0; i < adj_list.size(); i++){
        //     for (int j = 0; j < adj_list[i].size(); j++){
        //         cout << adj_list[i][j] << " ,";
        //     }
        //     cout << endl;
        // }

        degree_norm = vector<double>(nitems);
        degree = vector<int>(nitems);
        max_node_degree_norm = 0.0;
        for (int i = 0; i < nitems; ++i){
            degree_norm[i] = (double) adj_list[i].size()/nitems;
            degree[i] = adj_list[i].size();
            if (max_node_degree_norm < degree_norm[i]){
                max_node_degree_norm = degree_norm[i];
            }
        }
    }



    void Instance::collect_train_data(string save_to){
        Bin::CG cg(*this, 1, 1000);

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
                while (stream >> dual)
                    obj_coef.push_back(dual);
                knapsack_obj_coefs.push_back(obj_coef);
                obj_coef.clear();
            }
            else {
                while (stream >> sol_val)
                    solution.push_back(sol_val);
                knapsack_sol.push_back(solution);
                solution.clear();
            }
            ctr++;
            }
        num_pricing = ctr/2;
        opt_file.close();


        }


    void Instance::generate_random_pattern(){
        int sample_size = 20;
        vector<vector<int>> pattern_set = vector<vector<int>> (sample_size);

        mt19937 mt(1314);
        uniform_int_distribution<int> dist(0,RAND_MAX);

        vector<int> candidates(nitems);
        int aggregated_weight;
        int nb_candidates, idx, item, num;

        for (int i = 0; i < sample_size; ++i){
            nb_candidates = nitems;
            aggregated_weight = 0;
            cout << "Sample " << i <<":";
            for (int j = 0; j < nb_candidates; ++j){
                candidates[j] = j;
            }

            while (nb_candidates > 0){
                if (nb_candidates == nitems){
                    idx = i % nitems;
                } else{
                    idx = dist(mt) % nb_candidates;
                }
                item = candidates[idx]; // item number_id
                cout << item << " ";
                pattern_set[i].push_back(item);
                aggregated_weight += weight[item];
                int remaining_capacity = capacity - aggregated_weight;
                
                num = 0;
                for (int j = 0; j < nb_candidates; ++ j){
                    if (remaining_capacity > weight[candidates[j]] && j != idx){
                        candidates[num] = candidates[j];
                        num++;
                    }
                }

                nb_candidates = num;
            }
            cout << endl;

            cout << "Total weight for current pattern: " << aggregated_weight << endl;

        cout << endl;}

        for (int i=0; i < pattern_set.size(); ++i){
            int total_weight = 0;
            for (int j =0; j < pattern_set[i].size(); ++j){
                total_weight+= weight[pattern_set[i][j] ];
                cout << pattern_set[i][j] << " ";
                 
            }
            cout << total_weight ;
            cout << endl;
        }


        }




}




