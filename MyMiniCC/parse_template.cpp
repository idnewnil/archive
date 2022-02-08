#include "dot.h"
#include "parse_template.h"

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
    m_result = /*TO INFLATE: GRAMMAR_ENTRY*/;
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

/*TO INFLATE: DEFINE*/
