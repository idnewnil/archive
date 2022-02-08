// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827366332
// Union-Find
#include <iostream>

using namespace std;

int arr[100500];

void reset(int size) {
    for (int i = 1; i <= size; ++i) {
        arr[i] = -1;
    }
}

int find(int x) {
    while (arr[x] > 0) {
        x = arr[x];
    }
    return x;
}

void merge(int x, int y) {
    int fx = find(x);
    int fy = find(y);
    arr[x] = arr[x] > 0 ? fx : arr[x]; // 非常重要，没有添加不能AC
    arr[y] = arr[y] > 0 ? fy : arr[y]; // 非常重要，没有添加不能AC
    if (fx != fy) {
        arr[fx] += arr[fy];
        arr[fy] = fx;
    }
}

int main() {
    int N, M, x, y;
    char op;
    int count = 1;
    while (scanf("%d%d", &N, &M) != EOF) {
        reset(N);
        if (count != 1) printf("\n");
        printf("Case %d:\n", count);
        count++;
        for (int i = 0; i < M; ++i) {
            getchar();
            scanf("%c%d", &op, &x);
            if (op == 'M') {
                scanf("%d", &y);
                merge(x, y);
            } else if (op == 'Q') {
                printf("%d\n", -arr[find(x)]);
            }
        }
    }
    return 0;
}
