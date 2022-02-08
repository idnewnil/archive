#include "dot.h"
#include "parse.h"

#include <sstream>

using namespace std;

TreeNode::TreeNode() : type(NodeType::TEM), content({TokenType::UNKNOWN, L""}) {
}

void TreeNode::set_type(NodeType node_type) {
    assert(type == NodeType::TEM);
    type = node_type;
}

// 返回一个树和它的sibling构成的链表的两端
pair<shared_ptr<Node>, shared_ptr<Node>>
to_dot(const shared_ptr<const Node> &parent, const shared_ptr<const TreeNode> &tree_node, DiGraph &graph,
       size_t &seed, bool draw_parent_head_edge = false) {
    static map<NodeType, wstring> xlabels = {
            {NodeType::ARRAY_DECLARATION, L"array"},
            {NodeType::FUN_DECLARATION,   L"fun"},
            {NodeType::ARRAY,             L"array"},
            {NodeType::FUN,               L"fun"}
    };
    assert(tree_node->type != NodeType::ERROR && tree_node->type != NodeType::TEM);
    wstring head_name;
    wstring shape;
    if (tree_node->type == NodeType::EPSILON || tree_node->type == NodeType::EMPTY) {
        head_name = L"\"\"";
        shape = L"point";
    } else if (MiniC::get_bnf2().save_if()(tree_node->content.type)) {
        auto xlabel = xlabels.find(tree_node->type);
        if (xlabel != xlabels.end() && tree_node->content.type == TokenType::ID) {
            head_name = wstring(L"<<font>") + class_name(tree_node->content.type) + L"<font color=\"green\"><sup>" +
                        xlabel->second + L"</sup></font><br/>(" + tree_node->content.value + L")</font>>";
        } else {
            head_name =
                    wstring(L"\"") + class_name(tree_node->content.type) + L"\\n(" + tree_node->content.value + L")\"";
        }
        shape = L"oval";
    } else {
        auto &s = tree_node->content.value;
        if (s.find(L"{}") == 0) {
            // 以 {} 开头
            head_name = wstring(L"<<font>{}<font color=\"sienna\"><sup>") + s.substr(2) + L"</sup></font></font>>";
        } else {
            head_name = wstring(L"\"") + tree_node->content.value + L"\"";
        }
        shape = L"box";
    }
    auto head = node(to_wstring(seed++), {
            {L"label", head_name},
            {L"shape", shape}
    });
    graph.add_field(head);
    if (draw_parent_head_edge) {
        graph.add_field(edge(parent->name(), head->name(), {{L"style", L"invis"}}));
    }

    shared_ptr<Node> last_child;
    for (auto &child: tree_node->children) {
        auto res = to_dot(head, child, graph, seed);
        graph.add_field(edge(head->name(), res.first->name(), {}));
        if (last_child != nullptr) {
            graph.add_field(edge(last_child->name(), res.first->name(), {
                    {L"style",      L"invis"},
                    {L"constraint", L"false"}
            }));
        }
        last_child = res.second;
    }

    if (tree_node->sibling != nullptr) {
        auto res = to_dot(parent, tree_node->sibling, graph, seed, parent != nullptr);
        graph.add_field(edge(head->name(), res.first->name(), {{L"constraint", L"false"}}));
        return make_pair(head, res.second);
    } else {
        return make_pair(head, head);
    }
}

wstring to_dot(const TreeNode::Ptr &tree_node) {
    DiGraph graph(L"tree", {}, {
            node(L"node", {
                    {L"fontname", L"\"Consolas,'Courier New',monospace\""}
            })
    });
    size_t seed = 0;
    to_dot(nullptr, tree_node, graph, seed);
    return graph.to_string();
}

TreeNode::Ptr Parser::parse(Scanner &scanner) {
    m_scanner = &scanner;
    m_counter.clear();
    m_cur = scanner.next_token();
    m_errors.clear();
    m_result = program();
    return m_errors.empty() ? m_result : nullptr;
}

TreeNode::Ptr Parser::result() const {
    return m_result;
}

const std::vector<Error> &Parser::errors() const {
    return m_errors;
}

void Parser::record_error(vector<wstring> expected_tokens) {
    m_errors.emplace_back(Error{m_cur, move(expected_tokens)});
}

Token Parser::match(TokenType token_type) {
    if (m_cur.type != token_type) {
        record_error({symbol_name(token_type)});
    }
    Token old = m_cur;
    m_cur = m_scanner->next_token();
    return old;
}

void store(const TreeNode::Ptr &ptr, const TreeNode::Ptr &child) {
    ptr->children.emplace_back(child);
}

void store(const TreeNode::Ptr &ptr, const vector<TreeNode::Ptr> &children) {
    for (auto &child: children) {
        store(ptr, child);
    }
}

// make token
Token mt(TokenType type, wstring value) {
    return Token{type, move(value), size_t(-1), size_t(-1), size_t(-1)};
}

// make token
Token mt(wstring value) {
    return mt(TokenType::UNKNOWN, move(value));
}

// make node
TreeNode::Ptr mn(Token token) {
    TreeNode::Ptr res = make_shared<TreeNode>();
    res->set_type(NodeType::TOKEN);
    res->content = move(token);
    return res;
}

void store(const TreeNode::Ptr &ptr, Token token) {
    ptr->children.emplace_back(mn(move(token)));
}

template<typename T, typename... Ts>
void store(const TreeNode::Ptr &ptr, T t, Ts... ts) {
    store(ptr, t);
    store(ptr, ts...);
}

void store_sibling(const TreeNode::Ptr &ptr, TreeNode::Ptr sibling) {
    if (sibling != nullptr && sibling->type != NodeType::EPSILON) {
        auto last_tree_node = ptr;
        while (last_tree_node->sibling != nullptr) {
            last_tree_node = last_tree_node->sibling;
        }
        last_tree_node->sibling = move(sibling);
    }
}

template<typename T, typename... Ts>
void store_sibling(const TreeNode::Ptr &ptr, T t, Ts... ts) {
    store_sibling(ptr, t);
    store_sibling(ptr, ts...);
}

TreeNode::Ptr set_bitree_first(const TreeNode::Ptr &ptr, TreeNode::Ptr first) {
    if (ptr->type == NodeType::EPSILON) {
        return first;
    } else if (first->type == NodeType::EPSILON) {
        return ptr;
    } else {
        auto left = ptr;
        while (left->children.size() == 2) {
            left = left->children[0];
        }
        assert(ptr->type == NodeType::ERROR || left->children.size() == 1);
        left->children.emplace(left->children.begin(), first);
        return ptr;
    }
}

// 断言树中没有数据
void assert_no_data(const TreeNode::Ptr &ptr) {
    assert(ptr->content.type == TokenType::UNKNOWN && ptr->content.value.empty() && ptr->children.empty() &&
           ptr->sibling == nullptr);
}

void check(const TreeNode::Ptr &ptr) {
    if (ptr->type == NodeType::EMPTY || ptr->type == NodeType::EPSILON) {
        assert_no_data(ptr);
    }
}

TreeNode::Ptr Parser::additiveexpression() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM) {
        auto call0 = term();
        auto call1 = additiveexpressionEx();
        res = set_bitree_first(call1, call0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::additiveexpressionEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::ADD || m_cur.type == TokenType::SUB) {
        auto match0 = addop()->content;
        auto call0 = term();
        auto call1 = additiveexpressionEx();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
        res = set_bitree_first(call1, res);
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::RIGHT_PARENTHESE ||
               m_cur.type == TokenType::DELIMITER || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::LT || m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ ||
               m_cur.type == TokenType::GT || m_cur.type == TokenType::GE || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L"+", L",", L"-", L";", L"<", L"<=", L"==", L">", L">=", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::addop() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::ADD) {
        auto match0 = match(TokenType::ADD);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::SUB) {
        auto match0 = match(TokenType::SUB);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"+", L"-"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::arglist() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM) {
        auto call0 = expression();
        auto call1 = arglistEx();
        res = call0;
        store_sibling(res, call1);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::arglistEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::DELIMITER) {
        match(TokenType::DELIMITER);
        auto call0 = expression();
        auto call1 = arglistEx();
        res = call0;
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L")", L","});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::args() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM) {
        auto call0 = arglist();
        res = call0;
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L")", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::compoundstmt() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_BRACE) {
        size_t count = m_counter[L"compoundstmt"]++;
        match(TokenType::LEFT_BRACE);
        auto call0 = localdeclarations();
        auto call1 = statementlist();
        match(TokenType::RIGHT_BRACE);
        res->set_type(NodeType::OTHER);
        res->content = mt(L"{}" + to_wstring(count));
        store(res, call0, call1);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"{"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::declaration() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto match0 = typespecifier()->content;
        auto match1 = match(TokenType::ID);
        auto call0 = declarationEx();
        res->set_type(call0->type);
        res->content = move(match1);
        store(res, match0);
        if (res->type == NodeType::FUN_DECLARATION) {
            store(res, call0->children);
        } else if (res->type != NodeType::OTHER) {
            store(res, call0);
        } else {
            assert_no_data(call0);
        }
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::declarationEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE) {
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = params();
        match(TokenType::RIGHT_PARENTHESE);
        auto call1 = compoundstmt();
        res->set_type(NodeType::FUN_DECLARATION);
        store(res, call0, call1);
    } else if (m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::LEFT_BRACKET) {
        auto call0 = vardeclarationEx();
        if (call0->type == NodeType::EMPTY) {
            res->set_type(NodeType::OTHER);
        } else {
            res = call0;
        }
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"["});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::declarationlist() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto call0 = declaration();
        auto call1 = declarationlistEx();
        res = call0;
        store_sibling(res, call1);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::declarationlistEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto call0 = declaration();
        auto call1 = declarationlistEx();
        res = call0;
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::END) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"$", L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::expression() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE) {
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = expression();
        match(TokenType::RIGHT_PARENTHESE);
        auto call1 = expressionI();
        res = set_bitree_first(call1, call0);
    } else if (m_cur.type == TokenType::ID) {
        auto match0 = match(TokenType::ID);
        auto call0 = expressionEx();
        if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match0);
        if (res->type == NodeType::ARRAY || res->type == NodeType::FUN) {
            store(res, call0->children[0]);
            res = set_bitree_first(call0->children[1], res);
        } else {
            res = set_bitree_first(call0, res);
        }
    } else if (m_cur.type == TokenType::NUM) {
        auto match0 = match(TokenType::NUM);
        auto call0 = expressionI();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        res = set_bitree_first(call0, res);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::expressionEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE) {
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = args();
        match(TokenType::RIGHT_PARENTHESE);
        auto call1 = expressionI();
        res->set_type(NodeType::FUN);
        store(res, call0, call1);
    } else if (m_cur.type == TokenType::ASSIGN) {
        auto match0 = match(TokenType::ASSIGN);
        auto call0 = expression();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
    } else if (m_cur.type == TokenType::LEFT_BRACKET) {
        match(TokenType::LEFT_BRACKET);
        auto call0 = expression();
        match(TokenType::RIGHT_BRACKET);
        auto call1 = expressionExEx();
        res->set_type(NodeType::ARRAY);
        store(res, call0, call1);
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::MUL || m_cur.type == TokenType::ADD ||
               m_cur.type == TokenType::SUB || m_cur.type == TokenType::DIV || m_cur.type == TokenType::LT ||
               m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ || m_cur.type == TokenType::GT ||
               m_cur.type == TokenType::GE) {
        auto call0 = expressionI();
        res = call0;
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error(
                {L"!=", L"(", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"=", L"==", L">", L">=", L"[",
                 L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::expressionExEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::ASSIGN) {
        auto match0 = match(TokenType::ASSIGN);
        auto call0 = expression();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::MUL || m_cur.type == TokenType::ADD ||
               m_cur.type == TokenType::SUB || m_cur.type == TokenType::DIV || m_cur.type == TokenType::LT ||
               m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ || m_cur.type == TokenType::GT ||
               m_cur.type == TokenType::GE) {
        auto call0 = expressionI();
        res = call0;
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"=", L"==", L">", L">=", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::expressionI() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::NE || m_cur.type == TokenType::MUL || m_cur.type == TokenType::ADD ||
        m_cur.type == TokenType::SUB || m_cur.type == TokenType::DIV || m_cur.type == TokenType::LT ||
        m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ || m_cur.type == TokenType::GT ||
        m_cur.type == TokenType::GE) {
        auto call0 = termEx();
        auto call1 = additiveexpressionEx();
        auto call2 = simpleexpressionEx();
        res = set_bitree_first(call2, set_bitree_first(call1, call0));
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"==", L">", L">=", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::expressionstmt() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::END_STATEMENT) {
        match(TokenType::END_STATEMENT);
        res->set_type(NodeType::EMPTY);
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID ||
               m_cur.type == TokenType::NUM) {
        auto call0 = expression();
        match(TokenType::END_STATEMENT);
        res = call0;
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::factor() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE) {
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = expression();
        match(TokenType::RIGHT_PARENTHESE);
        res = call0;
    } else if (m_cur.type == TokenType::ID) {
        auto match0 = match(TokenType::ID);
        auto call0 = factorEx();
        if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match0);
        if (res->type != NodeType::OTHER) {
            if (res->type == NodeType::FUN || res->type == NodeType::ARRAY) {
                call0 = call0->children[0];
            }
            store(res, call0);
        } else {
            assert_no_data(call0);
        }
    } else if (m_cur.type == TokenType::NUM) {
        auto match0 = match(TokenType::NUM);
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::factorEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE) {
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = args();
        match(TokenType::RIGHT_PARENTHESE);
        res->set_type(NodeType::FUN);
        store(res, call0);
    } else if (m_cur.type == TokenType::LEFT_BRACKET) {
        auto call0 = varEx();
        if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res = call0;
        }
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::RIGHT_PARENTHESE ||
               m_cur.type == TokenType::MUL || m_cur.type == TokenType::ADD || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::SUB || m_cur.type == TokenType::DIV || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::LT || m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ ||
               m_cur.type == TokenType::GT || m_cur.type == TokenType::GE || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error(
                {L"!=", L"(", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"==", L">", L">=", L"[", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::iterationstmt() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::WHILE) {
        auto match0 = match(TokenType::WHILE);
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = expression();
        match(TokenType::RIGHT_PARENTHESE);
        auto call1 = statement();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0, call1);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"while"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::localdeclarations() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto call0 = localdeclarationsEx();
        res = call0;
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM || m_cur.type == TokenType::IF ||
               m_cur.type == TokenType::RETURN || m_cur.type == TokenType::WHILE ||
               m_cur.type == TokenType::LEFT_BRACE || m_cur.type == TokenType::RIGHT_BRACE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"if", L"int", L"return", L"void", L"while", L"{", L"}"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::localdeclarationsEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto call0 = vardeclaration();
        auto call1 = localdeclarationsEx();
        res = call0;
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM || m_cur.type == TokenType::IF ||
               m_cur.type == TokenType::RETURN || m_cur.type == TokenType::WHILE ||
               m_cur.type == TokenType::LEFT_BRACE || m_cur.type == TokenType::RIGHT_BRACE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"if", L"int", L"return", L"void", L"while", L"{", L"}"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::mulop() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::MUL) {
        auto match0 = match(TokenType::MUL);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::DIV) {
        auto match0 = match(TokenType::DIV);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"*", L"/"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::param() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto match0 = typespecifier()->content;
        auto match1 = match(TokenType::ID);
        auto call0 = paramEx();
        if (call0->type == NodeType::EMPTY) {
            res->set_type(NodeType::ARRAY_DECLARATION);
        } else if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match1);
        store(res, match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::paramEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_BRACKET) {
        match(TokenType::LEFT_BRACKET);
        match(TokenType::RIGHT_BRACKET);
        res->set_type(NodeType::EMPTY);
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE || m_cur.type == TokenType::DELIMITER) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L")", L",", L"["});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::paramlistEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::DELIMITER) {
        match(TokenType::DELIMITER);
        auto call0 = param();
        auto call1 = paramlistEx();
        res = call0;
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L")", L","});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::params() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT) {
        auto match0 = match(TokenType::INT);
        auto match1 = match(TokenType::ID);
        auto call0 = paramEx();
        auto call1 = paramlistEx();
        if (call0->type == NodeType::EMPTY) {
            res->set_type(NodeType::ARRAY_DECLARATION);
        } else if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match1);
        store(res, match0);
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::VOID) {
        auto match0 = match(TokenType::VOID);
        auto call0 = paramsEx();
        if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::EMPTY);
        } else {
            res->set_type(NodeType::OTHER);
            res->content = move(match0);
            call0->children.insert(call0->children.begin(), res);
            res = call0;
        }
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::paramsEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::ID) {
        auto match0 = match(TokenType::ID);
        auto call0 = paramEx();
        auto call1 = paramlistEx();
        if (call0->type == NodeType::EMPTY) {
            res->set_type(NodeType::ARRAY_DECLARATION);
        } else if (call0->type == NodeType::EPSILON) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match0);
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L")", L"ID"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::program() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto call0 = declarationlist();
        res = call0;
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::relop() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::NE) {
        auto match0 = match(TokenType::NE);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::LT) {
        auto match0 = match(TokenType::LT);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::LE) {
        auto match0 = match(TokenType::LE);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::EQ) {
        auto match0 = match(TokenType::EQ);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::GT) {
        auto match0 = match(TokenType::GT);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::GE) {
        auto match0 = match(TokenType::GE);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L"<", L"<=", L"==", L">", L">="});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::returnstmt() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::RETURN) {
        auto match0 = match(TokenType::RETURN);
        auto call0 = returnstmtEx();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"return"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::returnstmtEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::END_STATEMENT) {
        match(TokenType::END_STATEMENT);
        res->set_type(NodeType::EMPTY);
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID ||
               m_cur.type == TokenType::NUM) {
        auto call0 = expression();
        match(TokenType::END_STATEMENT);
        res = call0;
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::selectionstmt() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::IF) {
        auto match0 = match(TokenType::IF);
        match(TokenType::LEFT_PARENTHESE);
        auto call0 = expression();
        match(TokenType::RIGHT_PARENTHESE);
        auto call1 = statement();
        auto call2 = selectionstmtEx();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0, call1, call2);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"if"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::selectionstmtEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::ELSE) {
        match(TokenType::ELSE);
        auto call0 = statement();
        res = call0;
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM || m_cur.type == TokenType::ELSE ||
               m_cur.type == TokenType::IF || m_cur.type == TokenType::RETURN || m_cur.type == TokenType::WHILE ||
               m_cur.type == TokenType::LEFT_BRACE || m_cur.type == TokenType::RIGHT_BRACE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"else", L"if", L"return", L"while", L"{", L"}"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::simpleexpressionEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::NE || m_cur.type == TokenType::LT || m_cur.type == TokenType::LE ||
        m_cur.type == TokenType::EQ || m_cur.type == TokenType::GT || m_cur.type == TokenType::GE) {
        auto match0 = relop()->content;
        auto call0 = additiveexpression();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
    } else if (m_cur.type == TokenType::RIGHT_PARENTHESE || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L",", L";", L"<", L"<=", L"==", L">", L">=", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::statement() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_BRACE) {
        auto call0 = compoundstmt();
        res = call0;
    } else if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM) {
        auto call0 = expressionstmt();
        res = call0;
    } else if (m_cur.type == TokenType::WHILE) {
        auto call0 = iterationstmt();
        res = call0;
    } else if (m_cur.type == TokenType::RETURN) {
        auto call0 = returnstmt();
        res = call0;
    } else if (m_cur.type == TokenType::IF) {
        auto call0 = selectionstmt();
        res = call0;
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"if", L"return", L"while", L"{"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::statementlist() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
        m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM || m_cur.type == TokenType::IF ||
        m_cur.type == TokenType::RETURN || m_cur.type == TokenType::WHILE || m_cur.type == TokenType::LEFT_BRACE) {
        auto call0 = statementlistEx();
        res = call0;
    } else if (m_cur.type == TokenType::RIGHT_BRACE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"if", L"return", L"while", L"{", L"}"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::statementlistEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::END_STATEMENT ||
        m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM || m_cur.type == TokenType::IF ||
        m_cur.type == TokenType::RETURN || m_cur.type == TokenType::WHILE || m_cur.type == TokenType::LEFT_BRACE) {
        auto call0 = statement();
        auto call1 = statementlistEx();
        res = call0;
        store_sibling(res, call1);
    } else if (m_cur.type == TokenType::RIGHT_BRACE) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L";", L"ID", L"NUM", L"if", L"return", L"while", L"{", L"}"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::term() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_PARENTHESE || m_cur.type == TokenType::ID || m_cur.type == TokenType::NUM) {
        auto call0 = factor();
        auto call1 = termEx();
        res = set_bitree_first(call1, call0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"(", L"ID", L"NUM"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::termEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::MUL || m_cur.type == TokenType::DIV) {
        auto match0 = mulop()->content;
        auto call0 = factor();
        auto call1 = termEx();
        res->set_type(NodeType::OTHER);
        res->content = move(match0);
        store(res, call0);
        res = set_bitree_first(call1, res);
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::RIGHT_PARENTHESE ||
               m_cur.type == TokenType::ADD || m_cur.type == TokenType::DELIMITER || m_cur.type == TokenType::SUB ||
               m_cur.type == TokenType::END_STATEMENT || m_cur.type == TokenType::LT || m_cur.type == TokenType::LE ||
               m_cur.type == TokenType::EQ || m_cur.type == TokenType::GT || m_cur.type == TokenType::GE ||
               m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"==", L">", L">=", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::typespecifier() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT) {
        auto match0 = match(TokenType::INT);
        res->content = move(match0);
    } else if (m_cur.type == TokenType::VOID) {
        auto match0 = match(TokenType::VOID);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::varEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::LEFT_BRACKET) {
        match(TokenType::LEFT_BRACKET);
        auto call0 = expression();
        match(TokenType::RIGHT_BRACKET);
        res->set_type(NodeType::ARRAY);
        store(res, call0);
    } else if (m_cur.type == TokenType::NE || m_cur.type == TokenType::RIGHT_PARENTHESE ||
               m_cur.type == TokenType::MUL || m_cur.type == TokenType::ADD || m_cur.type == TokenType::DELIMITER ||
               m_cur.type == TokenType::SUB || m_cur.type == TokenType::DIV || m_cur.type == TokenType::END_STATEMENT ||
               m_cur.type == TokenType::LT || m_cur.type == TokenType::LE || m_cur.type == TokenType::EQ ||
               m_cur.type == TokenType::GT || m_cur.type == TokenType::GE || m_cur.type == TokenType::RIGHT_BRACKET) {
        res->set_type(NodeType::EPSILON);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"!=", L")", L"*", L"+", L",", L"-", L"/", L";", L"<", L"<=", L"==", L">", L">=", L"[", L"]"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::vardeclaration() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::INT || m_cur.type == TokenType::VOID) {
        auto match0 = typespecifier()->content;
        auto match1 = match(TokenType::ID);
        auto call0 = vardeclarationEx();
        if (call0->type == NodeType::EMPTY) {
            res->set_type(NodeType::OTHER);
        } else {
            res->set_type(call0->type);
        }
        res->content = move(match1);
        store(res, match0);
        if (res->type != NodeType::OTHER) {
            store(res, call0);
        } else {
            assert_no_data(call0);
        }
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L"int", L"void"});
    }
    check(res);
    return res;
}

TreeNode::Ptr Parser::vardeclarationEx() {
    TreeNode::Ptr res = make_shared<TreeNode>();
    if (m_cur.type == TokenType::END_STATEMENT) {
        match(TokenType::END_STATEMENT);
        res->set_type(NodeType::EMPTY);
    } else if (m_cur.type == TokenType::LEFT_BRACKET) {
        match(TokenType::LEFT_BRACKET);
        auto match0 = match(TokenType::NUM);
        match(TokenType::RIGHT_BRACKET);
        match(TokenType::END_STATEMENT);
        res->set_type(NodeType::ARRAY_DECLARATION);
        res->content = move(match0);
    } else {
        res->set_type(NodeType::ERROR);
        record_error({L";", L"["});
    }
    check(res);
    return res;
}
