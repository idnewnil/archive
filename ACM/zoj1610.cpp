// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365109
// 解决思路：线段树
#include <iostream>

using namespace std;
int const no_color = -1;
int const multi_color = -2;

struct node {
    int l, r;
    int c;
};

node nodes[16384];
int cs[8001];
int current_color;
int max_color;

void reset_nodes() {
    nodes[1] = {0, 8192, -1};
    for (int i = 2; i < sizeof(nodes) / sizeof(nodes[0]); ++i) {
        int mid = (nodes[i >> 1].l + nodes[i >> 1].r) >> 1;
        nodes[i] = {i & 1 ? mid : nodes[i >> 1].l, i & 1 ? nodes[i >> 1].r : mid, -1};
    }
    current_color = -1;
    max_color = -1;
    for (int i = 0; i <= 8000; ++i) {
        cs[i] = 0;
    }
}

void set(int l, int r, int c, int i = 1) {
    if (nodes[i].c == c) {
        return;
    }

    if (nodes[i].l == l && nodes[i].r == r) {
        nodes[i].c = c;
        return;
    }

    int mid = (nodes[i].l + nodes[i].r) >> 1;
    if (nodes[i].c != multi_color) {
        nodes[i << 1].c = nodes[i].c;
        nodes[i << 1 | 1].c = nodes[i].c;
        nodes[i].c = multi_color;
    }
    if (l < mid) {
        set(l, min(mid, r), c, i << 1);
    }
    if (r > mid) {
        set(max(l, mid), r, c, i << 1 | 1);
    }
    if (nodes[i << 1].c == nodes[i << 1 | 1].c) {
        nodes[i].c = nodes[i << 1].c;
    }
}

void get(int i = 1) {
    if (nodes[i].c != multi_color) {
        if (current_color != nodes[i].c) {
            current_color = nodes[i].c;
            if (current_color != no_color) {
                ++cs[current_color];
            }
        }
    } else if (nodes[i].r - nodes[i].l >= 2) {
        get(i << 1);
        get(i << 1 | 1);
    }
}

int main() {
    int n;
    while (cin >> n) {
        reset_nodes();
        for (int i = 0; i < n; ++i) {
            int x1, x2, c;
            cin >> x1 >> x2 >> c;
            set(x1, x2, c);
            max_color = max(c, max_color);
        }
        get();
        for (int i = 0; i <= 8000; ++i) {
            if (cs[i]) {
                cout << i << ' ' << cs[i] << endl;
            }
        }
        cout << endl;
    }
    return 0;
}
