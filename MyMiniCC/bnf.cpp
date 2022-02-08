#include "bnf.h"
#include "dot.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <utility>

#define F1(method) [this](auto &_1) { return method(_1); }

using namespace std;

void strip(wstring &ws) {
    ws.erase(ws.begin(), find_if_not(ws.begin(), ws.end(), iswspace));
    ws.erase(find_if_not(ws.rbegin(), ws.rend(), iswspace).base(), ws.end());
}

bool isspace0(int c) {
    return isspace(c);
}

Bnf::Bnf(const initializer_list<wstring> &bnf_strings, TokenName grammar_entry, TokenName epsilon, TokenName end,
         const map<TokenName, TokenType> &new_types, SaveIf save_if)
        : m_grammar_entry(move(grammar_entry)), m_epsilon(move(epsilon)), m_end(move(end)),
          m_save_if(save_if), m_terminal_types() {
    // 字符串产生式规则
    map<wstring, set<wstring>> string_productions;
    for (auto &bnf_string: bnf_strings) {
        // 找到分割点->
        size_t to_split = bnf_string.find(L"->");
        assert(to_split != wstring::npos);

        // 进行分割
        wstring head_string = bnf_string.substr(0, to_split);
        wstring body_string = bnf_string.substr(to_split + 2);
        // 去除两边空白后，断言head不再有空白字符
        strip(head_string);
        assert(find_if(head_string.begin(), head_string.end(), isspace0) == head_string.end());

        // 找到插入位置
        auto string_production = string_productions.find(head_string);
        if (string_production == string_productions.end()) {
            // 如果不存在则先插入空值
            string_production = string_productions.emplace(head_string, set<wstring>()).first;
        }

        // 对body进行分割，然后将每一部分写入string_production.second
        for (auto tem = body_string.begin(); tem != body_string.end();) {
            // 找到下一个分隔符号的位置
            auto next = find(tem, body_string.end(), L'|');
            // 插入
            wstring ws = wstring(tem, next);
            strip(ws);
            string_production->second.emplace(ws);
            tem = next == body_string.end() ? next : next + 1;
        }
    }

    // 断言grammar_entry存在于非终结符中
    assert(string_productions.find(m_grammar_entry) != string_productions.end());
    // 断言epsilon不能是一个非终结符
    assert(string_productions.find(m_epsilon) == string_productions.end());
    // 断言new_types只将终结符转换成其他类型
    for (auto &entry: new_types) {
        assert(string_productions.find(entry.first) == string_productions.end());
    }

    for (auto &entry: string_productions) {
        // 解析head
        wstringstream head_stream(entry.first);
        Scanner head_scanner(head_stream);
        Token head_token = head_scanner.next_token();
        assert(head_token.type == TokenType::ID);
        assert(head_scanner.next_token().type == TokenType::END);
        // 解析body
        ProductionBodies bodies;
        for (auto &ws: entry.second) {
            // 对于每一条规则
            ProductionBody body;
            wstringstream body_stream(ws);
            Scanner body_scanner(body_stream);
            for (Token body_token; (body_token = body_scanner.next_token()).type != TokenType::END;) {
                // 对于规则的每一个Token，填入body
                if (string_productions.find(body_token.value) == string_productions.end() &&
                    body_token.value != m_epsilon) {
                    assert(is_type(body_token.value, body_token.type, true));
                    m_terminal_types.emplace(body_token.value, body_token.type);
                }
                body.emplace_back(body_token.value);
            }
            // 删除多余的epsilon
            remove_redundant_epsilon(body);
            bodies.emplace(move(body));
        }
        m_productions.emplace(head_token.value, move(bodies));
    }

    // 最后更新指定的TokenType
    for (auto &entry: new_types) {
        assert(is_type(entry.first, TokenType::ID, false));
        m_terminal_types.at(entry.first) = entry.second;
    }

    // 计算first集合
    calculate_first();
    // 计算follow集合
    calculate_follow();
}

const Bnf::TokenName &Bnf::grammar_entry() const {
    return m_grammar_entry;
}

const Bnf::TokenName &Bnf::epsilon() const {
    return m_epsilon;
}

const Bnf::TokenName &Bnf::end() const {
    return m_end;
}

Bnf::SaveIf Bnf::save_if() const {
    return m_save_if;
}

const map<Bnf::TokenName, TokenType> &Bnf::terminal_types() const {
    return m_terminal_types;
}

const Bnf::Productions &Bnf::productions() const {
    return m_productions;
}

const map<Bnf::ProductionHead, set<Bnf::TokenName>> &Bnf::first() const {
    return m_first;
}

const map<Bnf::ProductionHead, set<Bnf::TokenName>> &Bnf::follow() const {
    return m_follow;
}

bool Bnf::is_nonterminal(const Bnf::TokenName &token_name) const {
    return m_productions.find(token_name) != m_productions.end();
}

bool Bnf::is_epsilon(const Bnf::TokenName &token_name) const {
    return token_name == m_epsilon;
}

bool Bnf::is_terminal(const Bnf::TokenName &token_name) const {
    return m_terminal_types.find(token_name) != m_terminal_types.end();
}

TokenType Bnf::terminal_type(const Bnf::TokenName &token_name) const {
    return m_terminal_types.at(token_name);
}

set<Bnf::TokenName> Bnf::first(ProductionBody::const_iterator begin, ProductionBody::const_iterator end) const {
    assert(begin != end);
    // 初始化填入m_epsilon
    set<TokenName> res{m_epsilon};
    for (auto token = begin; res.find(m_epsilon) != res.end() && token != end; ++token) {
        res.erase(m_epsilon);
        if (is_nonterminal(*token)) {
            auto &token_first = m_first.at(*token);
            res.insert(token_first.begin(), token_first.end());
        } else {
            res.emplace(*token);
        }
    }
    return res;
}

set<Bnf::TokenName> Bnf::first(const Bnf::ProductionBody &body) const {
    return first(body.cbegin(), body.cend());
}

wstring Bnf::to_string_first() const {
    wstringstream wss;
    for (auto &entry: first()) {
        wss << entry.first << L": ";
        for (auto &token: entry.second) {
            wss << token << L' ';
        }
        wss << endl;
    }
    return wss.str();
}

wstring Bnf::to_string_follow() const {
    wstringstream wss;
    for (auto &entry: follow()) {
        wss << entry.first << L": ";
        for (auto &token: entry.second) {
            wss << token << L' ';
        }
        wss << endl;
    }
    return wss.str();
}

wstring Bnf::to_string_productions() const {
    wstringstream wss;
    for (auto &entry: productions()) {
        wss << entry.first << L" -> ";
        for (auto body = entry.second.begin(); body != entry.second.end(); ++body) {
            if (body != entry.second.begin()) {
                wss << L" | ";
            }
            for (auto token = body->begin(); token != body->end(); ++token) {
                if (token != body->begin()) {
                    wss << L' ';
                }
                wss << *token;
            }
        }
        wss << endl;
    }
    return wss.str();
}

std::wstring Bnf::to_dot_invoke() const {
    DiGraph graph(L"invoke");
    for (auto &entry: productions()) {
        for (auto &body: entry.second) {
            for (auto &token: body) {
                if (is_nonterminal(token)) {
                    graph.add_field(edge(entry.first, token, {}));
                }
            }
        }
    }
    return graph.to_string();
}

wstring quote(const wstring &ws) {
    return L'"' + ws + L'"';
}

wstring Bnf::to_dot() const {
    DiGraph syntax_tree(L"syntax_tree_struct", {{L"dpi", L"200"}}, {
            node(L"node", {{L"fontname", L"\"'JetBrains Mono',Consolas,'Courier New',monospace\""}})
    });

    size_t count = 0;
    auto new_node_no = [&count]() {
        return to_wstring(count++);
    };

    for (auto &entry: productions()) {
        auto cluster = subgraph(L"cluster_" + entry.first, {}, {});

        // 一个点，代表这个cluster
        auto pin = node(entry.first, {
                {L"shape", L"point"},
                {L"label", L"\"\""}
        });

        // 生成pin到某个点的边
        auto pin_node = [&pin](const shared_ptr<Node> &b) {
            return edge(pin->name(), b->name(), {
                    {L"style", L"invis"}
            });
        };

        cluster->add_field(pin);

        for (auto &body: entry.second) {
            if (contains_nonterminal(body) || need_save(body)) {
                // 增加头节点
                auto head = node(new_node_no(), {
                        {L"shape", L"box"},
                        {L"label", quote(entry.first)}
                });
                cluster->add_field(head);
                cluster->add_field(pin_node(head));
                for (auto &token: body) {
                    if (is_nonterminal(token) || save_if0(token)) {
                        // 画一个三角形
                        auto triangle = node(new_node_no(), {
                                {L"shape", L"triangle"},
                                {L"label", quote(token)}
                        });
                        // 从头部到三角形的边
                        auto head_tri = edge(head->name(), triangle->name(), {});
                        if (is_nonterminal(token)) {
                            if (any_of(productions().at(token).begin(), productions().at(token).end(),
                                       F1(is_epsilon_body))) {
                                // 如果该token可以为empty，则边为虚线
                                triangle->set_attr(L"style", L"dashed");
                            }
                        } else {
                            // 不是非终结符，三角形设置为红色
                            triangle->set_attr(L"color", L"red");
                            triangle->set_attr(L"fontcolor", L"red");
                            head_tri->set_attr(L"color", L"red");
                        }
                        cluster->add_field(triangle);
                        cluster->add_field(head_tri);
                    }
                }
            }
        }
        if (cluster->fields().size() == 1) {
            // 之前没有生成任何的节点，生成一个只有头部的节点
            auto head = node(new_node_no(), {
                    {L"shape", L"box"},
                    {L"label", quote(entry.first)}
            });
            cluster->add_field(head);
            cluster->add_field(pin_node(head));
        }
        syntax_tree.add_field(cluster);
    }
    return syntax_tree.to_string();
}

set<pair<Bnf::ProductionHead, Bnf::ProductionHead>> Bnf::calculate_possible_first() const {
    // 计算每一个非终结符可能第一个符号
    // 根据这个可以知道如何减少递归下降发生的冲突
    set<pair<ProductionHead, ProductionHead>> possible_first;
    for (auto &entry: productions()) {
        for (auto &body: entry.second) {
            for (auto &token: body) {
                possible_first.emplace(make_pair(entry.first, token));
                if (is_nonterminal(token)) {
                    auto &token_first = first().at(token);
                    if (token_first.find(epsilon()) != token_first.end()) {
                        // 在A->Bα中，如果B包含ε，就需要再判断后面的
                        continue;
                    }
                }
                break;
            }
        }
    }
    return possible_first;
}

wstring Bnf::to_dot_possible_first() const {
    DiGraph graph(L"possible_first", {{L"dpi", L"200"}}, {});
    for (auto &entry: productions()) {
        graph.add_field(node(quote(entry.first), {{L"shape", L"box"}}));
    }
    for (auto &entry: calculate_possible_first()) {
        graph.add_field(edge(quote(entry.first), quote(entry.second), {}));
    }
    return graph.to_string();
}

wstring file_content(const char *filename) {
    constexpr const char *include_paths[] = {".\\", "..\\"};
    wifstream wifs;
    for (auto &path: include_paths) {
        wifs.clear();
        wifs.open(string(path) + filename);
        if (!wifs.fail()) {
            break;
        }
    }
    assert(wifs.good());
    wstringstream wss;
    wss << wifs.rdbuf();
    return wss.str();
}

wstring replace_all(const wstring &ws, const wstring &a, const wstring &b) {
    assert(!a.empty());
    wstring res = ws;
    for (size_t pos = 0; (pos = res.find(a, pos)) != wstring::npos; pos += b.size()) {
        res.replace(pos, a.size(), b);
    }
    return res;
}

static wstringstream &tab(wstringstream &wss, size_t tab_count) {
    for (size_t i = 0; i < tab_count; ++i) {
        wss << L"    ";
    }
    return wss;
}

bool has_intersection(const set<Bnf::TokenName> &a, const set<Bnf::TokenName> &b) {
    return any_of(a.begin(), a.end(), [&b](const Bnf::TokenName &it) {
        return b.find(it) != b.end();
    });
}

Code Bnf::generate_code(set<TokenType> keyword_to_save) const {
    wstring h_template = file_content("parse_template.h");
    wstring cpp_template = file_content("parse_template.cpp");

    h_template = replace_all(h_template, L"PARSE_TEMPLATE", L"PARSE");
    cpp_template = replace_all(cpp_template, L"parse_template", L"parse");
    cpp_template = replace_all(cpp_template, L"/*TO INFLATE: GRAMMAR_ENTRY*/", m_grammar_entry + L"()");

    // parse.h文件
    // 生成声明函数
    wstringstream declare_functions_wss;
    for (auto entry = productions().begin(); entry != productions().end(); ++entry) {
        if (entry != productions().begin()) {
            declare_functions_wss << endl << endl;
        }
        tab(declare_functions_wss, 1) << L"TreeNode::Ptr " << entry->first << L"();";
    }
    h_template = replace_all(h_template, L"/*TO INFLATE: DECLARE*/", declare_functions_wss.str());

    // 生成定义
    wstringstream define_functions_wss;
    // 递归非终结符和可展开符号
    set<TokenName> recursions;
    set<TokenName> can_expand;
    for (auto &entry: productions()) {
        for (auto &body: entry.second) {
            if (entry.first == body.back()) {
                recursions.emplace(entry.first);
            }
        }
        if (all_of(entry.second.begin(), entry.second.end(), [this](const ProductionBody &body) {
            return body.size() == 1 && is_terminal(body.front());
        })) {
            can_expand.emplace(entry.first);
        }
    }
    for (auto entry = productions().begin(); entry != productions().end(); ++entry) {
        if (entry != productions().begin()) {
            declare_functions_wss << endl << endl;
        }
        define_functions_wss << L"TreeNode::Ptr Parser::" << entry->first << L"() {" << endl;
        tab(define_functions_wss, 1) << L"TreeNode::Ptr res = make_shared<TreeNode>();"
                                     << endl;
        tab(define_functions_wss, 1);
        // 已经使用过的first，如果下一个body的first集存在在used集里的值，那么就发生了冲突
        set<TokenName> used_first;
        bool maybe_epsilon = false;
        bool first_output = true;
        auto keyword_if = [&first_output]() {
            if (first_output) {
                // 第一次用if
                first_output = false;
                return L"if (";
            } else {
                // 其他用else if
                return L"else if (";
            }
        };
        for (auto &body: entry->second) {
            if (is_epsilon_body(body)) {
                maybe_epsilon = true;
            } else {
                // 条件
                auto body_first = first(body);
                maybe_epsilon |= body_first.erase(m_epsilon);
                // 判断是否有冲突
                bool first_conflict = has_intersection(used_first, body_first);
                used_first.insert(body_first.begin(), body_first.end());
                assert(!first_conflict);
                define_functions_wss << keyword_if();
                // 输出条件
                define_functions_wss << to_string_conditions(L"m_cur.value", L"m_cur.type", body_first);
                define_functions_wss << L") {" << endl;
                size_t match_i = 0;
                size_t call_i = 0;
                vector<wstring> names;
                for (auto &token: body) {
                    tab(define_functions_wss, 2);
                    if (is_terminal(token)) {
                        wstring tem = wstring(L"match(TokenType::") + english_name(terminal_type(token)) + L")";
                        if (save_if0(token) || keyword_to_save.find(terminal_type(token)) != keyword_to_save.end()) {
                            wstring name = L"match" + to_wstring(match_i++);
                            define_functions_wss << L"auto " << name << L" = " << tem << L";";
                            names.emplace_back(move(name));
                        } else {
                            define_functions_wss << tem << L';';
                        }
                    } else if (can_expand.find(token) != can_expand.end()) {
                        // 可展开
                        wstring name = L"match" + to_wstring(match_i++);
                        define_functions_wss << L"auto " << name << L" = " << token << L"()->content;";
                        names.emplace_back(move(name));
                    } else {
                        wstring name = L"call" + to_wstring(call_i++);
                        define_functions_wss << L"auto " << name << L" = " << token << L"();";
                        names.emplace_back(move(name));
                    }
                    define_functions_wss << endl;
                }
                if (match_i == 0 && call_i == 0) {
                    // 无需记录任何信息
                    tab(define_functions_wss, 2) << L"res->set_type(NodeType::EMPTY);" << endl;
                } else {
                    if (match_i >= 1) {
                        wstring name = L"match" + to_wstring(match_i - 1);
                        tab(define_functions_wss, 2) << L"res->content = move(" << name << ");" << endl;
                        names.erase(find(names.begin(), names.end(), name));
                    }
                    if (recursions.find(body.back()) != recursions.end()) {
                        // 递归
                        names.erase(names.end() - 1);
                    }
                    if (!names.empty()) {
                        if (match_i == 0 && call_i == 1) {
                            // 中间节点
                            tab(define_functions_wss, 2) << L"res = call0;" << endl;
                        } else {
                            tab(define_functions_wss, 2) << L"store(res, ";
                            for (auto name = names.begin(); name != names.end(); ++name) {
                                if (name != names.begin()) {
                                    define_functions_wss << L", ";
                                }
                                define_functions_wss << *name;
                            }
                            define_functions_wss << L");" << endl;
                        }
                    }
                    if (recursions.find(body.back()) != recursions.end()) {
                        wstring name = L"call" + to_wstring(call_i - 1);
                        if (names.empty()) {
                            tab(define_functions_wss, 2) << L"res = call0;" << endl;
                        } else {
                            tab(define_functions_wss, 2) << L"store_sibling(res, call" << (call_i - 1) << L");" << endl;
                        }
                    }
                }
                // 结束
                tab(define_functions_wss, 1) << L"} ";
            }
        }
        auto require = move(used_first);
        if (maybe_epsilon) {
            // 该TreeNode可能是epsilon
            define_functions_wss << keyword_if();
            auto &head_follow = follow().at(entry->first);
            require.insert(head_follow.begin(), head_follow.end());
            define_functions_wss << to_string_conditions(L"m_cur.value", L"m_cur.type", head_follow);
            define_functions_wss << L") {" << endl;
            tab(define_functions_wss, 2) << L"res->set_type(NodeType::EPSILON);" << endl;
            tab(define_functions_wss, 1) << L"} ";
        }
        define_functions_wss << L"else {" << endl;
        tab(define_functions_wss, 2) << L"res->set_type(NodeType::ERROR);" << endl;
        tab(define_functions_wss, 2) << L"record_error({";
        for (auto it = require.begin(); it != require.end(); ++it) {
            if (it != require.begin()) {
                define_functions_wss << L", ";
            }
            define_functions_wss << L"L\"" << *it << L"\"";
        }
        define_functions_wss << L"});" << endl;
        tab(define_functions_wss, 1) << L'}' << endl;
        tab(define_functions_wss, 1) << L"check(res);" << endl;
        tab(define_functions_wss, 1) << L"return res;" << endl;
        define_functions_wss << L'}' << endl << endl;
    }
    cpp_template = replace_all(cpp_template, L"/*TO INFLATE: DEFINE*/", define_functions_wss.str());

    return Code{h_template, cpp_template};
}

bool Bnf::is_type(const Bnf::TokenName &token_name, TokenType token_type, bool not_exist) const {
    auto it = m_terminal_types.find(token_name);
    if (it != m_terminal_types.end()) {
        return it->second == token_type;
    }
    return not_exist;
}

void Bnf::remove_redundant_epsilon(Bnf::ProductionBody &body) const {
    assert(!body.empty());
    for (int i = 0; body.size() > 1 && i < body.size(); ++i) {
        if (body.at(i) == m_epsilon) {
            body.erase(body.begin() + i);
            --i;
        }
    }
}

void Bnf::calculate_first() {
    m_first.clear();
    for (auto &entry: productions()) {
        m_first.emplace(entry.first, set<TokenName>());
    }
    for (bool first_set_change = true; first_set_change;) {
        // 只要first集改变了，说明还未完全找到first集的元素
        first_set_change = false;
        for (auto &entry: productions()) {
            auto &store = m_first.at(entry.first);
            size_t old_size = store.size();
            for (auto &body: entry.second) {
                // 一个非终结符的first集相当于它的每个body的first集的并集
                auto res = first(body);
                store.insert(res.begin(), res.end());
            }
            if (store.size() != old_size) {
                first_set_change = true;
            }
        }
    }
}

void Bnf::calculate_follow() {
    m_follow.clear();
    for (auto &entry: m_productions) {
        m_follow.emplace(entry.first, set<TokenName>());
    }
    m_follow.at(m_grammar_entry).emplace(m_end);
    for (bool follow_change = true; follow_change;) {
        follow_change = false;
        for (auto &entry: m_productions) {
            for (auto &body: entry.second) {
                for (auto token = body.begin(); token != body.end(); ++token) {
                    if (is_nonterminal(*token)) {
                        // 只有非终结符才有follow集
                        auto &store = m_follow.at(*token);
                        size_t old_size = store.size();
                        if (token + 1 != body.end()) {
                            // 如果token不是最后一个，那么非终结符的follow集就是它后面的first集（有epsilon还要加上entry.first的follow集）
                            auto res = first(ProductionBody(token + 1, body.end()));
                            if (res.find(m_epsilon) != res.end()) {
                                res.erase(m_epsilon);
                                auto head_follow = m_follow.at(entry.first);
                                res.insert(head_follow.begin(), head_follow.end());
                            }
                            store.insert(res.begin(), res.end());
                        } else {
                            // 最后一个token，填入head_follow
                            auto head_follow = m_follow.at(entry.first);
                            store.insert(head_follow.begin(), head_follow.end());
                        }
                        if (store.size() != old_size) {
                            follow_change = true;
                        }
                    }
                }
            }
        }
    }
}

bool Bnf::contains_nonterminal(const Bnf::ProductionBody &body) const {
    return any_of(body.begin(), body.end(), F1(is_nonterminal));
}

bool Bnf::is_epsilon_body(const Bnf::ProductionBody &body) const {
    return body.size() == 1 && is_epsilon(body.front());
}

bool Bnf::save_if0(const TokenName &token_name) const {
    auto it = terminal_types().find(token_name);
    if (it != terminal_types().end()) {
        return save_if()(it->second);
    }
    return false;
}

bool Bnf::need_save(const Bnf::ProductionBody &body) const {
    return any_of(body.begin(), body.end(), F1(save_if0));
}

size_t Bnf::save_count(const Bnf::ProductionBody &body) const {
    return count_if(body.begin(), body.end(), F1(save_if0));
}

wstring Bnf::enum_name(const Bnf::TokenName &token_name) const {
    wstringstream wss;
    if (is_nonterminal(token_name)) {
        for (auto &c: token_name) {
            wss.put(toupper(c));
        }
        return wss.str();
    } else if (token_name == epsilon()) {
        return L"EPSILON";
    } else {
        return english_name(terminal_type(token_name));
    }
}

wstring Bnf::to_string_conditions(const wstring &token_value_name, const wstring &token_type_name,
                                  const set<Bnf::TokenName> &conditions) const {
    wstringstream wss;
    for (auto it = conditions.begin(); it != conditions.end(); ++it) {
        if (it != conditions.begin()) {
            wss << L" || ";
        }
        if (is_epsilon(*it)) {
            wss << token_value_name << L" == \"" << epsilon() << "\"";
        } else if (*it == m_end) {
            wss << token_type_name << L" == TokenType::END";
        } else {
            wss << token_type_name << L" == TokenType::" << english_name(terminal_type(*it));
        }
    }
    return wss.str();
}

set<Bnf::TokenName> get_nonterminals(const vector<pair<Bnf::ProductionHead, Bnf::ProductionBodies>> &tem_productions) {
    set<Bnf::TokenName> nonterminals;
    for (auto &entry: tem_productions) {
        nonterminals.emplace(entry.first);
    }
    return nonterminals;
}

Ebnf::Ebnf(const initializer_list<wstring> &bnf_strings, TokenName grammar_entry,
           TokenName epsilon, TokenName end, const map<TokenName, TokenType> &new_types,
           SaveIf save_if)
        : Bnf(bnf_strings, move(grammar_entry), move(epsilon), move(end), new_types, save_if) {
    // 临时的productions
    vector<pair<ProductionHead, ProductionBodies>> tem_productions = tem();
    remove_left_recursion(tem_productions);
    remove_left_associativity(tem_productions, {});
    assign(tem_productions);
    tem_productions = tem();
    /// 假设不存在间接左递归
    // 让其能够进行递归下降
    size_t size = tem_productions.size();
    for (int cur_pos = 0; cur_pos != tem_productions.size(); ++cur_pos) {
        auto cur = tem_productions.begin() + cur_pos;
        // 展开bodies
        auto expanded = expand(cur->second, cur->first);
        if (expanded != cur->second) {
            vector<pair<ProductionHead, ProductionBodies>> tem_expanded{
                    make_pair(cur->first, ProductionBodies(expanded.begin(), expanded.end()))
            };
            remove_left_associativity(tem_expanded, get_nonterminals(tem_productions));
            tem_productions.insert(tem_productions.end(), tem_expanded.begin(), tem_expanded.end());
            tem_productions.erase(tem_productions.begin() + cur_pos--);
        }
    }
    assign(tem_productions);

    // 删除所有没有指向的产生式
    vector<TokenName> used{m_grammar_entry};
    // 从m_grammar_entry开始，遍历整颗树，被访问过的则为使用节点
    for (size_t i = 0; i < used.size(); ++i) {
        auto &bodies = m_productions.at(used.at(i));
        for (auto &body: bodies) {
            for (auto &token: body) {
                if (is_nonterminal(token) && find(used.begin(), used.end(), token) == used.end()) {
                    used.emplace_back(token);
                }
            }
        }
    }
    set<TokenName> unused;
    for (auto &entry: m_productions) {
        if (find(used.begin(), used.end(), entry.first) == used.end()) {
            unused.emplace(entry.first);
        }
    }
    for (auto &it: unused) {
        m_productions.erase(it);
        m_first.erase(it);
        m_follow.erase(it);
    }
}

vector<pair<Bnf::ProductionHead, Bnf::ProductionBodies>> Ebnf::tem() const {
    vector<pair<ProductionHead, ProductionBodies>> tem_productions;
    for (auto &entry: productions()) {
        tem_productions.emplace_back(make_pair(entry.first, entry.second));
    }
    return tem_productions;
}

void Ebnf::assign(vector<pair<Bnf::ProductionHead, Bnf::ProductionBodies>> tem_productions) {
    m_productions.clear();
    for (auto &entry: tem_productions) {
        m_productions.emplace(entry.first, entry.second);
    }
    // 更新first和follow集
    calculate_first();
    calculate_follow();
}

set<Bnf::TokenName> union0(const set<Bnf::TokenName> &a, const set<Bnf::TokenName> &b) {
    set<Bnf::TokenName> c;
    c.insert(a.begin(), a.end());
    c.insert(b.begin(), b.end());
    return c;
}

Bnf::TokenName generate_new_name(const Bnf::TokenName &base_name, const set<Bnf::TokenName> &exist_names) {
    Bnf::TokenName new_name = base_name;
    for (size_t i = 0;
         exist_names.find(new_name) != exist_names.end(); new_name = base_name + L"Ex" + wstring(i, L'I'), ++i) {
    }
    return new_name;
}

void Ebnf::remove_left_recursion(vector<pair<Bnf::ProductionHead, Bnf::ProductionBodies>> &tem_productions) const {
    size_t size = tem_productions.size();
    for (int cur_pos = 0; cur_pos < size; ++cur_pos) {
        auto cur = tem_productions.begin() + cur_pos;
        // 如果cur的body中存在开头是cur的head的，才说明需要消除左递归
        if (any_of(cur->second.begin(), cur->second.end(),
                   [&cur](auto &it) { return it.front() == cur->first; })) {
            // 构建新的产生式A->cA'，A'->bA'|ε
            auto AEx = ::generate_new_name(cur->first, get_nonterminals(tem_productions));
            ProductionBodies bAEx_epsilon;
            ProductionBodies cAEx;
            for (auto &cur_body: cur->second) {
                if (cur_body.front() == cur->first) {
                    assert(cur_body.size() > 1);
                    auto tem = ProductionBody(cur_body.begin() + 1, cur_body.end());
                    tem.emplace_back(AEx);
                    remove_redundant_epsilon(tem);
                    bAEx_epsilon.emplace(tem);
                } else {
                    auto tem = cur_body;
                    tem.emplace_back(AEx);
                    remove_redundant_epsilon(tem);
                    cAEx.emplace(tem);
                }
            }
            bAEx_epsilon.emplace(ProductionBody{m_epsilon});

            cur->second = cAEx;
            tem_productions.emplace_back(make_pair(AEx, move(bAEx_epsilon)));
            // emplace_back后要更新cur
            cur = tem_productions.begin() + cur_pos;
        }
    }
}

void Ebnf::remove_left_associativity(vector<std::pair<ProductionHead, ProductionBodies>> &tem_productions,
                                     set<TokenName> exist_names) const {
    exist_names = union0(get_nonterminals(tem_productions), exist_names);
    size_t size = tem_productions.size();
    for (int cur_pos = 0; cur_pos < size; ++cur_pos) {
        auto cur = tem_productions.begin() + cur_pos;

        ProductionBodies new_bodies;
        vector<ProductionBody> tem;
        tem.insert(tem.end(), cur->second.begin(), cur->second.end());
        while (!tem.empty()) {
            // 筛选tem，将所有产生式开头和第一条一样的提取出来
            vector<ProductionBody> res{tem.front()};
            tem.erase(tem.begin());
            for (int i = 0; i < tem.size(); ++i) {
                auto it = tem.begin() + i;
                if (it->front() == res.front().front()) {
                    // 如果第一位相同
                    res.emplace_back(*it);
                    tem.erase(it);
                    --i;
                }
            }
            if (res.size() > 1) {
                // 有多条产生式一样
                // 判断res中第pos位的token是否都一样
                auto all_same_with_first_at_pos = [&res](size_t pos) {
                    return all_of(res.begin(), res.end(), [&res, &pos](const ProductionBody &it) {
                        return it.size() > pos && it.at(pos) == res.front().at(pos);
                    });
                };

                int pos;
                for (pos = 0; pos < res.front().size() && all_same_with_first_at_pos(pos); ++pos) {
                }

                ProductionBody new_A_body(res.front().begin(), res.front().begin() + pos);
                ProductionBodies AEx_bodies;
                auto AEx = ::generate_new_name(cur->first, exist_names);
                new_A_body.emplace_back(AEx);
                for (auto &body: res) {
                    if (pos == body.size()) {
                        AEx_bodies.emplace(ProductionBody{m_epsilon});
                    } else {
                        AEx_bodies.emplace(ProductionBody(body.begin() + pos, body.end()));
                    }
                }
                new_bodies.emplace(new_A_body);
                tem_productions.emplace_back(make_pair(AEx, AEx_bodies));
                // emplace_back后要更新cur
                cur = tem_productions.begin() + cur_pos;
            } else {
                new_bodies.emplace(res.front());
            }
        }
        cur->second = new_bodies;
    }
}

Bnf::ProductionBodies
Ebnf::replace(const ProductionBody &body, size_t begin, size_t end, const ProductionBodies &to_inserts) const {
    vector<Bnf::ProductionBody> new_bodies;
    for (auto &to_insert: to_inserts) {
        Bnf::ProductionBody new_body;
        new_body.insert(new_body.end(), body.begin(), body.begin() + begin);
        new_body.insert(new_body.end(), to_insert.begin(), to_insert.end());
        new_body.insert(new_body.end(), body.begin() + end, body.end());
        remove_redundant_epsilon(new_body);
        new_bodies.emplace_back(move(new_body));
    }
    return ProductionBodies(new_bodies.begin(), new_bodies.end());
}

Bnf::ProductionBodies
Ebnf::expand0(const ProductionBodies &bodies, const set<TokenName> &to_expands, const TokenName &head,
              set<TokenName> &tabu) const {
    tabu.emplace(head);
    vector<ProductionBody> tem_bodies(bodies.begin(), bodies.end());
    ProductionBodies new_bodies;
    for (int i = 0; i < tem_bodies.size(); ++i) {
        auto body = tem_bodies.begin() + i;
        for (int j = 0; j < body->size(); ++j) {
            auto production_first = first(body->cbegin() + j, body->end());
            if (has_intersection(production_first, to_expands) && is_nonterminal(body->at(j)) &&
                tabu.find(body->at(j)) == tabu.end()) {
                auto replaced = replace(*body, 0, j + 1,
                                        expand0(productions().at(body->at(j)), to_expands, body->at(j), tabu));
                new_bodies.insert(replaced.begin(), replaced.end());
                if (first().at(body->at(j)).count(epsilon()) == 0) {
                    break;
                }
            } else {
                new_bodies.emplace(ProductionBody(body->begin() + j, body->end()));
                break;
            }
        }
    }
    tabu.erase(head);
    return new_bodies;
}

Bnf::ProductionBodies Ebnf::expand(const ProductionBodies &bodies, const TokenName &head) const {
    set<TokenName> used_first;
    set<TokenName> to_expands;
    for (auto &body: bodies) {
        auto first = this->first(body);
        set_intersection(first.begin(), first.end(), used_first.begin(), used_first.end(),
                         inserter(to_expands, to_expands.end()));
        used_first.insert(first.begin(), first.end());
    }
    set<TokenName> tabu;
    return expand0(bodies, to_expands, head, tabu);
}
