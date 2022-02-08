#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <cassert>
#include <vector>

enum class TokenType {
    // 终止符
    END,
    // 标识符
    ID,
    // 数字
    NUM,
    // 未知符号
    UNKNOWN,
    // 保留字
    ELSE = 1000, IF, RETURN, WHILE, INT = 1100, VOID,
    // 专用符号，注释符号/*和*/也是专用符号，但是不会作为Token返回
    ADD = 2100, SUB, MUL = 2110, DIV, LT = 2120, LE, GT, GE, EQ, NE, ASSIGN = 2130,
    END_STATEMENT = 2200, DELIMITER,
    LEFT_PARENTHESE = 2300, RIGHT_PARENTHESE, LEFT_BRACKET, RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE
};

inline bool is_reserved_word(TokenType token_type) {
    return int(token_type) / 1000 == 1;
}

inline bool is_type_specifier(TokenType token_type) {
    return int(token_type) / 100 == 11;
}

inline bool is_special_symbol(TokenType token_type) {
    return int(token_type) / 1000 == 2;
}

inline bool is_operator(TokenType token_type) {
    return int(token_type) / 100 == 21;
}

inline bool is_add_op(TokenType token_type) {
    return int(token_type) / 10 == 210;
}

inline bool is_mul_op(TokenType token_type) {
    return int(token_type) / 10 == 211;
}

inline bool is_rel_op(TokenType token_type) {
    return int(token_type) / 10 == 212;
}

inline const wchar_t *symbol_name(TokenType token_type) {
    switch (token_type) {
        case TokenType::END:
            return L"$";
        case TokenType::ID:
            return L"ID";
        case TokenType::NUM:
            return L"NUM";
        case TokenType::ELSE:
            return L"else";
        case TokenType::IF:
            return L"if";
        case TokenType::INT:
            return L"int";
        case TokenType::RETURN:
            return L"return";
        case TokenType::VOID:
            return L"void";
        case TokenType::WHILE:
            return L"while";
        case TokenType::ADD:
            return L"+";
        case TokenType::SUB:
            return L"-";
        case TokenType::MUL:
            return L"*";
        case TokenType::DIV:
            return L"/";
        case TokenType::LT:
            return L"<";
        case TokenType::LE:
            return L"<=";
        case TokenType::GT:
            return L">";
        case TokenType::GE:
            return L">=";
        case TokenType::EQ:
            return L"==";
        case TokenType::NE:
            return L"!=";
        case TokenType::ASSIGN:
            return L"=";
        case TokenType::END_STATEMENT:
            return L";";
        case TokenType::DELIMITER:
            return L",";
        case TokenType::LEFT_PARENTHESE:
            return L"(";
        case TokenType::RIGHT_PARENTHESE:
            return L")";
        case TokenType::LEFT_BRACKET:
            return L"[";
        case TokenType::RIGHT_BRACKET:
            return L"]";
        case TokenType::LEFT_BRACE:
            return L"{";
        case TokenType::RIGHT_BRACE:
            return L"}";
        default:
            assert(false);
            return L"???";
    }
}

inline const wchar_t *class_name(TokenType token_type) {
    switch (token_type) {
        case TokenType::ID:
            return L"ID";
        case TokenType::NUM:
            return L"NUM";
        case TokenType::INT:
        case TokenType::VOID:
            return L"TYPE";
        case TokenType::ADD:
        case TokenType::SUB:
        case TokenType::MUL:
        case TokenType::DIV:
        case TokenType::LT:
        case TokenType::LE:
        case TokenType::GT:
        case TokenType::GE:
        case TokenType::EQ:
        case TokenType::NE:
        case TokenType::ASSIGN:
            return L"OP";
        default:
            assert(false);
            return L"???";
    }
}

inline const wchar_t *english_name(TokenType token_type) {
    switch (token_type) {
        case TokenType::END:
            return L"END";
        case TokenType::ID:
            return L"ID";
        case TokenType::NUM:
            return L"NUM";
        case TokenType::UNKNOWN:
            return L"UNKNOWN";
        case TokenType::ELSE:
            return L"ELSE";
        case TokenType::IF:
            return L"IF";
        case TokenType::INT:
            return L"INT";
        case TokenType::RETURN:
            return L"RETURN";
        case TokenType::VOID:
            return L"VOID";
        case TokenType::WHILE:
            return L"WHILE";
        case TokenType::ADD:
            return L"ADD";
        case TokenType::SUB:
            return L"SUB";
        case TokenType::MUL:
            return L"MUL";
        case TokenType::DIV:
            return L"DIV";
        case TokenType::LT:
            return L"LT";
        case TokenType::LE:
            return L"LE";
        case TokenType::GT:
            return L"GT";
        case TokenType::GE:
            return L"GE";
        case TokenType::EQ:
            return L"EQ";
        case TokenType::NE:
            return L"NE";
        case TokenType::ASSIGN:
            return L"ASSIGN";
        case TokenType::END_STATEMENT:
            return L"END_STATEMENT";
        case TokenType::DELIMITER:
            return L"DELIMITER";
        case TokenType::LEFT_PARENTHESE:
            return L"LEFT_PARENTHESE";
        case TokenType::RIGHT_PARENTHESE:
            return L"RIGHT_PARENTHESE";
        case TokenType::LEFT_BRACKET:
            return L"LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET:
            return L"RIGHT_BRACKET";
        case TokenType::LEFT_BRACE:
            return L"LEFT_BRACE";
        case TokenType::RIGHT_BRACE:
            return L"RIGHT_BRACE";
        default:
            assert(false);
            return L"???";
    }
}

inline const wchar_t *chinese_name(TokenType token_type) {
    switch (token_type) {
        case TokenType::END:
            return L"已完成 ";
        case TokenType::ID:
            return L"标识符 ";
        case TokenType::NUM:
            return L"数字";
        case TokenType::UNKNOWN:
            return L"无法识别";
        case TokenType::ELSE:
            return L"否则";
        case TokenType::IF:
            return L"如果";
        case TokenType::INT:
            return L"整型";
        case TokenType::RETURN:
            return L"返回";
        case TokenType::VOID:
            return L"空 ";
        case TokenType::WHILE:
            return L"循环";
        case TokenType::ADD:
            return L"加号";
        case TokenType::SUB:
            return L"减号";
        case TokenType::MUL:
            return L"乘号";
        case TokenType::DIV:
            return L"除号";
        case TokenType::LT:
            return L"小于";
        case TokenType::LE:
            return L"小于等于";
        case TokenType::GT:
            return L"大于";
        case TokenType::GE:
            return L"大于等于";
        case TokenType::EQ:
            return L"等于";
        case TokenType::NE:
            return L"不等于 ";
        case TokenType::ASSIGN:
            return L"赋值 ";
        case TokenType::END_STATEMENT:
            return L"结束段 ";
        case TokenType::DELIMITER:
            return L"分隔符 ";
        case TokenType::LEFT_PARENTHESE:
            return L"左圆括号";
        case TokenType::RIGHT_PARENTHESE:
            return L"右圆括号";
        case TokenType::LEFT_BRACKET:
            return L"左方括号";
        case TokenType::RIGHT_BRACKET:
            return L"右方括号";
        case TokenType::LEFT_BRACE:
            return L"左花括号";
        case TokenType::RIGHT_BRACE:
            return L"右花括号";
        default:
            assert(false);
            return L"???";
    }
}

struct Token {
    TokenType type;
    std::wstring value;
    size_t count;
    size_t lineno;
    size_t colno;
};

class Scanner {
public:
    explicit Scanner(std::wistream &is);


    Token next_token();


    size_t count() const;

    size_t lineno() const;

    size_t colno() const;

private:
    std::wistream &m_is;
    size_t m_lineno;
    size_t m_colno;
    std::vector<size_t> m_line_widths;

    int get();
    void unget();
};

#endif // SCANNER_H
