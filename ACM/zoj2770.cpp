#include <algorithm>
#include <iostream>
#include <queue>

using namespace std;

int n, m;
int map[1010][1010];
int dis[1010];
int vis[1010];
int const inf = (1u << 31u) - 1;

int main() {
    while (cin >> n >> m) {
        for (int i = 0; i <= n; ++i) {
            dis[i] = 0;
            vis[i] = 0;
            for (int j = 0; j <= n; ++j) {
                map[i][j] = inf;
            }
            map[i][i] = 0;
        }
        for (int i = 0; i < n; ++i) {
            int c;
            cin >> c;
            map[i + 1][i] = c;
            map[i][i + 1] = 0;
        }
        for (int l = 0; l < m; ++l) {
            int i, j, k;
            cin >> i >> j >> k;
            if (-k < map[i - 1][j]) map[i - 1][j] = -k;
        }
        deque<int> dq;
        int exist[21]{};
        for (int i = 0; i <= n; ++i) {
            dq.push_back(i);
            exist[i] = 1;
        }
        int flag = 0;
        while (!dq.empty()) {
            int cur = dq.front();
            dq.pop_front();
            for (int i = 0; i <= n; ++i) {
                if (map[cur][i] != inf) {
                    if (dis[cur] + map[cur][i] < dis[i]) {
                        dis[i] = dis[cur] + map[cur][i];
                        dq.push_back(i);
                        if (++vis[i] >= n) {
                            flag = 1;
                            goto end;
                        }
                        if (++vis[i] > n) {
                            flag = 1;
                            goto end;
                        }
                    }
                }
            }
            exist[cur] = 0;
        }
        end:
        if (flag == 1) {
            cout << "Bad Estimations" << endl;
        } else {
            cout << abs(dis[n] - dis[0]) << endl;
        }
    }
    return 0;
}
