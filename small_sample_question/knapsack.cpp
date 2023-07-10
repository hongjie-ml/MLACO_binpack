#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;



int knapsack_dp(int W, int N, vector<int> weight, vector<double> val);
int knapsack_greedy(int W, int N, vector<int> weight, vector<int> val);
double cal_max(double a, double b);


int main(void){
    vector<double> val{0.4,0.6,0.6,1,0.4,0,0,0,0.4,0.6,0.2,0,0.8,0.4,0,1,1,0.4,0.6,0,0,0.6,1,0.6,1,0.6,0.4, 0,0,1,0,0.4,0,0.2,0.4,0,0.4,0.6,1,0.4};
    vector<int> weight{40, 56, 79, 163, 52, 57, 19, 3, 52, 110, 54, 34, 151, 100, 28, 114, 135, 76, 135, 21, 41, 67, 91, 106, 155, 35, 138, 52, 21, 168, 5, 98, 26, 35, 104, 12, 63, 125, 150, 61};
    int W = 200;
    int n = val.size();

    int sol = knapsack_dp(W, n, weight,val);
    cout << sol << endl;


}

double cal_max(double a, double b)
{
    return (a > b) ? a : b;
}


int knapsack_dp(int W, int N, vector<int> weight, vector<double> val){

    vector<vector<double>> dp(N+1, vector<double>(W+1, 0));
    for (int i = 1; i < N+1; i++){
        for (int w = 1; w < W+1; w++){
            if (w < weight[i - 1]){
                dp[i][w] = dp[i - 1] [w];
            }
            else{
                cout << dp[i-1][w-weight[i-1]] + val[i-1] << " " << dp[i-1][w] << endl;
                dp[i][w] = cal_max(dp[i-1][w-weight[i-1]] + val[i-1], dp[i-1][w]);
                cout << dp[i][w] << endl;
            }
        }
    }

    return dp[N][W];


    



}

