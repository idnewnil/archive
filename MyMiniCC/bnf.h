#ifndef BNF_H
#define BNF_H

#include "scan.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace MiniC {
    inline std::initializer_list<std::wstring> get_bnf_strings() {
        static std::initializer_list<std::wstring> bnf_strings = {
                L"program -> declarationlist",
                L"declarationlist -> declarationlist declaration | declaration",
                L"declaration -> vardeclaration | fundeclaration",
                L"vardeclaration -> typespecifier ID; | typespecifier ID[NUM];",
                L"typespecifier -> int | void",
                L"fundeclaration -> typespecifier ID(params) compoundstmt",
                L"params -> paramlist | void",
                L"paramlist -> paramlist, param | param",
                L"param -> typespecifier ID | typespecifier ID[]",
                L"compoundstmt -> { localdeclarations statementlist }",
                L"localdeclarations -> localdeclarations vardeclaration | empty",
                L"statementlist -> statementlist statement | empty",
                L"statement -> expressionstmt | compoundstmt | selectionstmt | iterationstmt | returnstmt",
                L"expressionstmt -> expression; | ;",
                L"selectionstmt -> if (expression) statement | if (expression) statement else statement",
                L"iterationstmt -> while (expression) statement",
                L"returnstmt -> return ; | return expression;",
                L"expression -> var = expression | simpleexpression",
                L"var -> ID | ID[expression]",
                L"simpleexpression -> additiveexpression relop additiveexpression | additiveexpression",
                L"relop -> <= | < | > | >= | == | !=",
                L"additiveexpression -> additiveexpression addop term | term",
                L"addop -> + | -",
                L"term -> term mulop factor | factor",
                L"mulop -> * | /",
                L"factor -> (expression) | var | call | NUM",
                L"call -> ID(args)",
                L"args -> arglist | empty",
                L"arglist -> arglist, expression | expression"
        };
        return bnf_strings;
    }

    inline std::initializer_list<std::wstring> get_bnf_strings2() {
        static std::initializer_list<std::wstring> bnf_strings = {
                L"additiveexpression -> term additiveexpressionEx",
                L"additiveexpressionEx -> addop term additiveexpressionEx | empty",
                L"addop -> + | -",
                L"arglist -> expression arglistEx",
                L"arglistEx -> , expression arglistEx | empty",
                L"args -> arglist | empty",
                L"compoundstmt -> { localdeclarations statementlist }",
                L"declaration -> typespecifier ID declarationEx",
                L"declarationEx -> ( params ) compoundstmt | vardeclarationEx",
                L"declarationlist -> declaration declarationlistEx",
                L"declarationlistEx -> declaration declarationlistEx | empty",
                L"expression -> ( expression ) expressionI | ID expressionEx | NUM expressionI",
                L"expressionI -> termEx additiveexpressionEx simpleexpressionEx",
                L"expressionEx -> ( args ) expressionI | = expression | [ expression ] expressionExEx | expressionI",
                L"expressionExEx -> = expression | expressionI",
                L"expressionstmt -> ; | expression ;",
                L"factor -> ( expression ) | ID factorEx | NUM",
                L"factorEx -> ( args ) | varEx",
                L"iterationstmt -> while ( expression ) statement",
                L"localdeclarations -> localdeclarationsEx",
                L"localdeclarationsEx -> empty | vardeclaration localdeclarationsEx",
                L"mulop -> * | /",
                L"param -> typespecifier ID paramEx",
                L"paramEx -> [ ] | empty",
                L"paramlistEx -> , param paramlistEx | empty",
                L"params -> int ID paramEx paramlistEx | void paramsEx",
                L"paramsEx -> ID paramEx paramlistEx | empty",
                L"program -> declarationlist",
                L"relop -> != | < | <= | == | > | >=",
                L"returnstmt -> return returnstmtEx",
                L"returnstmtEx -> ; | expression ;",
                L"selectionstmt -> if ( expression ) statement selectionstmtEx",
                L"selectionstmtEx -> else statement | empty",
                L"simpleexpressionEx -> empty | relop additiveexpression",
                L"statement -> compoundstmt | expressionstmt | iterationstmt | returnstmt | selectionstmt",
                L"statementlist -> statementlistEx",
                L"statementlistEx -> empty | statement statementlistEx",
                L"term -> factor termEx",
                L"termEx -> empty | mulop factor termEx",
                L"typespecifier -> int | void",
                L"varEx -> [ expression ] | empty",
                L"vardeclaration -> typespecifier ID vardeclarationEx",
                L"vardeclarationEx -> ; | [ NUM ] ;"
        };
        return bnf_strings;
    }

    constexpr const wchar_t *grammar_entry = L"program";
    constexpr const wchar_t *epsilon = L"empty";
    constexpr const wchar_t *end = L"$";

    inline std::map<std::wstring, TokenType> get_new_types() {
        static std::map<std::wstring, TokenType> new_types = {
                {L"NUM", TokenType::NUM}
        };
        return new_types;
    }

    inline bool save_if(TokenType token_type) {
        return token_type == TokenType::ID || token_type == TokenType::NUM || token_type == TokenType::INT || token_type == TokenType::VOID ||
               is_type_specifier(token_type) || is_operator(token_type);
    }

    inline std::set<TokenType> get_keyword_to_save() {
        static std::set<TokenType> keyword_to_save = {
            TokenType::IF, TokenType::RETURN, TokenType::WHILE
        };
        return keyword_to_save;
    }
}

struct Code {
    std::wstring h_file;
    std::wstring cpp_file;
};

class Bnf {
public:
    using TokenName = std::wstring;
    using ProductionHead = TokenName;
    using ProductionBody = std::vector<TokenName>;
    using ProductionBodies = std::set<ProductionBody>;
    using Productions = std::map<ProductionHead, ProductionBodies>;
    using SaveIf = bool (*)(TokenType);

    Bnf(const std::initializer_list<std::wstring> &bnf_strings, TokenName grammar_entry, TokenName epsilon,
        TokenName end, const std::map<TokenName, TokenType> &new_types, SaveIf save_if);

    const TokenName &grammar_entry() const;

    const TokenName &epsilon() const;

    const TokenName &end() const;

    SaveIf save_if() const;

    const std::map<TokenName, TokenType> &terminal_types() const;

    const Productions &productions() const;

    const std::map<ProductionHead, std::set<TokenName>> &first() const;

    const std::map<ProductionHead, std::set<TokenName>> &follow() const;

    bool is_nonterminal(const TokenName &token_name) const;

    bool is_epsilon(const TokenName &token_name) const;

    bool is_terminal(const TokenName &token_name) const;

    TokenType terminal_type(const TokenName &token_name) const;

    std::set<TokenName> first(ProductionBody::const_iterator begin, ProductionBody::const_iterator end) const;

    std::set<TokenName> first(const ProductionBody &body) const;


    std::wstring to_string_first() const;

    std::wstring to_string_follow() const;

    std::wstring to_string_productions() const;

    std::wstring to_dot_invoke() const;

    std::wstring to_dot() const;


    std::set<std::pair<ProductionHead, ProductionHead>> calculate_possible_first() const;


    std::wstring to_dot_possible_first() const;


    // 返回三个字符串，分别是parse.h，parse.cpp和语法树类型的switch函数
    Code generate_code(std::set<TokenType> keyword_to_save) const;

protected:
    TokenName m_grammar_entry;
    TokenName m_epsilon;
    TokenName m_end;
    SaveIf m_save_if;
    std::map<TokenName, TokenType> m_terminal_types;
    Productions m_productions;
    std::map<ProductionHead, std::set<TokenName>> m_first;
    std::map<ProductionHead, std::set<TokenName>> m_follow;

    bool is_type(const TokenName &token_name, TokenType token_type, bool not_exist) const;

    void remove_redundant_epsilon(ProductionBody &body) const;


    void calculate_first();

    void calculate_follow();


    bool contains_nonterminal(const ProductionBody &body) const;

    bool is_epsilon_body(const ProductionBody &body) const;

    bool save_if0(const TokenName &token_name) const;

    bool need_save(const ProductionBody &body) const;

    size_t save_count(const ProductionBody &body) const;


    std::wstring enum_name(const TokenName &token_name) const;

    std::wstring to_string_conditions(const std::wstring &token_value_name, const std::wstring &token_type_name,
                                      const std::set<TokenName> &conditions) const;
};

class Ebnf : public Bnf {
public:
    Ebnf(const std::initializer_list<std::wstring> &entry, TokenName grammar_entry,
         TokenName epsilon, TokenName end, const std::map<TokenName, TokenType> &new_types,
         SaveIf save_if);

private:
    std::vector<std::pair<ProductionHead, ProductionBodies>> tem() const;

    void assign(std::vector<std::pair<ProductionHead, ProductionBodies>> tem_productions);

    void remove_left_recursion(std::vector<std::pair<ProductionHead, ProductionBodies>> &tem_productions) const;

    void remove_left_associativity(std::vector<std::pair<ProductionHead, ProductionBodies>> &tem_productions,
                                   std::set<TokenName> exist_names) const;

    ProductionBodies
    replace(const ProductionBody &body, size_t begin, size_t end, const ProductionBodies &to_inserts) const;

    ProductionBodies
    expand0(const ProductionBodies &bodies, const std::set<TokenName> &to_expands, const TokenName &head,
            std::set<TokenName> &tabu) const;

    ProductionBodies expand(const ProductionBodies &bodies, const TokenName &head) const;
};

namespace MiniC {
    inline const Bnf &get_bnf() {
        static Bnf bnf(get_bnf_strings(), grammar_entry, epsilon, end, get_new_types(), save_if);
        return bnf;
    }

    inline const Bnf &get_bnf2() {
        static Bnf bnf(get_bnf_strings2(), grammar_entry, epsilon, end, get_new_types(), save_if);
        return bnf;
    }

    inline const Ebnf &get_ebnf() {
        static Ebnf ebnf(get_bnf_strings(), grammar_entry, epsilon, end, get_new_types(), save_if);
        return ebnf;
    }
}

#endif // BNF_H
