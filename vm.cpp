#include "vm.h"

#include <iostream>
#include <sstream>
#include <algorithm>

static bool isDigits(const std::string& s) {
    if (s.empty()) return false;
    std::size_t i = 0;
    if (s[0] == '-') i = 1;
    if (i == s.size()) return false;
    for (; i < s.size(); ++i) if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

VKind VM::kindFromTypes(Types t) {
    switch (t) {
        case Types::INT:    return VKind::Int;
        case Types::DOUBLE: return VKind::Double;
        case Types::CHAR:   return VKind::Char;
        case Types::BOOL:   return VKind::Bool;
        case Types::VOID:   return VKind::Void;
    }
    return VKind::Void;
}

VM::VM(const POLIZ& poliz, const TFunc& funcs)
    : poliz_(poliz), funcs_(funcs) {

    // Build function metadata from TFunc + POLIZ layout produced by your Syntaxer:
    // LABEL <end_ip>
    // ... function signature stuff ...
    // ALLOCATE <bytes>
    // FUNCTION_ADRESS <entry_after_alloc>
    // ... body ...
    // FREE <bytes>
    // (end_ip points after FREE)
    for (int i = 0; i < funcs_.GiveSize(); ++i) {
        TFuncElement fe = funcs_.Get(i);

        FuncMeta fm;
        fm.entry_after_alloc = fe.GivePolizIndex();
        fm.alloc_ip = fm.entry_after_alloc - 1;
        fm.ret = fe.GiveReturnValue();

        for (TIDElement* p : fe.GiveParam()) {
            ParamMeta pm;
            pm.offset = p->GiveOffset();
            pm.type = p->GiveType();
            fm.params.push_back(pm);
        }

        // Your Syntaxer places POLIZ_LABEL(end_ip) two items before ALLOCATE in function prologue.
        if (fm.alloc_ip - 2 >= 0) {
            auto lab = poliz_.GiveEl(fm.alloc_ip - 2);
            if (lab.first == POLIZ_Element::POLIZ_LABEL && isDigits(lab.second)) {
                fm.end_ip = std::stoi(lab.second);
            }
        }

        fmeta_[fm.entry_after_alloc] = fm;
    }
}

StackItem VM::pop() {
    if (eval_.empty()) throw RuntimeError("VM stack underflow");
    StackItem x = eval_.back();
    eval_.pop_back();
    return x;
}

void VM::push(StackItem x) {
    eval_.push_back(std::move(x));
}

Value VM::asValue(const StackItem& x) {
    if (std::holds_alternative<Value>(x)) return std::get<Value>(x);
    const auto addr = std::get<Address>(x).offset;
    return loadAt(addr);
}

Address VM::asAddress(const StackItem& x) {
    if (std::holds_alternative<Address>(x)) return std::get<Address>(x);
    const Value v = std::get<Value>(x);
    if (v.kind == VKind::Int) return Address{std::get<std::int64_t>(v.data)};
    throw RuntimeError("Expected address, got non-address value");
}

static char decodeEscapedChar(const std::string& s) {
    if (s.size() == 1) return s[0];
    if (s.size() == 2 && s[0] == '\\') {
        switch (s[1]) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            default: return s[1];
        }
    }
    return s.empty() ? '\0' : s[0];
}

Value VM::parseLiteral(const std::string& s) {
    // Heuristic:
    // - contains '.' => double
    // - all digits (possibly with leading '-') => int
    // - short (<=2) => char (including escaped like "\n")
    // - otherwise => string
    if (s.find('.') != std::string::npos) {
        double d = 0.0;
        try { d = std::stod(s); } catch (...) { throw RuntimeError("Bad double literal: " + s); }
        return Value::makeDouble(d);
    }

    if (isDigits(s)) {
        std::int64_t x = 0;
        try { x = std::stoll(s); } catch (...) { throw RuntimeError("Bad int literal: " + s); }
        return Value::makeInt(x);
    }

    if (!s.empty() && s.size() <= 2) {
        return Value::makeChar(decodeEscapedChar(s));
    }

    return Value::makeString(s);
}

VM::Cell& VM::ensureCell(std::int64_t addr) {
    return mem_[addr];
}

const VM::Cell& VM::getCell(std::int64_t addr) const {
    auto it = mem_.find(addr);
    if (it == mem_.end()) throw RuntimeError("Read from missing cell at " + std::to_string(addr));
    return it->second;
}

Value VM::loadAt(std::int64_t addr) {
    if (addr < 0) throw RuntimeError("Negative address");
    if (addr >= sp_) throw RuntimeError("Read out of allocated memory: addr=" + std::to_string(addr));

    const Cell& c = getCell(addr);
    if (c.is_ref) return loadAt(c.ref_target);

    if (!c.initialized)
        throw RuntimeError("Read from uninitialized variable at " + std::to_string(addr));

    return Value{c.kind, c.data};
}

void VM::storeAt(std::int64_t addr, const Value& v) {
    if (addr < 0) throw RuntimeError("Negative address");
    if (addr >= sp_) throw RuntimeError("Write out of allocated memory: addr=" + std::to_string(addr));

    Cell& c = ensureCell(addr);

    if (c.is_ref) {
        storeAt(c.ref_target, v);
        return;
    }

    if (!c.initialized) c.kind = v.kind;
    else if (c.kind != v.kind) throw RuntimeError("Type mismatch store at " + std::to_string(addr));

    c.data = v.data;
    c.initialized = true;
}

void VM::execUnary(const std::string& op) {
    Value a = asValue(pop());

    if (op == "!") {
        if (a.kind != VKind::Bool) throw RuntimeError("Unary ! expects bool");
        push(Value::makeBool(!std::get<bool>(a.data)));
        return;
    }
    if (op == "-") {
        if (a.kind == VKind::Int) { push(Value::makeInt(-std::get<std::int64_t>(a.data))); return; }
        if (a.kind == VKind::Double) { push(Value::makeDouble(-std::get<double>(a.data))); return; }
        throw RuntimeError("Unary - expects int/double");
    }
    if (op == "+") {
        if (a.kind == VKind::Int || a.kind == VKind::Double) { push(a); return; }
        throw RuntimeError("Unary + expects int/double");
    }

    throw RuntimeError("Unknown unary op: " + op);
}

void VM::execOperation(const std::string& op) {
    if (op == "=") {
        Value rhs = asValue(pop());
        Address lhs = asAddress(pop());
        storeAt(lhs.offset, rhs);
        push(rhs); // assignment expression yields rhs
        return;
    }

    if (op == "&&" || op == "||") {
        Value b = asValue(pop());
        Value a = asValue(pop());
        if (a.kind != VKind::Bool || b.kind != VKind::Bool)
            throw RuntimeError("Logical op expects bool");
        bool av = std::get<bool>(a.data);
        bool bv = std::get<bool>(b.data);
        push(Value::makeBool(op == "&&" ? (av && bv) : (av || bv)));
        return;
    }

    auto do_cmp = [&](auto aa, auto bb) -> Value {
        bool res = false;
        if (op == "==") res = (aa == bb);
        else if (op == "!=") res = (aa != bb);
        else if (op == "<") res = (aa < bb);
        else if (op == "<=") res = (aa <= bb);
        else if (op == ">") res = (aa > bb);
        else if (op == ">=") res = (aa >= bb);
        else throw RuntimeError("Unknown cmp op: " + op);
        return Value::makeBool(res);
    };

    if (op == "==" || op == "!=") {
        Value b = asValue(pop());
        Value a = asValue(pop());
        if (a.kind != b.kind) throw RuntimeError("==/!= require same types");
        switch (a.kind) {
            case VKind::Int:    push(do_cmp(std::get<std::int64_t>(a.data), std::get<std::int64_t>(b.data))); return;
            case VKind::Double: push(do_cmp(std::get<double>(a.data), std::get<double>(b.data))); return;
            case VKind::Bool:   push(do_cmp(std::get<bool>(a.data), std::get<bool>(b.data))); return;
            case VKind::Char:   push(do_cmp(std::get<char>(a.data), std::get<char>(b.data))); return;
            case VKind::String: push(do_cmp(std::get<std::string>(a.data), std::get<std::string>(b.data))); return;
            default: throw RuntimeError("Bad type for ==");
        }
    }

    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        Value b = asValue(pop());
        Value a = asValue(pop());
        if (a.kind != b.kind) throw RuntimeError("Relational ops require same types");
        if (a.kind == VKind::Int) { push(do_cmp(std::get<std::int64_t>(a.data), std::get<std::int64_t>(b.data))); return; }
        if (a.kind == VKind::Double) { push(do_cmp(std::get<double>(a.data), std::get<double>(b.data))); return; }
        if (a.kind == VKind::Char) { push(do_cmp(std::get<char>(a.data), std::get<char>(b.data))); return; }
        throw RuntimeError("Relational ops expect int/double/char");
    }

    Value b = asValue(pop());
    Value a = asValue(pop());
    if (a.kind != b.kind) throw RuntimeError("Binary op requires same types");

    if (a.kind == VKind::Int) {
        std::int64_t av = std::get<std::int64_t>(a.data);
        std::int64_t bv = std::get<std::int64_t>(b.data);
        if (op == "+") push(Value::makeInt(av + bv));
        else if (op == "-") push(Value::makeInt(av - bv));
        else if (op == "*") push(Value::makeInt(av * bv));
        else if (op == "/") { if (bv == 0) throw RuntimeError("Division by zero"); push(Value::makeInt(av / bv)); }
        else if (op == "%") { if (bv == 0) throw RuntimeError("Modulo by zero"); push(Value::makeInt(av % bv)); }
        else throw RuntimeError("Unknown int op: " + op);
        return;
    }

    if (a.kind == VKind::Double) {
        double av = std::get<double>(a.data);
        double bv = std::get<double>(b.data);
        if (op == "+") push(Value::makeDouble(av + bv));
        else if (op == "-") push(Value::makeDouble(av - bv));
        else if (op == "*") push(Value::makeDouble(av * bv));
        else if (op == "/") push(Value::makeDouble(av / bv));
        else throw RuntimeError("Unknown double op: " + op);
        return;
    }

    throw RuntimeError("Operator '" + op + "' is not defined for this type");
}

void VM::execCall() {
    // Stack convention: push args (addresses), then FUNCTION_ADRESS value, then CALL_FUNCTION
    Value f = asValue(pop());
    if (f.kind != VKind::Int) throw RuntimeError("FUNCTION_ADRESS must be int poliz index");
    int entry = (int)std::get<std::int64_t>(f.data);

    auto it = fmeta_.find(entry);
    if (it == fmeta_.end()) throw RuntimeError("Unknown function entry: " + std::to_string(entry));
    const FuncMeta& fm = it->second;

    const int argc = (int)fm.params.size();
    std::vector<Address> args(argc);
    for (int i = argc - 1; i >= 0; --i) args[i] = asAddress(pop());

    auto al = poliz_.GiveEl(fm.alloc_ip);
    if (al.first != POLIZ_Element::ALLOCATE || !isDigits(al.second))
        throw RuntimeError("Function prologue is not ALLOCATE");

    std::int64_t alloc_bytes = std::stoll(al.second);

    Frame fr;
    fr.return_ip = ip_ + 1;
    fr.end_ip = fm.end_ip;
    fr.sp_base = sp_;
    fr.eval_base = eval_.size();
    fr.ret = fm.ret;
    call_stack_.push_back(fr);

    sp_ += alloc_bytes;

    // Bind params as reference cells
    for (int i = 0; i < argc; ++i) {
        const ParamMeta& pm = fm.params[i];
        Cell& c = ensureCell(pm.offset);
        c.is_ref = true;
        c.ref_target = args[i].offset;
        c.kind = kindFromTypes(pm.type);
        c.initialized = true;
    }

    ip_ = fm.entry_after_alloc;
}

void VM::maybeReturnFromFunction() {
    if (call_stack_.empty()) return;
    const Frame& fr = call_stack_.back();
    if (ip_ != fr.end_ip) return;

    Value retv = Value::makeVoid();
    if (fr.ret != Types::VOID) {
        if (eval_.empty()) throw RuntimeError("Missing return value on stack");
        retv = asValue(pop());
    }

    while (eval_.size() > fr.eval_base) eval_.pop_back();
    sp_ = fr.sp_base;

    int rip = fr.return_ip;
    Types rt = fr.ret;
    call_stack_.pop_back();

    if (rt != Types::VOID) push(retv);
    ip_ = rip;
}

void VM::run() {
    ip_ = 0;
    sp_ = 0;
    eval_.clear();
    call_stack_.clear();
    mem_.clear();

    while (ip_ >= 0 && ip_ < poliz_.GiveSize()) {
        auto cur = poliz_.GiveEl(ip_);
        auto kind = cur.first;
        const std::string& arg = cur.second;

        if (trace_) {
            std::cerr << "[ip=" << ip_ << "] kind=" << (int)kind << " arg='" << arg << "'\n";
        }

        switch (kind) {
            case POLIZ_Element::NUM:
                push(parseLiteral(arg));
                ++ip_;
                break;

            case POLIZ_Element::SYMBOL:
                // IMPORTANT: SYMBOL must represent an address/offset (NOT a numeric literal).
                push(Address{std::stoll(arg)});
                ++ip_;
                break;

            case POLIZ_Element::POLIZ_LABEL:
                push(Value::makeInt(std::stoll(arg)));
                ++ip_;
                break;

            case POLIZ_Element::POLIZ_GO: {
                Value target = asValue(pop());
                ip_ = (int)std::get<std::int64_t>(target.data);
                break;
            }

            case POLIZ_Element::POLIZ_FGO: {
                Value target = asValue(pop());
                Value cond = asValue(pop());
                if (cond.kind != VKind::Bool) throw RuntimeError("FGO expects bool");
                if (!std::get<bool>(cond.data)) ip_ = (int)std::get<std::int64_t>(target.data);
                else ++ip_;
                break;
            }

            case POLIZ_Element::ALLOCATE:
                sp_ += std::stoll(arg);
                ++ip_;
                break;

            case POLIZ_Element::FREE: {
                sp_ -= std::stoll(arg);
                for (auto it = mem_.begin(); it != mem_.end(); )
                    if (it->first >= sp_) it = mem_.erase(it);
                    else ++it;
                ++ip_;
                break;
            }

            case POLIZ_Element::FUNCTION_ADRESS:
                push(Value::makeInt(std::stoll(arg)));
                ++ip_;
                break;

            case POLIZ_Element::CALL_FUNCTION:
                execCall();
                break;

            case POLIZ_Element::CALL_ARRAY: {
                // Convention: CALL_ARRAY expects the top of stack is an int address
                Value off = asValue(pop());
                if (off.kind != VKind::Int) throw RuntimeError("CALL_ARRAY expects int");
                push(Address{std::get<std::int64_t>(off.data)});
                ++ip_;
                break;
            }

            case POLIZ_Element::UNARY_OPERATION:
                execUnary(arg);
                ++ip_;
                break;

            case POLIZ_Element::OPERATION:
                if (arg == ",") {
                    Value right = asValue(pop());
                    (void)asValue(pop());
                    push(right);
                    ++ip_;
                    break;
                }
                execOperation(arg);
                ++ip_;
                break;

            case POLIZ_Element::END_OF_PROGRAM:
                return;

            default:
                throw RuntimeError("Unknown POLIZ element");
        }

        maybeReturnFromFunction();
    }
}
