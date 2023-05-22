#include "instance.h"
#include "cg.h"


using Bin::Instance;

int main(void) {


    const string input_dir = "../data/";

    const auto instance = Instance("u20_00", input_dir, false);

    
    auto bin = Bin::CG(instance);
    
    cout << bin.dual_values.size() << endl;
    

    bin.initializing_pattern();
    bin.solve_restricted_master_problem();

}   