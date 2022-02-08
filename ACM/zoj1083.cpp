// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364582
// 拓扑排序

#include <iostream>

using namespace std;

int l[26], r[26], t[26], b[26], e[26], ne;
int h, w;
int board[40][40];
int map[26][26];
int const none = '.' - 'A';

int in[26];
char seq[27];

void dfs(int dep = 0) {
    if (dep == ne) {
        seq[dep] = 0;
        printf("%s\n", seq);
        return;
    }
    for (int i = 0; i < 26; ++i) {
        if (e[i] && in[i] == 0) {
            seq[dep] = 'A' + i;
            e[i] = 0;
            for (int j = 0; j < 26; ++j) if (map[i][j]) --in[j];
            dfs(dep + 1);
            for (int j = 0; j < 26; ++j) if (map[i][j]) ++in[j];
            e[i] = 1;
        }
    }
}

int main() {
    char buf[40];
    while (cin >> h >> w) {
        for (int i = 0; i < 26; ++i) {
            e[i] = 0;
            l[i] = w;
            r[i] = 0;
            t[i] = h;
            b[i] = 0;
            ne = 0;
            for (int j = 0; j < 26; ++j) {
                map[i][j] = 0;
            }
        }
        for (int i = 0; i < h; ++i) {
            cin >> buf;
            for (int j = 0; j < w; ++j) {
                int ch = board[i][j] = buf[j] - 'A';
                if (ch != none) {
                    e[ch] = 1;
                    l[ch] = min(j, l[ch]);
                    r[ch] = max(j, r[ch]);
                    t[ch] = min(i, t[ch]);
                    b[ch] = max(i, b[ch]);
                }
            }
        }
        for (int n = 0; n < 26; ++n) {
            if (e[n] == none) continue;
            for (int i = t[n]; i <= b[n]; ++i) {
                for (int j = l[n]; j <= r[n]; ++j) {
                    if (i != t[n] && i != b[n] && j != l[n]) j = r[n];
                    if (board[i][j] != n) map[n][board[i][j]] = 1;
                }
            }
        }
        for (int j = 0; j < 26; ++j) {
            in[j] = 0;
            if (e[j]) {
                for (int i = 0; i < 26; ++i) in[j] += map[i][j];
                ++ne;
            }
        }
        dfs();
    }
    return 0;
}
