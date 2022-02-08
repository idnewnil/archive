#include "symtab.h"

#include <string>
#include <utility>
#include <sstream>
#include <iomanip>

using namespace std;

Location loc_from_token(const Token &token) {
    return {token.count, token.lineno, token.colno};
}

void SymtabItem::set_specifier(TokenType token_type) {
    assert(is_type_specifier(token_type));
    this->specifier = token_type;
}

const wstring &Symtab::name() const {
    return m_name;
}

ScopeType Symtab::type() const {
    return is_compoundstmt() ? ScopeType::STMT : name().empty() ? ScopeType::GLOBAL : ScopeType::FUN;
}

pair<int, ScopeType> Symtab::lookup(const wstring &name) const {
    auto entry = find(name);
    if (entry != m_data.end()) {
        size_t count = entry->second->address;
        auto fun = m_parent;
        while (fun != nullptr && fun->is_compoundstmt()) {
            count += fun->m_size;
            fun = fun->m_parent;
        }
        // 全局变量和栈区的增长方向是相反的，因此需要修改数组的首地址
        if (m_name.empty() && entry->second->type == ItemType::ARRAY) {
            // 是全局区，改变首地址
            count += entry->second->size - 1;
        }
        return make_pair(int(count), type());
    } else {
        if (m_parent == nullptr) {
            return make_pair(-1, ScopeType::NONE);
        } else {
            return m_parent->lookup(name);
        }
    }
}

vector<pair<wstring, SymtabItem::Ptr>>::const_iterator Symtab::find(const wstring &name) const {
    return find_if(m_data.begin(), m_data.end(), [&name](const pair<wstring, SymtabItem::Ptr> &entry) {
        return entry.first == name;
    });
}

SymtabItem::Ptr Symtab::new0(const Token &token) {
    auto entry = find(token.value);
    if (entry != m_data.end()) {
        // 错误，声明已存在的变量
        m_errors->emplace_back(SymtabError{SymtabErrorType::MULTI_DECLARATION, token, loc_from_token(token)});
        return entry->second;
    } else {
        m_data.emplace_back(token.value, make_shared<SymtabItem>());
        m_data.back().second->def = loc_from_token(token);
        return m_data.back().second;
    }
}

SymtabItem::Ptr Symtab::lookup0(const wstring &name) const {
    auto entry = find(name);
    if (entry != m_data.end()) {
        return entry->second;
    } else {
        if (m_parent == nullptr) {
            return nullptr;
        } else {
            return m_parent->lookup0(name);
        }
    }
}

TokenType Symtab::check_type(const TreeNode::Ptr &tree_node, TokenType specifier) const {
    assert(is_type_specifier(specifier) || specifier == TokenType::UNKNOWN);
    switch (tree_node->content.type) {
        case TokenType::ID: {
            auto res = lookup0(tree_node->content.value);
            if (res == nullptr) {
                // 错误，没有找到声明
                m_errors->emplace_back(SymtabError{SymtabErrorType::NOT_FOUND, tree_node->content});
                return TokenType::UNKNOWN;
            }
            if (tree_node->type == NodeType::FUN && res->type != ItemType::FUN ||
                tree_node->type == NodeType::ARRAY && res->type != ItemType::ARRAY ||
                tree_node->type == NodeType::OTHER && res->type != ItemType::VAR) {
                // 错误，不正确使用变量
                m_errors->emplace_back(SymtabError{SymtabErrorType::INCORRECT_USE, tree_node->content, res->def});
            } else {
                if (tree_node->type == NodeType::FUN) {
                    size_t param_count = 0;
                    auto param = tree_node->children[0];
                    if (param->type != NodeType::EMPTY && param->type != NodeType::EPSILON) {
                        for (; param != nullptr; param = param->sibling, ++param_count) {
                            if (param_count < res->params.size()) {
                                if (res->params[param_count].is_array) {
                                    if (param->type != NodeType::OTHER/*param的值为OTHER才说明可能是传入数组*/) {
                                        // 函数参数应该是数组
                                        m_errors->emplace_back(
                                                SymtabError{SymtabErrorType::REQUIRE_ARRAY, param->content, res->def});
                                    }
                                    auto param_res = lookup0(param->content.value);
                                    if (param_res == nullptr) {
                                        // 错误，没有找到声明
                                        m_errors->emplace_back(SymtabError{SymtabErrorType::NOT_FOUND, param->content});
                                        return TokenType::UNKNOWN;
                                    } else if (param_res->specifier != res->params[param_count].type) {
                                        // 错误，类型不匹配
                                        m_errors->emplace_back(
                                                SymtabError{SymtabErrorType::TYPE_ERROR, param->content, res->def});
                                    }
                                } else {
                                    if (check_type(param, TokenType::UNKNOWN) != res->params[param_count].type) {
                                        // 错误，类型不匹配
                                        m_errors->emplace_back(
                                                SymtabError{SymtabErrorType::TYPE_ERROR, param->content, res->def});
                                    }
                                }
                            }
                        }
                    }
                    if (param_count != res->params.size()) {
                        // 参数个数不匹配
                        m_errors->emplace_back(
                                SymtabError{SymtabErrorType::WRONG_PARAM_SIZE, tree_node->content, res->def});
                    }
                } else if (tree_node->type == NodeType::ARRAY) {
                    if (check_type(tree_node->children[0], TokenType::UNKNOWN) != TokenType::INT) {
                        m_errors->emplace_back(
                                SymtabError{SymtabErrorType::ERROR_EXP_TYPE, tree_node->children[0]->content});
                    }
                }
                // 如果是UNKNOWN则不处理
                if (specifier != TokenType::UNKNOWN && res->specifier != specifier) {
                    // 错误，和需求的类型不一致
                    m_errors->emplace_back(
                            SymtabError{SymtabErrorType::ERROR_EXP_TYPE, tree_node->content});
                }
            }
            return res->specifier;
        }
        case TokenType::NUM:
            if (specifier != TokenType::UNKNOWN && specifier != TokenType::INT) {
                // 错误，和需求的类型不一致
                m_errors->emplace_back(
                        SymtabError{SymtabErrorType::ERROR_EXP_TYPE, tree_node->content});
            }
            return TokenType::INT;
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
        case TokenType::ASSIGN: {
            assert(specifier == TokenType::UNKNOWN || specifier == TokenType::INT);
            auto a = check_type(tree_node->children[0], TokenType::UNKNOWN);
            auto b = check_type(tree_node->children[1], TokenType::UNKNOWN);
            if (a != TokenType::INT) {
                m_errors->emplace_back(SymtabError{SymtabErrorType::ERROR_EXP_TYPE, tree_node->children[0]->content});
            }
            if (b != TokenType::INT) {
                m_errors->emplace_back(SymtabError{SymtabErrorType::ERROR_EXP_TYPE, tree_node->children[1]->content});
            }
            return TokenType::INT;
        }
        default:
            assert(false);
            return TokenType::UNKNOWN;
    }
}

Symtab::Symtab() : m_size(0) {
}

Symtab::Symtab(wstring name, Location def, std::shared_ptr<std::vector<SymtabError>> errors)
        : m_name(move(name)), m_def(def), m_errors(move(errors)), m_size(0) {
}

template<typename T1, typename T2>
wstring join(const wstring &ws, vector<T1> v, T2 f) {
    wstringstream wss;
    wss << L"(";
    for (auto param = v.begin(); param != v.end(); ++param) {
        if (param != v.begin()) {
            wss << ws;
        }
        wss << f(*param);
    }
    wss << L")";
    return wss.str();
}

bool Symtab::is_compoundstmt() const {
    return name().find(L"{}") == 0;
}

Symtab::Ptr Symtab::in_which_fun() const {
    if (m_parent == nullptr) {
        return nullptr;
    } else {
        auto fun = m_parent;
        while (fun->is_compoundstmt()) {
            fun = fun->m_parent;
        }
        return fun;
    }
}

wstring Symtab::to_string() const {
    vector<vector<wstring>> table;
    table.emplace_back(vector<wstring>{
            L"name", L"type", L"specifier", L"params", L"size", L"address", L"lookup_address", L"def", L"refs"
    });
    for (auto &entry: m_data) {
        vector<wstring> raw;
        raw.emplace_back(entry.first);
        raw.emplace_back(item_type_name(entry.second->type));
        raw.emplace_back(english_name(entry.second->specifier));
        raw.emplace_back(
                entry.second->type == ItemType::FUN ? join(L", ", entry.second->params, [](const ParamDesc &pd) {
                    return wstring(symbol_name(pd.type)) + (pd.is_array ? L"[]" : L"");
                }) : L"");
        raw.emplace_back(entry.second->type == ItemType::ARRAY ? to_wstring(entry.second->size) : L"");
        raw.emplace_back(to_wstring(entry.second->address));
        raw.emplace_back(to_wstring(lookup(entry.first).first));
        raw.emplace_back(
                L"(" + to_wstring(entry.second->def.lineno) + L"," + to_wstring(entry.second->def.colno) + L")");
        raw.emplace_back(entry.second->refs.empty() ? L"" : join(L" ", entry.second->refs, [](const Location &loc) {
            return L"(" + to_wstring(loc.lineno) + L"," + to_wstring(loc.colno) + L")";
        }));
        table.emplace_back(raw);
    }
    // 记录最大宽度
    vector<size_t> widths;
    widths.resize(table[0].size());
    for (auto &raw:table) {
        for (int i = 0; i < raw.size(); ++i) {
            if (widths[i] < raw[i].size()) {
                widths[i] = raw[i].size();
            }
        }
    }
    wstringstream wss;
    for (auto &raw:table) {
        for (int i = 0; i < raw.size(); ++i) {
            wss << setiosflags(ios::left) << setw(widths[i] + 2) << raw[i];
        }
        wss << endl;
    }
    wstring ws = wss.str();
    return ws.substr(0, ws.size() - 1);
}

ScopeSymtabs::ScopeSymtabs(const TreeNode::Ptr &tree_node) : m_errors(make_shared<vector<SymtabError>>()) {
    Token fake{TokenType::UNKNOWN, L"", size_t(-1), size_t(-1), size_t(-1)};
    auto global = new0(fake);

    // input函数
    auto input = global->new0(Token{TokenType::ID, L"input", size_t(-1), size_t(-1), size_t(-1)});
    input->type = ItemType::FUN;
    input->specifier = TokenType::INT;
    input->size = NO_SIZE;
    input->address = -1;

    // output函数
    auto output = global->new0(Token{TokenType::ID, L"output", size_t(-1), size_t(-1), size_t(-1)});
    output->type = ItemType::FUN;
    output->specifier = TokenType::VOID;
    output->size = NO_SIZE;
    output->params = {{TokenType::INT, false}};
    output->address = -1;

    build(global/*全局范围的符号表*/, tree_node);

    auto it = m_symtabs.front()->lookup0(L"main");
    if (it == nullptr || it->type != ItemType::FUN) {
        // 没有main函数
        m_errors->emplace_back(SymtabError{SymtabErrorType::NO_MAIN});
    }
}

const vector<Symtab::Ptr> &ScopeSymtabs::symtabs() const {
    return m_symtabs;
}

const std::shared_ptr<std::vector<SymtabError>> &ScopeSymtabs::errors() const {
    return m_errors;
}

std::wstring ScopeSymtabs::to_string() const {
    wstringstream wss;
    for (auto &it: m_symtabs) {
        wstring parent_name = it->m_parent != nullptr ? it->m_parent->name() : L"";
        wss << L"Scope: \"" << it->name() << L"\"; Parent: ";
        if (it->m_parent != nullptr) {
            wss << L"\"" << parent_name << L"\"";
        }
        wss << L";" << endl;
        wss << it->to_string() << endl << endl;
    }
    wstring ws = wss.str();
    return ws.substr(0, ws.size() - 2);
}

std::vector<Symtab::Ptr>::const_iterator ScopeSymtabs::find(const wstring &name) const {
    return find_if(m_symtabs.begin(), m_symtabs.end(), [&name](const Symtab::Ptr &it) {
        return it->name() == name;
    });
}

Symtab::Ptr ScopeSymtabs::new0(const Token &token) {
    auto res = find(token.value);
    if (res != m_symtabs.end()) {
        // 错误，已存在的符号表
        m_errors->emplace_back(SymtabError{SymtabErrorType::MULTI_DECLARATION, token, res->get()->m_def});
        return *res;
    } else {
        m_symtabs.emplace_back(make_shared<Symtab>(token.value, loc_from_token(token), m_errors));
        m_symtabs.back()->m_def = loc_from_token(token);
        return m_symtabs.back();
    }
}

bool none(const TreeNode::Ptr &tree_node) {
    return tree_node->type == NodeType::EPSILON || tree_node->type == NodeType::EMPTY;
}

bool complete_return(const TreeNode::Ptr &tree_node) {
    bool complete = false;
    if (tree_node == nullptr) {
        return false;
    } else if (tree_node->content.type == TokenType::RETURN) {
        return true;
    } else if (tree_node->content.value.find(L"{}") == 0) {
        // 代码段
        complete = complete_return(tree_node->children[1]);
    } else if (tree_node->content.type == TokenType::IF) {
        // If
        complete = complete_return(tree_node->children[1]) && complete_return(tree_node->children[2]);
    }
    return complete || complete_return(tree_node->sibling);
}

void ScopeSymtabs::build(const Symtab::Ptr &cur, const TreeNode::Ptr &tree_node) {
    if (tree_node == nullptr) {
    } else if (tree_node->type == NodeType::FUN_DECLARATION) {
        // 声明函数
        auto item = cur->new0(tree_node->content);
        item->type = ItemType::FUN;
        item->set_specifier(tree_node->children[0]->content.type);
        item->size = NO_SIZE;
        item->address = -1;

        // 函数作用域，放置参数用
        auto symtab = new0(tree_node->content);
        symtab->m_parent = cur;
        build(symtab, tree_node->children[1]);
        if (!none(tree_node->children[1])) {
            for (auto param = tree_node->children[1]; param != nullptr; param = param->sibling) {
                ParamDesc desc{param->children[0]->content.type, param->type == NodeType::ARRAY_DECLARATION};
                item->params.emplace_back(desc);
            }
        }
        build(symtab, tree_node->children[2]);

        // 处理返回
        bool complete = complete_return(tree_node->children[2]);
        if (!complete && item->specifier != TokenType::VOID) {
            // 返回没有完备，非void函数要求每个分支都必须返回
            m_errors->emplace_back(SymtabError{SymtabErrorType::NO_COMPLETE_RETURN, tree_node->content});
        }

        // 处理兄弟节点
        build(cur, tree_node->sibling);
    } else if (tree_node->type == NodeType::ARRAY_DECLARATION) {
        // 声明数组
        auto item = cur->new0(tree_node->content);
        item->type = ItemType::ARRAY;
        item->set_specifier(tree_node->children[0]->content.type);
        if (tree_node->children.size() == 2) {
            item->size = stoi(tree_node->children[1]->content.value);
            item->address = cur->m_size;
            cur->m_size += item->size;
        } else {
            // 参数声明，可以是任何长度
            item->size = ANY_SIZE;
            item->address = cur->m_size++;
        }
        if (item->specifier == TokenType::VOID) {
            // 错误，不允许的类型
            m_errors->emplace_back(SymtabError{SymtabErrorType::NOT_ARROWED_TYPE, tree_node->content});
        }
        build(cur, tree_node->sibling);
    } else if (tree_node->type == NodeType::OTHER && tree_node->content.type == TokenType::ID &&
               !tree_node->children.empty()) {
        // 声明普通变量
        auto item = cur->new0(tree_node->content);
        item->type = ItemType::VAR;
        item->set_specifier(tree_node->children[0]->content.type);
        item->size = NO_SIZE;
        item->address = cur->m_size++;
        if (item->specifier == TokenType::VOID) {
            // 错误，不允许的类型
            m_errors->emplace_back(SymtabError{SymtabErrorType::NOT_ARROWED_TYPE, tree_node->content});
        }
        build(cur, tree_node->sibling);
    } else if (tree_node->type == NodeType::OTHER && tree_node->content.value.find(L"{}") == 0) {
        // 代码段作用域
        auto symtab = new0(tree_node->content);
        symtab->m_parent = cur;
        build(symtab, tree_node->children[0]);
        build(symtab, tree_node->children[1]);
        build(cur, tree_node->sibling);
    } else {
        // 其他
        switch (tree_node->content.type) {
            case TokenType::ID:
                cur->check_type(tree_node, TokenType::UNKNOWN);
                break;
            case TokenType::IF:
            case TokenType::WHILE:
                cur->check_type(tree_node->children[0], TokenType::INT);
                for (auto child = tree_node->children.begin() + 1; child != tree_node->children.end(); ++child) {
                    build(cur, *child);
                }
                break;
            case TokenType::RETURN: {
                // 寻找函数节点
                auto fun = cur->in_which_fun();
                // 查找函数定义
                // m_symtabs.front()就是全局作用域
                auto item = m_symtabs.front()->lookup0(fun->name());
                assert(item->specifier == TokenType::INT || item->specifier == TokenType::VOID);
                if (item->specifier == TokenType::INT && tree_node->children[0]->type == NodeType::EMPTY ||
                    item->specifier == TokenType::VOID && tree_node->children[0]->type != NodeType::EMPTY) {
                    // 返回类型不匹配
                    m_errors->emplace_back(SymtabError{SymtabErrorType::TYPE_ERROR, tree_node->content, item->def});
                }
                break;
            }
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
                cur->check_type(tree_node, TokenType::INT);
                break;
            default:
                for (auto &child: tree_node->children) {
                    build(cur, child);
                }
                break;
        }
        build(cur, tree_node->sibling);
    }
}
