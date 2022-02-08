#ifndef PARSE_TEMPLATE_H
#define PARSE_TEMPLATE_H

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


/*TO INFLATE: DECLARE*/
};

#endif // PARSE_TEMPLATE_H
