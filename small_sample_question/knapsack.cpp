#include <vector>
#include <iostream>
#include <algorithm>
using std::vector;
using std::cout;
using std::endl;
using std::max;


int knapsack_dp(int W, int N, vector<int> weight, vector<int> val);
int knapsack_greedy(int W, int N, vector<int> weight, vector<int> val);


int main(void){
    vector<int> val{50, 100, 150, 200};
    vector<int> weight{8, 16, 32, 40};
    int W = 64;
    int n = val.size();

    int sol = knapsack_dp(64, n, weight,val);
    cout << sol << endl;


}



int knapsack_dp(int W, int N, vector<int> weight, vector<int> val){

    vector<vector<int>> dp(N+1, vector<int>(W+1, 0));
    for (int i = 1; i < N+1; i++){
        for (int w = 1; w < W+1; w++){
            if (w < weight[i - 1]){
                dp[i][w] = dp[i - 1] [w];
            }
            else{
                dp[i][w] = max(dp[i-1][w-weight[i-1]] + val[i-1], dp[i-1][w]);
                cout << dp[i][w] << endl;
            }
        }
    }

    return dp[N][W];


}