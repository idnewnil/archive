#include "scan.h"

#include <map>
#include <sstream>

using std::wistream;
using std::ios;
using std::map;
using std::wstring;
using std::wstringstream;

Scanner::Scanner(wistream &is) : m_is(is), m_lineno(0), m_colno(0) {
}

enum class State {
    START, DONE, IN_NUM, IN_ID, IN_NE, IS_SPECIAL_OP1,
    IS_DIVIDE_OP, IN_COMMENT, EXITING_COMMENT
};

inline TokenType map_char_to_token_type(char c) {
    switch (c) {
        case '+':
            return TokenType::ADD;
        case '-':
            return TokenType::SUB;
        case '*':
            return TokenType::MUL;
        case '/':
            return TokenType::DIV;
        case '<':
            return TokenType::LT;
        case '>':
            return TokenType::GT;
        case '=':
            return TokenType::ASSIGN;
        case ';':
            return TokenType::END_STATEMENT;
        case ',':
            return TokenType::DELIMITER;
        case '(':
            return TokenType::LEFT_PARENTHESE;
        case ')':
            return TokenType::RIGHT_PARENTHESE;
        case '[':
            return TokenType::LEFT_BRACKET;
        case ']':
            return TokenType::RIGHT_BRACKET;
        case '{':
            return TokenType::LEFT_BRACE;
        case '}':
            return TokenType::RIGHT_BRACE;
        default :
            return TokenType::UNKNOWN;
    }
}

inline TokenType find_id_type(const wstring &id) {
    static map<wstring, TokenType> reversed_words = {
            {L"else",   TokenType::ELSE},
            {L"if",     TokenType::IF},
            {L"int",    TokenType::INT},
            {L"return", TokenType::RETURN},
            {L"void",   TokenType::VOID},
            {L"while",  TokenType::WHILE}
    };
    auto mapped = reversed_words.find(id);
    if (mapped != reversed_words.end()) {
        return mapped->second;
    } else {
        return TokenType::ID;
    }
}

Token Scanner::next_token() {
    wstringstream ss;
    State state = State::START;
    Token token;
    while (state != State::DONE) {
        int c = get();
        // 状态转移
        switch (state) {
            case State::START:
                if (iswspace(c)) {
                } else if (iswdigit(c)) {
                    ss.put(c);
                    state = State::IN_NUM;
                } else if (iswalpha(c)) {
                    ss.put(c);
                    state = State::IN_ID;
                } else if (c == '<' || c == '>' || c == '=') {
                    ss.put(c);
                    state = State::IS_SPECIAL_OP1;
                    token.type = map_char_to_token_type(char(c));
                } else if (c == '!') {
                    ss.put(c);
                    state = State::IN_NE;
                } else if (c == '/') {
                    // 不清楚是除号还是注释，所以先推入流中
                    ss.put(c);
                    state = State::IS_DIVIDE_OP;
                    token.type = TokenType::DIV;
                } else if (c == WEOF) {
                    state = State::DONE;
                    token.type = TokenType::END;
                } else {
                    ss.put(c);
                    state = State::DONE;
                    token.type = map_char_to_token_type(char(c));
                }
                break;
            case State::IN_NUM:
                if (iswdigit(c)) {
                    ss.put(c);
                } else {
                    // 该字符不属于NUM一部分，不消耗字符
                    if (c != WEOF) {
                        unget();
                    }
                    state = State::DONE;
                    token.type = TokenType::NUM;
                }
                break;
            case State::IN_ID:
                if (iswalpha(c)) {
                    ss.put(c);
                } else {
                    // 该字符不属于ID一部分，不消耗字符
                    if (c != WEOF) {
                        unget();
                    }
                    state = State::DONE;
                    token.type = TokenType::ID;
                }
                break;
            case State::IN_NE:
                state = State::DONE;
                if (c == '=') {
                    ss.put(c);
                    token.type = TokenType::NE;
                } else {
                    // 出错，不消耗字符
                    if (c != WEOF) {
                        unget();
                    }
                    token.type = TokenType::UNKNOWN;
                }
                break;
            case State::IS_SPECIAL_OP1:
                state = State::DONE;
                if (c == '=') {
                    ss.put(c);
                    // 是等号则说明这是一个两个符号组成的运算符
                    switch (token.type) {
                        case TokenType::LT:
                            token.type = TokenType::LE;
                            break;
                        case TokenType::GT:
                            token.type = TokenType::GE;
                            break;
                        case TokenType::ASSIGN:
                            token.type = TokenType::EQ;
                            break;
                        default:
                            assert(false);
                            break;
                    }
                } else {
                    // 其他字符则不消耗
                    if (c != WEOF) {
                        unget();
                    }
                    state = State::DONE;
                }
                break;
            case State::IS_DIVIDE_OP:
                if (c == '*') {
                    // 是注释，将之前推入的/去掉
                    ss.seekp(-1, ios::cur);
                    state = State::IN_COMMENT;
                } else {
                    // 其他字符则不消耗
                    if (c != WEOF) {
                        unget();
                    }
                    state = State::DONE;
                }
                break;
            case State::IN_COMMENT:
                if (c == '*') {
                    state = State::EXITING_COMMENT;
                } else {
                }
                break;
            case State::EXITING_COMMENT:
                if (c == '*') {
                } else if (c == '/') {
                    state = State::START;
                } else {
                    state = State::IN_COMMENT;
                }
                break;
            default:
                assert(false);
                break;
        }
    }
    token.value = ss.str().substr(0, ss.tellp());
    token.count = count() - token.value.size();
    token.lineno = lineno();
    token.colno = colno() - token.value.size();
    if (token.type == TokenType::ID) {
        token.type = find_id_type(token.value);
    }
    return token;
}

size_t Scanner::count() const {
    return m_is.tellg();
}

size_t Scanner::lineno() const {
    return m_lineno;
}

size_t Scanner::colno() const {
    return m_colno;
}

int Scanner::get() {
    int ch = m_is.get();
    if (ch != WEOF) {
        // 记录统计信息
        ++m_colno;
        if (ch == '\n') {
            m_line_widths.emplace_back(m_colno);
            ++m_lineno;
            m_colno = 0;
        }
    }
    return ch;
}

void Scanner::unget() {
    m_is.unget();
    if (m_colno == 0) {
        --m_lineno;
        m_colno = m_line_widths.back();
        m_line_widths.pop_back();
    }
    --m_colno;
}
