import random
import networkx as nx
import numpy as np
import os
import sys

def generate_conflict_graph(input_BPfile, given_density, seed):

    with open(input_BPfile) as f:
        lines = f.readlines()
        nitems = int(lines[0])
        f.close()

    # it is undirected graph
    # given density, the number of edge
    nedge = round((given_density * nitems * (nitems-1))/2)



    G = nx.gnm_random_graph(nitems, nedge, seed,  directed=False)
    G.to_undirected()
    return G




if __name__ == "__main__":
    # loop dataset, generate same density graph
    density = float(sys.argv[1])
    seed = int(sys.argv[2])
    istrain = sys.argv[3]
    if istrain == "train":
        dataset_dir = "./train_data/binpacking_data/"
        conflict_dir = "./train_data/conflict_data/"+str(density)+"_seed_"+str(seed)+"/"
    else:
        dataset_dir = "./test_data/hard_test_data/"
        conflict_dir = "./test_data/hard_conflict_data/"+ str(density)+"_seed_"+str(seed)+"/"
    try:
        os.makedirs(conflict_dir)
    except FileExistsError:
        pass

    dataset_name_list = os.listdir(dataset_dir)
    # print(dataset_name_list)
    dataset_path_list = [os.path.join(dataset_dir, name) for name in  dataset_name_list]
    for dataset in dataset_name_list:
        dataset_path = os.path.join(dataset_dir, dataset)
        conflict_path = os.path.join(conflict_dir, dataset[:len(dataset)-4] + ".adjlist")
        # print(conflict_path)
        generated_G = generate_conflict_graph(dataset_path, density, seed)
        nx.write_adjlist(generated_G, conflict_path)
