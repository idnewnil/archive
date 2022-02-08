// https://zoj.pintia.cn/problem-sets/91827364500/problems/91827364716
// 解题思路：BFS
#include <iostream>
#include <map>
#include <queue>
#include <string>

using namespace std;

map<string, string> build(string const &goal) {
    char const ds[] = {'u', 'r', 'd', 'l'};
    map<string, string> ret;
    queue<string> que;
    ret[goal] = "";
    que.push(goal);
    while (!que.empty()) {
        string cur = que.front();
        que.pop();

        int cur_pos = cur.find('x');
        int cur_x = cur_pos % 3;
        int cur_y = cur_pos / 3;
        for (int i = 0; i < sizeof(ds); ++i) {
            int forward_x = cur_x;
            int forward_y = cur_y;
            if (ds[i] == 'u') {
                --forward_y;
            } else if (ds[i] == 'r') {
                ++forward_x;
            } else if (ds[i] == 'd') {
                ++forward_y;
            } else if (ds[i] == 'l') {
                --forward_x;
            } else;

            if (forward_x < 0 || forward_x >= 3 || forward_y < 0 || forward_y >= 3) {
                continue;
            }

            int forward_pos = forward_y * 3 + forward_x;
            string forward = cur;
            forward[forward_pos] ^= forward[cur_pos] ^= forward[forward_pos] ^= forward[cur_pos];
            if (ret.find(forward) == ret.end()) {
                ret[forward] = ds[(i + 2) % 4] + ret[cur];
                que.push(forward);
            }
        }
    }

    return ret;
}

int main() {
    map<string, string> res = build("12345678x");
    char buf[10] = {};
    while (cin >> buf[0]) {// 最开始用的是while(cin)，然后一直无法AC
        for (int i = 1; i < 9; ++i) {// 最开始用的是i=0..9
            cin >> buf[i];
        }
        string str(buf);
        auto find_res = res.find(str);
        if (find_res == res.end()) {
            cout << "unsolvable" << endl;
        } else {
            cout << find_res->second << endl;
        }
    }
    return 0;
}
