#ifndef DOT_H
#define DOT_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class IAttrStore {
public:
    void set_attr(const std::wstring &key, const std::wstring &value);

    void set_attrs(std::initializer_list<std::pair<std::wstring, std::wstring>> attrs);

    const std::map<std::wstring, std::wstring> &attrs() const;


    std::wstring to_string_attrs() const;

private:
    std::map<std::wstring, std::wstring> m_attrs;
};

class Object : public IAttrStore {
public:
    virtual std::wstring to_string(size_t tab_count) const = 0;
};

class Field : public Object {
};

class IFieldStore {
public:
    void add_field(const std::shared_ptr<Field> &field);

    void add_fields(std::initializer_list<std::shared_ptr<Field>> fields);

    const std::vector<std::shared_ptr<Field>> &fields() const;


    std::wstring to_string_fields(size_t tab_count) const;

private:
    std::vector<std::shared_ptr<Field>> m_fields;
};

class DiGraph : public Object, public IFieldStore {
public:
    explicit DiGraph(std::wstring name);

    DiGraph(std::wstring name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs,
            std::initializer_list<std::shared_ptr<Field>> fields);


    const std::wstring &name() const;


    std::wstring to_string() const;

    std::wstring to_string(size_t tab_count) const override;

private:
    std::wstring m_name;
};

class SubGraph : public Field, public IFieldStore {
public:
    explicit SubGraph(std::wstring name);

    SubGraph(std::wstring name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs,
             std::initializer_list<std::shared_ptr<Field>> fields);


    const std::wstring &name() const;


    std::wstring to_string(size_t tab_count) const override;

private:
    std::wstring m_name;
};

class Node : public Field {
public:
    explicit Node(std::wstring name);

    Node(std::wstring name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs);


    const std::wstring &name() const;


    std::wstring to_string(size_t tab_count) const override;

private:
    std::wstring m_name;
};

class Edge : public Field {
public:
    Edge(std::wstring a, std::wstring b);

    Edge(std::wstring a, std::wstring b, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs);


    const std::wstring &a() const;

    const std::wstring &b() const;


    std::wstring to_string(size_t tab_count) const override;

private:
    std::wstring m_a;
    std::wstring m_b;
};

std::shared_ptr<SubGraph>
subgraph(const std::wstring &name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs,
         std::initializer_list<std::shared_ptr<Field>> fields);

std::shared_ptr<Node>
node(const std::wstring &name, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs);

std::shared_ptr<Edge>
edge(const std::wstring &a, const std::wstring &b, std::initializer_list<std::pair<std::wstring, std::wstring>> attrs);

#endif // DOT_H
