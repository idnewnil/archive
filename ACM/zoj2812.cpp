// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827366311
// 解题思路：None
#include <iostream>
#include <cstring>

using namespace std;

int main() {
    char str[256];
    while (cin.getline(str, 256) && strcmp(str, "#") != 0) {
        int sum = 0;
        for (int i = 0, len = strlen(str); i < len; ++i) {
            sum += (i + 1) * (str[i] == ' ' ? 0 : str[i] - 'A' + 1);
        }
        cout << sum << endl;
    }
    return 0;
}
