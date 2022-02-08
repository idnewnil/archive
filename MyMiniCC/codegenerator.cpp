#include "codegenerator.h"
#include <iomanip>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <functional>

constexpr int LAZY = ~int(unsigned(-1) >> 1);

std::wstring tos(int n) {
    if (n == LAZY) {
        return L"???";
    } else {
        return std::to_wstring(n);
    }
}

using namespace std;

inline bool is_RO(const wchar_t *opcode) {
    static const wchar_t *ro[] = {
            L"HALT", L"IN", L"OUT",
            L"ADD", L"SUB", L"MUL", L"DIV"
    };
    return any_of(begin(ro), end(ro), [&opcode](auto &it) {
        return wcscmp(opcode, it) == 0;
    });
}

CodeGenerator::CodeGenerator(const TreeNode::Ptr &tree_node, const ScopeSymtabs &scope_symtabs) : m_comment_tab(0) {
    comment(L"     ========  ENTRY  ========");
    mov(Reg::SP, 1023);
    call<int>(L"main", {}, [](auto it) {});
    gen(Instruction::HALT());
    comment(L"");

    // 生成input函数
    m_fun_back_patch.emplace(L"input", m_instructions.size());
    begin_comment(L"FunDef: input");
    gen(Instruction::IN(Reg::AX));
    inline_comment(L"input to AX");
    ret();
    end_comment(L"");

    // 生成output函数
    m_fun_back_patch.emplace(L"output", m_instructions.size());
    begin_comment(L"FunDef: output");
    mov(Reg::AX, {Reg::BP, 1});
    gen(Instruction::OUT(Reg::AX));
    inline_comment(L"output AX");
    ret();
    end_comment(L"");

    generate(L"", tree_node, scope_symtabs, false);

    // back patch
    for (auto &instruction: m_instructions) {
        switch (instruction.bp_d.type) {
            case BackPatchType::NONE:
                break;
            case BackPatchType::FUN_ADDRESS:
                instruction.d = m_fun_back_patch.at(instruction.bp_d.fun_name);
                instruction.bp_d.type = BackPatchType::NONE;
                break;
            default:
                assert(false);
        }
    }
}

std::wstring CodeGenerator::to_string() const {
    wstringstream wss;
    for (int i = 0; i < m_instructions.size(); ++i) {
        auto entry = m_comments.find(i); // 寻找第i行的注释
        if (entry != m_comments.end()) {
            for (auto &comment: entry->second) {
                wss << L"* " << comment << endl;
            }
        }
        wss << setw(4) << i << L":"
            << setw(7) << m_instructions[i].opcode
            << setw(4) << L"  " << m_instructions[i].r;
        if (is_RO(m_instructions[i].opcode)) {
            wss << L"," << setw(4) << m_instructions[i].s
                << L"," << setw(4) << m_instructions[i].t << L" ";
        } else {
            wss << L"," << setw(4) << m_instructions[i].d
                << L"(" << setw(4) << m_instructions[i].s << L")";
        }
        auto entry2 = m_inline_comments.find(i);
        if (entry2 != m_inline_comments.end()) {
            wss << L" * " << entry2->second;
        }
        wss << endl;
    }
    return wss.str();
}

size_t CodeGenerator::gen(Instruction instruction) {
    m_instructions.emplace_back(move(instruction));
    return m_instructions.size() - 1;
}

void CodeGenerator::begin_comment(const wstring &ws) {
    m_comments[int(m_instructions.size())].emplace_back(wstring((m_comment_tab++) << 1, L' ') + ws);
}

void CodeGenerator::comment(const wstring &ws) {
    m_comments[int(m_instructions.size())].emplace_back(wstring(m_comment_tab << 1, L' ') + ws);
}

void CodeGenerator::end_comment(const wstring &ws) {
    m_comments[int(m_instructions.size())].emplace_back(wstring((--m_comment_tab) << 1, L' ') + ws);
}

void CodeGenerator::inline_comment(wstring comment, wstring default_comment) {
    if (!comment.empty()) {
        m_inline_comments[int(m_instructions.size() - 1)] = move(comment);
    } else {
        m_inline_comments[int(m_instructions.size() - 1)] = move(default_comment);
    }
}

const wchar_t *look_above = L"...";

size_t CodeGenerator::mova(Reg a, Mem m, wstring comment) {
    size_t res = gen(Instruction::LDA(a, m.s, m.d));
    inline_comment(move(comment), wstring(reg_name(a)) + L" = " + reg_name(m.s) + L" + (" + to_wstring(m.d) + L")");
    return res;
}

void CodeGenerator::mov(Reg a, Reg b, wstring comment) {
    mova(a, {b, 0}, L"");
    inline_comment(move(comment), wstring(reg_name(a)) + L" = " + reg_name(b));
}

size_t CodeGenerator::mov(Reg a, Mem m, wstring comment) {
    size_t res = gen(Instruction::LD(a, m));
    inline_comment(move(comment),
                   wstring(reg_name(a)) + L" = data[" + reg_name(m.s) + L" + (" + to_wstring(m.d) + L")]");
    return res;
}

size_t CodeGenerator::mov(Mem m, Reg a, wstring comment) {
    size_t res = gen(Instruction::ST(a, m));
    inline_comment(move(comment), wstring(L"data[") + reg_name(m.s) + L" + (" + tos(m.d) + L")] = " + reg_name(a));
    return res;
}

size_t CodeGenerator::mov(Reg a, int b, wstring comment) {
    size_t res = gen(Instruction::LDC(a, b));
    inline_comment(move(comment), wstring(reg_name(a)) + L" = " + tos(b));
    return res;
}

size_t CodeGenerator::add(Reg a, int b, wstring comment) {
    size_t res = mova(a, {a, b});
    inline_comment(move(comment), wstring(reg_name(a)) + L" += " + tos(b));
    return res;
}

void CodeGenerator::sub(Reg a, Reg b, wstring comment) {
    gen(Instruction::SUB(a, a, b));
    inline_comment(move(comment), wstring(reg_name(a)) + L" -= " + reg_name(b));
}

void CodeGenerator::push(Reg a, wstring comment) {
    add(Reg::SP, -1);
    inline_comment(move(comment), wstring(L"push ") + reg_name(a));
    mov({Reg::SP, 0}, a, look_above);
}

void CodeGenerator::pop(Reg a, wstring comment) {
    gen(Instruction::LD(a, {Reg::SP, 0}));
    inline_comment(move(comment), wstring(L"pop ") + reg_name(a));
    add(Reg::SP, 1, look_above);
}

template<typename P, typename F>
void CodeGenerator::call(const wstring &fun, const vector<P> &params, F calc_result_to_ax) {
    // 保存基址
    push(Reg::BP);
    // push参数，从最后一个开始push
    for (int i = int(params.size() - 1); i >= 0; --i) {
        calc_result_to_ax(params[i]);
        push(Reg::AX, wstring(L"push param ") + tos(i));
    }
    // push返回地址
    size_t ret_to = mov(Reg::AX, LAZY, L"push return address"); // 返回地址
    push(Reg::AX, look_above);
    // 设置基址
    mov(Reg::BP, Reg::SP);
    // 跳转
    Instruction kall = Instruction::LDC(Reg::IP, LAZY);
    kall.bp_d = BackPatch{BackPatchType::FUN_ADDRESS, fun};
    gen(kall);
    inline_comment(L"call function");
    m_instructions[ret_to].d = int(m_instructions.size());
    // 恢复栈顶
    mov(Reg::SP, Reg::BP);
    // 弹出返回地址和参数
    add(Reg::SP, int(params.size() + 1));
    // 恢复基址
    pop(Reg::BP);
}

void CodeGenerator::ret() {
    mov(Reg::IP, {Reg::BP, 0});
    inline_comment(L"ret");
}

template<typename F>
void CodeGenerator::cmp(F f, Reg a, Reg b) {
    gen(Instruction::SUB(Reg::AX, a, b));
    inline_comment(L"test and jmp");
    size_t test = gen(f(Reg::AX, Reg::IP, LAZY)); // 测试0，为真则跳转到设置1
    inline_comment(look_above);
    mov(Reg::AX, 0, L"set false");
    size_t jmp = mov(Reg::IP, LAZY, L"skip set true"); // 设置为0后跳转
    mov(Reg::AX, 1, L"set true");
    size_t end = m_instructions.size();
    m_instructions[test].d = int(jmp - test);
    m_instructions[jmp].d = int(end);
}

size_t CodeGenerator::je(int a) {
    return gen(Instruction::JEQ(Reg::AX, Reg::IP, a));
}

void
CodeGenerator::generate(const wstring &scope, const TreeNode::Ptr &tree_node, const ScopeSymtabs &scope_symtabs,
                        bool address_only_if_can, bool also_sibling) {
    if (tree_node == nullptr) {
        return;
    } else if (tree_node->type == NodeType::FUN_DECLARATION) {
        // 声明函数
        begin_comment(L"FunDef: " + tree_node->content.value);
        m_fun_back_patch.emplace(tree_node->content.value, m_instructions.size());
        generate(tree_node->content.value, tree_node->children[2], scope_symtabs, false);
        if (!complete_return(tree_node->children[2]) && tree_node->children[0]->content.type == TokenType::VOID) {
            // 确保函数能返回
            ret();
        }
        end_comment(L"");
    } else if (tree_node->type == NodeType::ARRAY_DECLARATION) {
        // 声明数组
    } else if (tree_node->type == NodeType::OTHER && tree_node->content.type == TokenType::ID &&
               !tree_node->children.empty()) {
        // 声明普通变量
    } else if (tree_node->type == NodeType::FUN) {
        // 函数调用
        begin_comment(L"Fun: " + tree_node->content.value);
        vector<TreeNode::Ptr> params;
        if (!none(tree_node->children[0])) {
            for (auto it = tree_node->children[0]; it != nullptr; it = it->sibling) {
                params.push_back(it);
            }
        }
        call(tree_node->content.value, params, [this, &scope, &scope_symtabs](auto it) {
            generate(scope, it, scope_symtabs, false, false);
        });
        end_comment(L"|");
    } else if (tree_node->type == NodeType::ARRAY) {
        // 数组调用
        begin_comment(L"Array: " + tree_node->content.value);
        int offset;
        ScopeType decl_at; // 在哪个作用域声明的
        tie(offset, decl_at) = scope_symtabs.find(scope)->get()->lookup(tree_node->content.value);
        assert(offset != -1);
        wstring comment = L"array address: " + tree_node->content.value;
        if (decl_at == ScopeType::GLOBAL) {
            generate(scope, tree_node->children[0], scope_symtabs, false); // 下标
            push(Reg::BX);
            mov(Reg::BX, Reg::AX);
            mov(Reg::AX, offset);
            sub(Reg::AX, Reg::BX, move(comment));
            pop(Reg::BX);
        } else if (decl_at == ScopeType::FUN) {
            push(Reg::BX);
            mov(Reg::BX, {Reg::BP, 1 + offset}); // 首地址
            generate(scope, tree_node->children[0], scope_symtabs, false); // 下标
            gen(Instruction::SUB(Reg::AX, Reg::BX, Reg::AX));
            inline_comment(move(comment));
            pop(Reg::BX);
        } else {
            push(Reg::BX);
            mova(Reg::BX, {Reg::BP, -1 - offset}); // 首地址
            generate(scope, tree_node->children[0], scope_symtabs, false); // 下标
            gen(Instruction::SUB(Reg::AX, Reg::BX, Reg::AX));
            inline_comment(move(comment));
            pop(Reg::BX);
        }
        if (!address_only_if_can) {
            mov(Reg::AX, {Reg::AX, 0}, L"array value: " + tree_node->content.value);
        }
        end_comment(L"|");
    } else if (tree_node->type == NodeType::OTHER && tree_node->content.type == TokenType::ID) {
        // 普通变量调用
        int offset;
        ScopeType decl_at;
        auto symtab = scope_symtabs.find(scope)->get();
        tie(offset, decl_at) = symtab->lookup(tree_node->content.value);
        assert(offset != -1);
        auto res = symtab->lookup0(tree_node->content.value);
        assert(res != nullptr);
        if (res->type == ItemType::ARRAY) {
            wstring comment = L"array address: " + tree_node->content.value;
            // 应该是数组类型，但以普通变量的方式调用（即函数传参为数组类型）
            if (decl_at == ScopeType::GLOBAL) {
                mov(Reg::AX, offset, move(comment));
            } else if (decl_at == ScopeType::FUN) {
                mov(Reg::AX, {Reg::BP, 1 + offset}, move(comment));
            } else {
                mova(Reg::AX, {Reg::BP, -1 - offset}, move(comment));
            }
        } else {
            // 声明时就是普通类型
            wstring comment1 = L"var address: " + tree_node->content.value;
            wstring comment2 = L"var value: " + tree_node->content.value;
            if (decl_at == ScopeType::GLOBAL) {
                mov(Reg::AX, offset, move(comment1));
                if (!address_only_if_can) {
                    mov(Reg::AX, {Reg::AX, 0}, move(comment2));
                }
            } else if (decl_at == ScopeType::FUN) {
                if (address_only_if_can) {
                    mova(Reg::AX, {Reg::BP, 1 + offset}, move(comment1));
                } else {
                    mov(Reg::AX, {Reg::BP, 1 + offset}, move(comment2));
                }
            } else {
                if (address_only_if_can) {
                    mova(Reg::AX, {Reg::BP, -1 - offset}, move(comment1));
                } else {
                    mov(Reg::AX, {Reg::BP, -1 - offset}, move(comment2));
                }
            }
        }
    } else if (tree_node->type == NodeType::OTHER && tree_node->content.value.find(L"{}") == 0) {
        // 代码段
        // 分配空间
        begin_comment(L"Compoundstmt:");
        add(Reg::SP, -int(scope_symtabs.find(tree_node->content.value)->get()->m_size));
        if (!none(tree_node->children[1])) {
            generate(tree_node->content.value, tree_node->children[1], scope_symtabs, false);
        }
        // 取消分配
        add(Reg::SP, int(scope_symtabs.find(tree_node->content.value)->get()->m_size));
        end_comment(L"|");
    } else if (tree_node->content.type == TokenType::NUM) {
        // 数字
        mov(Reg::AX, stoi(tree_node->content.value), L"num " + tree_node->content.value);
    } else if (tree_node->content.type == TokenType::IF) {
        // if语句
        begin_comment(L"If:");
        comment(L"=>cond");
        generate(scope, tree_node->children[0], scope_symtabs, false);
        // 跳转
        size_t je = this->je(LAZY);
        // then
        comment(L"=>then");
        generate(scope, tree_node->children[1], scope_symtabs, false);
        size_t jmp = mov(Reg::IP, LAZY); // then结束后跳过else
        // else
        m_instructions[je].d = int(jmp - je); // 回填je
        if (!none(tree_node->children[2])) {
            comment(L"=>else");
            generate(scope, tree_node->children[2], scope_symtabs, false);
        }
        size_t end = m_instructions.size();
        m_instructions[jmp].d = int(end); // 回填jmp
        comment(L"=>end");
        end_comment(L"|");
    } else if (tree_node->content.type == TokenType::WHILE) {
        // while语句
        begin_comment(L"While:");
        size_t beg = m_instructions.size();
        comment(L"=>cond");
        generate(scope, tree_node->children[0], scope_symtabs, false);
        size_t je = this->je(LAZY);
        comment(L"=>then");
        generate(scope, tree_node->children[1], scope_symtabs, false);
        mov(Reg::IP, int(beg), L"loop");
        m_instructions[je].d = int(m_instructions.size() - je - 1);
        comment(L"=>end");
        end_comment(L"|");
    } else if (tree_node->content.type == TokenType::RETURN) {
        // return语句
        begin_comment(L"Return:");
        if (!tree_node->children.empty()) {
            generate(scope, tree_node->children[0], scope_symtabs, false);
        }
        ret();
        end_comment(L"|");
    } else if (tree_node->content.type == TokenType::ASSIGN) {
        // 赋值
        // 计算地址
        begin_comment(L"Assign:");
        push(Reg::BX);
        generate(scope, tree_node->children[0], scope_symtabs, true);
        push(Reg::AX);
        // 计算表达式
        generate(scope, tree_node->children[1], scope_symtabs, false);
        pop(Reg::BX);
        gen(Instruction::ST(Reg::AX, {Reg::BX, 0}));
        pop(Reg::BX);
        end_comment(L"|");
    } else if (is_operator(tree_node->content.type)) {
        // 其它二元运算符
        begin_comment(L"Op:");
        push(Reg::BX);
        generate(scope, tree_node->children[0], scope_symtabs, false);
        mov(Reg::BX, Reg::AX);
        generate(scope, tree_node->children[1], scope_symtabs, false);
        switch (tree_node->content.type) {
            case TokenType::ADD:
                gen(Instruction::ADD(Reg::AX, Reg::BX, Reg::AX));
                inline_comment(L"add");
                break;
            case TokenType::SUB:
                gen(Instruction::SUB(Reg::AX, Reg::BX, Reg::AX));
                inline_comment(L"sub");
                break;
            case TokenType::MUL:
                gen(Instruction::MUL(Reg::AX, Reg::BX, Reg::AX));
                inline_comment(L"mul");
                break;
            case TokenType::DIV:
                gen(Instruction::DIV(Reg::AX, Reg::BX, Reg::AX));
                inline_comment(L"div");
                break;
            case TokenType::LT:
                cmp(Instruction::JLT, Reg::BX, Reg::AX);
                break;
            case TokenType::LE:
                cmp(Instruction::JLE, Reg::BX, Reg::AX);
                break;
            case TokenType::GT:
                cmp(Instruction::JGT, Reg::BX, Reg::AX);
                break;
            case TokenType::GE:
                cmp(Instruction::JGE, Reg::BX, Reg::AX);
                break;
            case TokenType::EQ:
                cmp(Instruction::JEQ, Reg::BX, Reg::AX);
                break;
            case TokenType::NE:
                cmp(Instruction::JNE, Reg::BX, Reg::AX);
                break;
            default:
                assert(false);
                break;
        }
        pop(Reg::BX);
        end_comment(L"|");
    } else {
        assert(false);
    }
    if (also_sibling) {
        generate(scope, tree_node->sibling, scope_symtabs, false);
    }
}
