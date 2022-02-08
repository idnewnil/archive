#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "symtab.h"

enum class Reg {
    // 数据寄存器
    AX = 0, BX = 1, //CX = 2, DX = 3,
    // 堆栈寄存器
    BP = 5, SP = 6,
    // 指令指针寄存器
    IP = 7
};

inline const wchar_t *reg_name(Reg reg) {
    switch (reg) {
        case Reg::AX:
            return L"AX";
        case Reg::BX:
            return L"BX";
        case Reg::BP:
            return L"BP";
        case Reg::SP:
            return L"SP";
        case Reg::IP:
            return L"IP";
        default:
            assert(false);
            return L"???";
    }
}

struct Mem {
    Reg s;
    int d;
};

enum class BackPatchType {
    NONE, FUN_ADDRESS
};

struct BackPatch {
    BackPatchType type = BackPatchType::NONE;
    std::wstring fun_name;
};

struct Instruction {
    const wchar_t *opcode = nullptr;
    size_t r = 0;
    size_t s = 0;
    union {
        size_t t = 0;
        int d;
    };

    // d的back_patch
    BackPatch bp_d;

    inline static Instruction HALT() { return {L"HALT", 0, 0, 0}; }

    inline static Instruction IN(Reg r) { return {L"IN", size_t(r)}; }

    inline static Instruction OUT(Reg r) { return {L"OUT", size_t(r)}; }

    inline static Instruction ADD(Reg r, Reg s, Reg t) { return {L"ADD", size_t(r), size_t(s), size_t(t)}; }

    inline static Instruction SUB(Reg r, Reg s, Reg t) { return {L"SUB", size_t(r), size_t(s), size_t(t)}; }

    inline static Instruction MUL(Reg r, Reg s, Reg t) { return {L"MUL", size_t(r), size_t(s), size_t(t)}; }

    inline static Instruction DIV(Reg r, Reg s, Reg t) { return {L"DIV", size_t(r), size_t(s), size_t(t)}; }

    inline static Instruction LD(Reg r, Mem m) { return {L"LD", size_t(r), size_t(m.s), size_t(m.d)}; }

    inline static Instruction LDA(Reg r, Reg s, int d) { return {L"LDA", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction LDC(Reg r, int d) { return {L"LDC", size_t(r), 0, size_t(d)}; }

    inline static Instruction ST(Reg r, Mem m) { return {L"ST", size_t(r), size_t(m.s), size_t(m.d)}; }

    inline static Instruction JLT(Reg r, Reg s, int d) { return {L"JLT", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction JLE(Reg r, Reg s, int d) { return {L"JLE", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction JGE(Reg r, Reg s, int d) { return {L"JGE", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction JGT(Reg r, Reg s, int d) { return {L"JGT", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction JEQ(Reg r, Reg s, int d) { return {L"JEQ", size_t(r), size_t(s), size_t(d)}; }

    inline static Instruction JNE(Reg r, Reg s, int d) { return {L"JNE", size_t(r), size_t(s), size_t(d)}; }
};

class CodeGenerator {
public:
    CodeGenerator(const TreeNode::Ptr &tree_node, const ScopeSymtabs &scope_symtabs);

    std::wstring to_string() const;

private:
    std::vector<Instruction> m_instructions;
    std::map<int, std::vector<std::wstring>> m_comments;
    size_t m_comment_tab;
    std::map<int, std::wstring> m_inline_comments;
    std::map<std::wstring, int> m_fun_back_patch;

    size_t gen(Instruction instruction);

    void begin_comment(const std::wstring &ws);

    void comment(const std::wstring &ws);

    void end_comment(const std::wstring &ws);

    void inline_comment(std::wstring comment, std::wstring default_comment = L"");

    size_t mova(Reg a, Mem m, std::wstring comment = L"");

    void mov(Reg a, Reg b, std::wstring comment = L"");

    size_t mov(Reg a, Mem m, std::wstring comment = L"");

    size_t mov(Mem m, Reg a, std::wstring comment = L"");

    size_t mov(Reg a, int b, std::wstring comment = L"");

    size_t add(Reg a, int b, std::wstring comment = L"");

    void sub(Reg a, Reg b, std::wstring comment = L"");

    void push(Reg a, std::wstring comment = L"");

    void pop(Reg a, std::wstring comment = L"");

    // 影响ax，返回值存放在ax中
    template<typename P, typename F>
    void call(const std::wstring &fun, const std::vector<P> &params, F calc_result_to_ax);

    void ret();

    template<typename F>
    void cmp(F f, Reg a, Reg b);

    size_t je(int a);

    void generate(const std::wstring &scope, const TreeNode::Ptr &tree_node, const ScopeSymtabs &scope_symtabs,
                  bool address_only_if_can, bool also_sibling = true);
};

#endif // CODEGENERATOR_H
