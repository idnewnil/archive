// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364905
// prim

#include <iostream>

using namespace std;

int n;
int cost_table[26][26];
int const inf = unsigned(-1) >> 1u;

int prim() {
    bool mark[26] = {true};
    int connected_towns[26] = {0};
    int connected_towns_count = 1;
    int total_cost = 0;

    while (connected_towns_count != n) {
        int min_from = -1;
        int min_to = -1;
        int min_cost = inf;
        for (int i = 0; i < connected_towns_count; ++i) {
            int from = connected_towns[i];
            for (int to = 0; to < n; ++to) {
                if (!mark[to] && min_cost > cost_table[from][to]) {
                    min_from = from;
                    min_to = to;
                    min_cost = cost_table[from][to];
                }
            }
        }
        mark[min_to] = true;
        connected_towns[connected_towns_count++] = min_to;
        total_cost += min_cost;
    }

    return total_cost;
}

int main() {
    while (cin >> n && n) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cost_table[i][j] = inf;
            }
        }
        for (int i = 0; i < n - 1; ++i) {
            char a;
            int ne;
            cin >> a >> ne;
            for (int j = 0; j < ne; ++j) {
                char b;
                int cost;
                cin >> b >> cost;
                cost_table[a - 'A'][b - 'A'] = cost_table[b - 'A'][a - 'A'] = cost;
            }
        }
        cout << prim() << endl;
    }
    return 0;
}


