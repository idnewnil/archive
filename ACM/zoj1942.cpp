// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365441
// 最短路径
#include <cmath>
#include <iostream>

using namespace std;

int n;
int scenario = 1;
double x[210];
double y[210];
double dis[210][210];

double calcDis(int i, int j) {
    return sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
}

int main() {
    while (scanf("%d", &n) && n) {
        for (int i = 0; i < n; ++i) {
            int a, b;
            scanf("%d%d", &a, &b);
            x[i] = a;
            y[i] = b;
            for (int j = 0; j < i; ++j) {
                dis[i][j] = dis[j][i] = calcDis(j, i);
            }
        }
        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                if (i == k) continue;
                for (int j = i + 1; j < n; ++j) {
                    if (j == k || i == j) continue;
                    dis[i][j] = dis[j][i] = min(dis[i][j], max(dis[i][k], dis[k][j]));
                }
            }
        }
        printf("Scenario #%d\nFrog Distance = %.3f\n\n", scenario, dis[0][1]);
        ++scenario;
    }
    return 0;
}
