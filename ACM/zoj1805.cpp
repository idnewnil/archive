// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365304
// 解题思路：四分树
#include <iostream>
#include <vector>

using namespace std;

struct node {
    int sub_node[4];

    node() : sub_node{} {}

    node(int a, int b, int c, int d) : sub_node{a, b, c, d} {}

    bool operator==(node const &that) {

        for (int i = 0; i < 4; ++i) {
            if (this->sub_node[i] != that.sub_node[i]) {
                return false;
            }
        }
        return true;
    }
};

char board[128][129];
vector<node> nodes(10010);
int quadtree_node;
int squadtree_node;

int handle(int x, int y, int width) {
    int record = squadtree_node;
    quadtree_node += 1;
    squadtree_node += 1;
    if (width == 1) {
        return board[y][x] == '0' ? 0 : 1;
    }

    int new_width = width >> 1;
    node curr(handle(x, y, new_width), handle(x + new_width, y, new_width), handle(x, y + new_width, new_width),
              handle(x + new_width, y + new_width, new_width));
    if (curr == nodes[0]) {
        quadtree_node -= 4;
        squadtree_node -= 4;
        return 0;
    }
    if (curr == nodes[1]) {
        quadtree_node -= 4;
        squadtree_node -= 4;
        return 1;
    }
    for (int i = 0; i < nodes.size(); ++i) {
        if (curr == nodes[i]) {
            squadtree_node = record;
            return i;
        }
    }
    nodes.emplace_back(curr);
    return nodes.size() - 1;
}

void handle(int n, int m) {
    nodes.resize(0);
    nodes.emplace_back(node(0, 0, 0, 0));
    nodes.emplace_back(node(1, 1, 1, 1));
    quadtree_node = 0;
    squadtree_node = 0;
    int width = 1;
    while (width < n || width < m) width <<= 1;
    for (int i = n; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            board[i][j] = '0';
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = m; j < width; ++j) {
            board[i][j] = '0';
        }
    }
    handle(0, 0, width);
    cout << quadtree_node << ' ' << squadtree_node << endl;
}

int main() {
    int n, m;
    while (cin >> n >> m && n && m) {
        for (int i = 0; i < n; ++i) {
            cin >> board[i];
        }
        handle(n, m);
    }
    return 0;
}


// 网上代码
#include<iostream>
#include<cstdio>
#include<cstring>

using namespace std;

struct state {
    int s1, s2, s3, s4;

    state() {}

    state(int n1, int n2, int n3, int n4) {
        s1 = n1, s2 = n2, s3 = n3, s4 = n4;
    }

    bool check(int n1, int n2, int n3, int n4) {
        return s1 == n1 && s2 == n2 && s3 == n3 && s4 == n4;
    }
} p[10010];

int tot;
char s[130][130];
int a[130][130];
int vis[20];
int c1, c2;

int div(int x1, int y1, int x2, int y2) {
    int tmp = c1;
    c1++, c2++;
    if (x1 == x2 && y1 == y2) return a[x1][y1];
    int x3 = x1 + x2 >> 1, y3 = y1 + y2 >> 1;
    int d1 = div(x1, y1, x3, y3);
    int d2 = div(x1, y3 + 1, x3, y2);
    int d3 = div(x3 + 1, y1, x2, y3);
    int d4 = div(x3 + 1, y3 + 1, x2, y2);
    if (d1 == 0 && d2 == 0 && d3 == 0 && d4 == 0) {
        c1 -= 4, c2 -= 4;
        return 0;
    }
    if (d1 == 1 && d2 == 1 && d3 == 1 && d4 == 1) {
        c1 -= 4, c2 -= 4;
        return 1;
    }
    for (int i = 2; i < tot; i++)
        if (p[i].check(d1, d2, d3, d4)) {
            c1 = tmp;
            return i;
        }
    p[tot++] = state(d1, d2, d3, d4);
    return tot - 1;
}

int main0() {
    int n, m;
    while (cin >> n >> m, n) {
        memset(a, 0, sizeof a);
        for (int i = 0; i < n; i++) {
            scanf("%s", s[i]);
            for (int j = 0; s[i][j]; j++)
                a[i][j] = s[i][j] - '0';
        }
        int k = 1;
        if (n < m) swap(n, m);
        while (k < n) k <<= 1;
        memset(vis, 0, sizeof vis);
        tot = 2, c1 = c2 = 0;
        div(0, 0, k - 1, k - 1);
        printf("%d %d\n", c2, c1);
    }
    return 0;
}
