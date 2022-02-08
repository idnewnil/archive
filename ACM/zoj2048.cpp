// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365547
// 最小生成树prim
#include <iostream>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::pair;

struct pos {
    int x;
    int y;
};

struct way {
    int t1;
    int t2;
};

int const max_N = 760;
pos towns[max_N];
int distance[max_N][max_N];

vector<way> ways(max_N);
vector<int> tree_node(max_N);

void prim(int N) {
    ways.clear();
    tree_node.clear();

    bool mark[max_N]{}; // 是否在当前的生成树内
    mark[1] = true;
    tree_node.emplace_back(1); // 位于生成树内的town编号

    while (tree_node.size() != N) {
        way w = {0, 0};
        for (auto t1:tree_node) {
            if (distance[w.t1][w.t2] == 0) {
                break;
            }
            for (int t2 = 1; distance[w.t1][w.t2] != 0 && t2 <= N; ++t2) {
                if (!mark[t2] && distance[w.t1][w.t2] > distance[t1][t2]) {
                    w.t1 = t1;
                    w.t2 = t2;
                }
            }
        }
        mark[w.t2] = true;
        tree_node.emplace_back(w.t2);
        if (distance[w.t1][w.t2] != 0) {
            ways.emplace_back(w);
        }
    }
}

int main() {
    int t;
    cin >> t;
    while (t--) {
        int N, M;
        cin >> N;
        for (int n = 1; n <= N; ++n) {
            cin >> towns[n].x >> towns[n].y;
        }

        distance[0][0] = unsigned(-1) >> 1u;
        for (int i = 1; i <= N; ++i) {
            for (int j = 1; j <= N; ++j) {
                distance[i][j] = distance[j][i] = (towns[i].x - towns[j].x) * (towns[i].x - towns[j].x) +
                                                  (towns[i].y - towns[j].y) * (towns[i].y - towns[j].y);
            }
        }

        cin >> M;
        while (M--) {
            int t1, t2;
            cin >> t1 >> t2;
            distance[t1][t2] = distance[t2][t1] = 0;
        }

        prim(N);
        for (auto &way : ways) {
            cout << way.t1 << ' ' << way.t2 << endl;
        }

        if (t != 0) {
            cout << endl;
        }
    }
    return 0;
}
