// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364720
// 最短路径

#include <iostream>
#include <sstream>

using namespace std;

int map[21][21];
int dis[21];
int const inf = 100000;

int main() {
    int n;
    int test = 1;
    while (cin >> n) {
        for (int i = 1; i <= 20; ++i) {
            for (int j = 1; j <= 20; ++j) {
                map[i][j] = inf;
            }
            map[i][i] = 0;
        }
        for (int i = 1; i < 20; ++i) {
            if (i != 1) cin >> n;
            for (int j = 0; j < n; ++j) {
                int k;
                cin >> k;
                map[i][k] = map[k][i] = 1;
            }
        }
        for (int k = 1; k <= 20; ++k) {
            for (int i = 1; i <= 20; ++i) {
                if (i == k) continue;
                for (int j = i + 1; j <= 20; ++j) {
                    if (j == k) continue;
                    if (map[i][j] > map[i][k] + map[k][j]) {
                        map[i][j] = map[j][i] = map[i][k] + map[k][j];
                    }
                }
            }
        }
        int t;
        cin >> t;
        cout << "Test Set #" << test++ << endl;
        for (int i = 0; i < t; ++i) {
            int u, v;
            cin >> u >> v;
            cout << u << " to " << v << ": " << map[u][v] << endl;
        }
        cout << endl;
    }
    return 0;
}
