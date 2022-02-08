#include "dot.h"

#include <memory>
#include <sstream>
#include <utility>

using namespace std;

void IAttrStore::set_attr(const wstring &key, const wstring &value) {
    m_attrs.emplace(key, value);
}

void IAttrStore::set_attrs(initializer_list<pair<wstring, wstring>> attrs) {
    for (auto &attr: attrs) {
        set_attr(attr.first, attr.second);
    }
}

const map<wstring, wstring> &IAttrStore::attrs() const {
    return m_attrs;
}

wstring IAttrStore::to_string_attrs() const {
    wstringstream wss;
    wss << L"[";
    for (auto it = attrs().begin(); it != attrs().end(); ++it) {
        if (it != attrs().begin()) {
            wss << L", ";
        }
        wss << it->first << L'=' << it->second;
    }
    wss << L"]";
    return wss.str();
}

void IFieldStore::add_field(const shared_ptr<Field> &field) {
    m_fields.emplace_back(field);
}

void IFieldStore::add_fields(initializer_list<shared_ptr<Field>> fields) {
    m_fields.insert(m_fields.end(), fields.begin(), fields.end());
}

const vector<shared_ptr<Field>> &IFieldStore::fields() const {
    return m_fields;
}

static wstringstream &tab(wstringstream &wss, size_t tab_count) {
    for (size_t i = 0; i < tab_count; ++i) {
        wss << L"    ";
    }
    return wss;
}

wstring IFieldStore::to_string_fields(size_t tab_count) const {
    wstringstream wss;
    for (auto it = fields().begin(); it != fields().end(); ++it) {
        if (it != fields().begin()) {
            wss << endl;
        }
        wss << (*it)->to_string(tab_count);
    }
    return wss.str();
}

DiGraph::DiGraph(wstring name) : m_name(move(name)) {
}

DiGraph::DiGraph(wstring name, initializer_list<pair<wstring, wstring>> attrs,
                 initializer_list<shared_ptr<Field>> fields) : m_name(move(name)) {
    set_attrs(attrs);
    add_fields(fields);
}

const std::wstring &DiGraph::name() const {
    return m_name;
}

wstring DiGraph::to_string() const {
    return to_string(0);
}

wstring DiGraph::to_string(size_t tab_count) const {
    wstringstream wss;
    wss << L"strict digraph " << m_name << L" {" << endl;
    if (!attrs().empty()) {
        tab(wss, tab_count + 1) << L"graph " << to_string_attrs() << L';' << endl << endl;
    }
    wss << to_string_fields(tab_count + 1) << endl;
    tab(wss, tab_count) << L"}";
    return wss.str();
}

SubGraph::SubGraph(wstring name) : m_name(move(name)) {
}

SubGraph::SubGraph(wstring name, initializer_list<pair<wstring, wstring>> attrs,
                   initializer_list<shared_ptr<Field>> fields) : m_name(move(name)) {
    set_attrs(attrs);
    add_fields(fields);
}

wstring SubGraph::to_string(size_t tab_count) const {
    wstringstream wss;
    tab(wss, tab_count) << L"subgraph ";
    if (!m_name.empty()) {
        wss << m_name << L' ';
    }
    wss << L"{" << endl;
    if (!attrs().empty()) {
        tab(wss, tab_count + 1) << L"graph " << to_string_attrs() << L';' << endl << endl;
    }
    wss << to_string_fields(tab_count + 1) << endl;
    tab(wss, tab_count) << L"}";
    return wss.str();
}

Node::Node(wstring name) : m_name(move(name)) {
}

Node::Node(wstring name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs) : m_name(std::move(name)) {
    set_attrs(attrs);
}

const std::wstring &Node::name() const {
    return m_name;
}

wstring Node::to_string(size_t tab_count) const {
    wstringstream wss;
    tab(wss, tab_count) << m_name;
    if (!attrs().empty()) {
        wss << L' ' << to_string_attrs();
    }
    wss << L';';
    return wss.str();
}

Edge::Edge(wstring a, wstring b) : m_a(move(a)), m_b(move(b)) {
}

Edge::Edge(std::wstring a, std::wstring b, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs)
        : m_a(std::move(a)), m_b(std::move(b)) {
    set_attrs(attrs);
}

const std::wstring &Edge::a() const {
    return m_a;
}

const std::wstring &Edge::b() const {
    return m_b;
}

wstring Edge::to_string(size_t tab_count) const {
    wstringstream wss;
    tab(wss, tab_count) << m_a << L" -> " << m_b;
    if (!attrs().empty()) {
        wss << L' ' << to_string_attrs();
    }
    wss << L';';
    return wss.str();
}

shared_ptr<SubGraph> subgraph(const wstring &name, initializer_list<pair<wstring, wstring>> attrs,
                           initializer_list<shared_ptr<Field>> fields) {
    return make_shared<SubGraph>(name, attrs, fields);
}

shared_ptr<Node> node(const wstring &name, initializer_list<pair<wstring, wstring>> attrs) {
    return make_shared<Node>(name, attrs);
}

shared_ptr<Edge> edge(const wstring &a, const wstring &b, initializer_list<pair<wstring, wstring>> attrs) {
    return make_shared<Edge>(a, b, attrs);
}
