// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364501
// 分析：https://my.oschina.net/u/4271175/blog/3265185
// 二部图

#include <iostream>
#include <algorithm>

using namespace std;

char board[4][5];
int x[4][4];
int x_count;
int y[4][4];
int y_count;
bool g[10][10]; // 图
int x_match[10];
int y_match[10];
int n;

bool x_mark[10];
bool y_mark[10];

bool dfs(int u) {
    x_mark[u] = true;
    for (int i = 0; i < y_count; ++i) {
        if (y_mark[i] || !g[u][i]) continue;
        if (y_match[i] == -1) {
            x_match[u] = i;
            y_match[i] = u;
            return true;
        } else {
            int old_match = y_match[i];
            if (x_mark[old_match]) continue;
            x_match[old_match] = -1;
            x_match[u] = i;
            y_match[i] = u;
            y_mark[i] = true;
            if (dfs(old_match)) {
                return true;
            }
            x_match[u] = -1;
            y_match[i] = old_match;
            x_match[old_match] = i;
        }
    }
    return false;
}

int main() {
    while (cin >> n && n) {
        for (int i = 0; i < n; ++i) {
            cin >> board[i];
        }

        x_count = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                x[i][j] = board[i][j] == 'X' ? -1 : j == 0 || x[i][j - 1] == -1 ? x_count++ : x[i][j - 1];
            }
        }

        y_count = 0;
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                y[i][j] = board[i][j] == 'X' ? -1 : i == 0 || y[i - 1][j] == -1 ? y_count++ : y[i - 1][j];
            }
        }

        for (int i = 0; i < y_count; ++i) {
            for (int j = 0; j < x_count; ++j) {
                g[i][j] = false;
                g[j][i] = false;
            }
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (x[i][j] == -1 || y[i][j] == -1) continue;
                g[x[i][j]][y[i][j]] = true;
                g[y[i][j]][x[i][j]] = true;
            }
        }

        fill_n(x_match, x_count, -1);
        fill_n(y_match, y_count, -1);
        int c = 0;
        for (int i = 0; i < x_count; ++i) {
            fill_n(x_mark, x_count, false);
            fill_n(y_mark, y_count, false);
            if (x_match[i] != -1 || dfs(i)) c++;
        }
        cout << c << endl;
    }
    return 0;
}
