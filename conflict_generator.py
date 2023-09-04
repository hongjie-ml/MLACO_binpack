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
    dataset_dir = "./Falkenauer/"
    conflict_dir = "./Falkenauer_conflict/"+str(density)+"_seed_"+str(seed)+"/"
    try:
        os.makedirs(conflict_dir)
    except FileExistsError:
        pass
    dataset_name_list = os.listdir("./Falkenauer/")
    dataset_path_list = [os.path.join(dataset_dir, name) for name in  dataset_name_list]
    for dataset in dataset_name_list:
        dataset_path = os.path.join(dataset_dir, dataset)
        conflict_path = os.path.join(conflict_dir, dataset[:len(dataset)-4] + ".adjlist")
        generated_G = generate_conflict_graph(dataset_path, density, seed)
        nx.write_adjlist(generated_G, conflict_path)