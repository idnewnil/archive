#ifndef PARSE_H
#define PARSE_H

#include "bnf.h"

enum class NodeType {
    // error
    ERROR,
    // epsilon
    EPSILON,
    // 中间树
    TEM,
    // token
    TOKEN,
    // special statement
    ARRAY_DECLARATION, FUN_DECLARATION,
    // special expression
    ARRAY, FUN,
    // empty
    EMPTY,
    // other
    OTHER
};

struct TreeNode {
    using Ptr = std::shared_ptr<TreeNode>;

    NodeType type;
    Token content;
    std::vector<Ptr> children;
    Ptr sibling;

    TreeNode();

    void set_type(NodeType node_type);
};

std::wstring to_dot(const TreeNode::Ptr &tree_node);

struct Error {
    Token token;
    std::vector<std::wstring> expected_tokens;
};

class Parser {
public:
    TreeNode::Ptr parse(Scanner &scanner);

    TreeNode::Ptr result() const;

    const std::vector<Error> &errors() const;

private:
    Scanner *m_scanner;
    std::map<std::wstring, size_t> m_counter;
    Token m_cur;
    TreeNode::Ptr m_result;
    std::vector<Error> m_errors;

    void record_error(std::vector<std::wstring> expected_tokens);

    Token match(TokenType token_type);


    TreeNode::Ptr additiveexpression();

    TreeNode::Ptr additiveexpressionEx();

    TreeNode::Ptr addop();

    TreeNode::Ptr arglist();

    TreeNode::Ptr arglistEx();

    TreeNode::Ptr args();

    TreeNode::Ptr compoundstmt();

    TreeNode::Ptr declaration();

    TreeNode::Ptr declarationEx();

    TreeNode::Ptr declarationlist();

    TreeNode::Ptr declarationlistEx();

    TreeNode::Ptr expression();

    TreeNode::Ptr expressionEx();

    TreeNode::Ptr expressionExEx();

    TreeNode::Ptr expressionI();

    TreeNode::Ptr expressionstmt();

    TreeNode::Ptr factor();

    TreeNode::Ptr factorEx();

    TreeNode::Ptr iterationstmt();

    TreeNode::Ptr localdeclarations();

    TreeNode::Ptr localdeclarationsEx();

    TreeNode::Ptr mulop();

    TreeNode::Ptr param();

    TreeNode::Ptr paramEx();

    TreeNode::Ptr paramlistEx();

    TreeNode::Ptr params();

    TreeNode::Ptr paramsEx();

    TreeNode::Ptr program();

    TreeNode::Ptr relop();

    TreeNode::Ptr returnstmt();

    TreeNode::Ptr returnstmtEx();

    TreeNode::Ptr selectionstmt();

    TreeNode::Ptr selectionstmtEx();

    TreeNode::Ptr simpleexpressionEx();

    TreeNode::Ptr statement();

    TreeNode::Ptr statementlist();

    TreeNode::Ptr statementlistEx();

    TreeNode::Ptr term();

    TreeNode::Ptr termEx();

    TreeNode::Ptr typespecifier();

    TreeNode::Ptr varEx();

    TreeNode::Ptr vardeclaration();

    TreeNode::Ptr vardeclarationEx();
};

#endif // PARSE_H
