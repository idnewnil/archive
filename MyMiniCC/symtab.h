#ifndef SYMTAB_H
#define SYMTAB_H

#include "parse.h"
#include "dot.h"

#define ANY_SIZE (-1)
#define NO_SIZE (-2)

struct Location {
    size_t count = size_t(-1);
    size_t lineno = size_t(-1);
    size_t colno = size_t(-1);
};

Location loc_from_token(const Token &token);

enum class SymtabErrorType {
    NOT_ARROWED_TYPE,
    NOT_FOUND,
    WRONG_PARAM_SIZE,
    REQUIRE_ARRAY,
    INCORRECT_USE,
    MULTI_DECLARATION,
    TYPE_ERROR,
    ERROR_EXP_TYPE,
    NO_COMPLETE_RETURN,
    NO_MAIN
};

struct SymtabError {
    SymtabErrorType type;
    Token token;
    Location conflict;
};

enum class ItemType {
    FUN, ARRAY, VAR
};

inline const wchar_t *item_type_name(ItemType item_type) {
    switch (item_type) {
        case ItemType::FUN:
            return L"FUN";
        case ItemType::ARRAY:
            return L"ARRAY";
        case ItemType::VAR:
            return L"VAR";
        default:
            assert(false);
            return L"???";
    }
}

// 参数描述
struct ParamDesc {
    TokenType type; // 参数类型
    bool is_array; // 是否是数组
};

struct SymtabItem {
    using Ptr = std::shared_ptr<SymtabItem>;

    ItemType type; // 符号类型
    TokenType specifier; // 类型描述符
    int size; // 数组大小
    std::vector<ParamDesc> params; // 函数参数的描述，变量则不使用该字段
    Location def; // 定义位置
    size_t address; // 相对地址，如果在函数内则相对于函数作用域否则相对于全局作用域
    std::vector<Location> refs; // 引用位置

    void set_specifier(TokenType token_type);
};

// 作用域类型
enum class ScopeType {
    NONE, GLOBAL, FUN, STMT
};

class Symtab {
public:
    friend class ScopeSymtabs;

    friend class CodeGenerator;

    Symtab();

    Symtab(std::wstring name, Location def, std::shared_ptr<std::vector<SymtabError>> errors);

    using Ptr = std::shared_ptr<Symtab>;

    const std::wstring &name() const;

    ScopeType type() const;

    // 返回结果int为地址，ScopeType为是在哪种作用域找到该变量的
    std::pair<int, ScopeType> lookup(const std::wstring &name) const;

    std::wstring to_string() const;

private:
    Ptr m_parent;
    std::wstring m_name;
    std::vector<std::pair<std::wstring, SymtabItem::Ptr>> m_data;
    Location m_def; // 定义位置
    std::shared_ptr<std::vector<SymtabError>> m_errors;
    size_t m_size;

    std::vector<std::pair<std::wstring, SymtabItem::Ptr>>::const_iterator find(const std::wstring &name) const;

    SymtabItem::Ptr new0(const Token &token);

    SymtabItem::Ptr lookup0(const std::wstring &name) const;

    bool is_compoundstmt() const;

    Ptr in_which_fun() const;

    TokenType check_type(const TreeNode::Ptr &tree_node, TokenType specifier) const;
};

bool none(const TreeNode::Ptr &tree_node);

// 返回是否每个分支都返回了
bool complete_return(const TreeNode::Ptr &tree_node);

// 作用域符号表集合，根据作用域
class ScopeSymtabs {
public:
    friend class CodeGenerator;

    explicit ScopeSymtabs(const TreeNode::Ptr &tree_node);

    const std::vector<Symtab::Ptr> &symtabs() const;

    const std::shared_ptr<std::vector<SymtabError>> &errors() const;

    std::wstring to_string() const;

private:
    // 一个 <作用域, 符号表> 映射
    std::vector<Symtab::Ptr> m_symtabs;
    std::shared_ptr<std::vector<SymtabError>> m_errors;

    std::vector<Symtab::Ptr>::const_iterator find(const std::wstring &name) const;

    Symtab::Ptr new0(const Token &token);

    void build(const Symtab::Ptr &cur, const TreeNode::Ptr &tree_node);
};

#endif // SYMTAB_H
