// http://poj.org/problem?id=2503
// 解题思路：None
#include <cstring>
#include <cstdio>
#include <map>
#include <string>

using namespace std;

int main() {
    map<string, string> dict;
    char line[256];
    char value[128];
    char key[128];
    while (gets(line) && strlen(line)) {
        sscanf(line, "%s %s", value, key);
        dict[key] = value;
    }
    while (gets(key)) {
        map<string, string>::const_iterator res = dict.find(key);
        if (res != dict.end())
            printf("%s\n", res->second.c_str());
        else
            printf("eh\n");
    }
    return 0;
}