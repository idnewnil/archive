// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365015
// 最大匹配

#include <iostream>
#include <cstring>

using namespace std;


int board[110][110];
bool edge[60][60];
int n, m;
int c;
int from[60];
int used[60];

bool dfs(int i) {
    for (int j = 0; j < c; ++j) {
        if (!used[j] && edge[i][j]) {
            used[j] = 1;
            if (from[j] == -1 || dfs(from[j])) {
                from[j] = i;
                return true;
            }
        }
    }
    return false;
}

int solve() {
    int ans = 0;
    memset(from, -1, sizeof(from));
    for (int i = 0; i < c; ++i) {
        memset(used, 0, sizeof(used));
        if (dfs(i))
            ans++;
    }
    return ans / 2;
}


int main() {
    while (cin >> n >> m && n && m) {
        memset(board, 0, sizeof(board));
        memset(edge, 0, sizeof(edge));
        int k;
        cin >> k;
        while (k--) {
            int i, j;
            cin >> i >> j;
            board[i - 1][j - 1] = -1;
        }
        c = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (board[i][j] != 0) continue;
                board[i][j] = c++;
                if (i > 0 && board[i - 1][j] != -1) {
                    edge[board[i][j]][board[i - 1][j]] = true;
                    edge[board[i - 1][j]][board[i][j]] = true;
                }
                if (j > 0 && board[i][j - 1] != -1) {
                    edge[board[i][j]][board[i][j - 1]] = true;
                    edge[board[i][j - 1]][board[i][j]] = true;
                }
            }
        }
        cout << solve() << endl;
    }
    return 0;
}

