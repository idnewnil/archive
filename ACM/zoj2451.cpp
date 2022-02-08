// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827365950
// 线段树
#include <iostream>

using namespace std;

constexpr int inf = 500010;
struct node {
    int from;
    int to;
    int pre;
};

node nodes[500010];

void init(int from, int to, int i = 1) {
    nodes[i] = node{from, to, from == 1 ? 0 : inf};
    if (from != to) {
        int mid = (from + to) / 2;
        init(from, mid, i * 2);
        init(mid + 1, to, i * 2 + 1);
    }
}

int min_pre(int from, int to, int i = 1) {
    if (nodes[i].from == from && nodes[i].to == to) {
        return nodes[i].pre;
    }

    int nodes_mid = (nodes[i].from + nodes[i].to) / 2;
    if (to <= nodes_mid) {
        return min_pre(from, to, i * 2);
    } else if (from >= nodes_mid + 1) {
        return min_pre(from, to, i * 2 + 1);
    } else {
        return min(min_pre(from, nodes_mid, i * 2), min_pre(nodes_mid + 1, to, i * 2 + 1));
    }
}

void update(int p, int value, int i = 1) {
    nodes[i].pre = min(nodes[i].pre, value);
    if (nodes[i].from == p && nodes[i].to == p) {
        return;
    }

    int nodes_mid = (nodes[i].from + nodes[i].to) / 2;
    if (p <= nodes_mid) {
        update(p, value, i * 2);
    } else {
        update(p, value, i * 2 + 1);
    }
}

int main() {
    int N, M;
    while (cin >> N >> M) {
        init(1, N);
        for (int k = 0; k < M; ++k) {
            int i, j;
            cin >> i >> j;
            update(j, min_pre(i, j - 1) + 1);
        }
        cout << min_pre(N, N) << endl;
    }
    return 0;
}
