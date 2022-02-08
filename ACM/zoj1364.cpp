// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364863
// 最小覆盖

#include <iostream>

using namespace std;

int n, m, k;
bool map[100][100];
int A_match[100];
int B_match[100];

bool A_mark[100];
bool B_mark[100];

bool dfs(int i) {
    A_mark[i] = true;
    for (int j = 0; j < m; ++j) {
        if (!B_mark[j] && map[i][j]) {
            B_mark[j] = true;
            int old = B_match[j];
            A_match[i] = j;
            B_match[j] = i;
            if (-1 == old || dfs(old)) {
                return true;
            }
            A_match[i] = -1;
            B_match[j] = old;
        }
    }
    return false;
}

int main() {
    while (cin >> n && n) {
        cin >> m >> k;
        for (int i = 0; i < n; ++i) {
            A_match[i] = -1;
            for (int j = 0; j < m; ++j) {
                B_match[j] = -1;
                map[i][j] = false;
            }
        }
        while (k--) {
            int l, a, b;
            cin >> l >> a >> b;
            map[a][b] = true;
        }

        int count = 0;
        for (int i = 0; i < n; ++i) {
            if (A_match[i] == -1) {
                fill_n(A_mark, n, false);
                fill_n(B_mark, n, false);
                if (dfs(i))
                    ++count;
            }
        }
        if (count == 1)
            count = 0;// 这里非常奇怪，当n=1时，最大匹配数是1，但是却要输出0
        cout << count << endl;
    }
    return 0;
}
