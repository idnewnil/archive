#ifndef ACM_COMPARE_H
#define ACM_COMPARE_H

#include <iostream>
#include <fstream>
#include <sstream>

using std::string;

class fun {
public:
    fun(int main()) { this->main = main; }

    string exec(string const &input) const {
        // clear
        cin.clear();
        cout.clear();
        // 写入输入到_stdin文件
        FILE *_stdin = fopen("_stdin", "w");
        fwrite(input.c_str(), 1, input.length(), _stdin);
        fclose(_stdin);
        // 重定向stdin到_stdin文件，重定向stdout到_stdout文件
        freopen("_stdin", "r", stdin);
        freopen("_stdout", "w", stdout);
        // 执行函数
        main();
        // 还原，Windows使用CON可以重定向回控制台，Linux下为/dev/console
        freopen("CON", "r", stdin);
        freopen("CON", "w", stdout);
        // 读入输出
        std::ifstream _stdout("_stdout");
        std::istreambuf_iterator<char> begin(_stdout);
        std::istreambuf_iterator<char> end;
        string ret(begin, end);
        _stdout.close();
        return ret;
    }

private:
    int (*main)();
};

struct compare_result {
    string input;
    string my_result;
    string others_result;
};

void test(int function(), string generate_input()) {
    while (true) {
        string in = generate_input();
        string out = fun(function).exec(in);
    }
}

compare_result test(int my_function(), int others_function(), string generate_input()) {
    compare_result ret;
    do {
        ret.input = generate_input();
        ret.my_result = fun(my_function).exec(ret.input);
        ret.others_result = fun(others_function).exec(ret.input);
    } while (ret.my_result == ret.others_result);
    return ret;
}

#endif //ACM_COMPARE_H
