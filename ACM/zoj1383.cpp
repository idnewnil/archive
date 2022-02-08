// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364882
// 解题思路：None
#include <iostream>

using namespace std;

int main() {
    int d;
    cin >> d;
    for (int i = 0; i < d; ++i) {
        unsigned num;
        cin >> num;
        bool first = true;
        int pos = 0;
        while (num) {
            if (num & 1) {
                if (first)
                    first = false;
                else cout << ' ';
                cout << pos;
            }
            num >>= 1;
            ++pos;
        }
        cout << endl;
    }
    return 0;
}
