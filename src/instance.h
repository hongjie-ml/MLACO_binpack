#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <random>

using std::string;
using std::vector;
using std::mt19937;
using std::uniform_int_distribution;

namespace Bin{


    class Instance {

        
        int bestsolvalue;
        const string bin_file_name;
        const string conflict_file_name;
        std::string dataset_input_dir;
        std::string conflict_input_dir;


        
        void read_bpa();
        void read_conflict();
        void read_bintxt();
        

    public:
        int num_pricing = 0;
        vector<int> weight;
        vector<vector<double>> knapsack_obj_coefs;
        vector<vector<bool>> knapsack_sol;
        int nitems;
        int capacity;
        int n_edges;
        vector<vector<int>> adj_list;
        vector<double> degree_norm;
        float max_node_degree_norm;
        vector<int> degree;


        explicit Instance(string bin_file_name, string conflict_file_name, string input_dir, bool _solve);
        void load_train_data(std::string read_from);
        void collect_train_data(std::string save_to);
        void generate_random_pattern();

        

    };



}

#endif