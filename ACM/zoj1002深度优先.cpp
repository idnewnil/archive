// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364501
// 解题思路：None
#include <iostream>

using namespace std;

int board[16];
int n;

void influence(int x, int y, int num) {
    board[y * n + x] += num;
    int point;

    for (int i = x - 1; i >= 0 && board[point = y * n + i] != -1; --i) {
        board[point] += num;
    }

    for (int i = x + 1; i < n && board[point = y * n + i] != -1; ++i) {
        board[point] += num;
    }

    for (int i = y - 1; i >= 0 && board[point = i * n + x] != -1; --i) {
        board[point] += num;
    }

    for (int i = y + 1; i < n && board[point = i * n + x] != -1; ++i) {
        board[point] += num;
    }
}

int find(int begin = 0) {
    int maximum = 0;
    for (int i = begin; i < n * n; ++i) {
        int y = i / n;
        int x = i % n;
        if (board[i] == 0) {
            influence(x, y, 1);
            maximum = max(maximum, find(i + 1) + 1);
            influence(x, y, -1);
        }
    }
    return maximum;
}

int main() {
    while (cin >> n && n > 0) {
        for (int i = 0; i < n * n; ++i) {
            char tem;
            cin >> tem;
            if (tem == '.') {
                board[i] = 0;
            } else {
                board[i] = -1;
            }
        }
        cout << find() << endl;
    }
    return 0;
}
