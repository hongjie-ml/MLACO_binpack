import sys, os
import numpy as np

def dual_to_primal():
    dual_path = './train_data/svm_train_model'
    primal_path = './train_data/svm.param'

    with open(dual_path, 'r') as f:
        lines = f.readlines()
        lines = [line.strip() for line in lines if len(line)!=0]
    
    nsv = int(lines[3].split(' ')[1]) 
    b = -float(lines[4].split(' ')[1])
    svs = lines[8:]
    assert(len(svs) == nsv)
    w = np.zeros(5, dtype=float)
    for sv in svs:
        tokens = sv.split(' ')
        coef = float(tokens[0])
        for i, feature in enumerate(tokens[1:]):
            feature = float(feature.split(':')[1])
            w[i] += feature*coef
    
    with open(primal_path, 'w+') as f:
        for weight in w.tolist():
            f.write(f'{weight}\n')
        f.write(f"{b}\n")

def train_svm_linear():
    os.system(f'cd ./build/ && ./Binpack 0')
    dual_to_primal()


if __name__ == '__main__':

    train_svm_linear()

