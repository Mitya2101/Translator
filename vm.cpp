#include "vm.h"

#include <iostream>
#include <stdexcept>

PolizVm::PolizVm(const POLIZ& code, const TFunc& funcs)
    : code_(code), funcs_(funcs) {
    frames_.push_back(Frame{0, 0, 0});
}

std::int64_t PolizVm::toI64(const std::string& s) {
    if (s.empty()) return 0;
    return std::stoll(s);
}

char PolizVm::decodeChar(const std::string& s) {
    if (s.empty()) return '\0';
    if (s.size() == 1) return s[0];
    if (s.size() == 2 && s[0] == '\\') {
        switch (s[1]) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '\\': return '\\';
            case '\'': return '\'';
            case '0': return '\0';
            default: return s[1];
        }
    }
    return s[0];
}

std::size_t PolizVm::sizeOf(Types t) const {
    switch (t) {
        case Types::INT: return sizeof(int);
        case Types::DOUBLE: return sizeof(double);
        case Types::BOOL: return sizeof(bool);
        case Types::CHAR: return sizeof(char);
        case Types::VOID: return 0;
    }
    return 0;
}

PolizVm::Value PolizVm::eval(Value v) {
    if (std::holds_alternative<Address>(v)) {
        return readTyped(std::get<Address>(v));
    }
    return v;
}

PolizVm::Address PolizVm::asAddress(const Value& v) {
    if (!std::holds_alternative<Address>(v)) {
        throw std::runtime_error("VM: expected address");
    }
    return std::get<Address>(v);
}

PolizVm::Value PolizVm::readTyped(const Address& a) {
    if (a.abs + sizeOf(a.type) > mem_.size()) throw std::runtime_error("VM: memory read OOB");

    switch (a.type) {
        case Types::INT: {
            int x = readPod<int>(a.abs);
            return static_cast<std::int32_t>(x);
        }
        case Types::DOUBLE: {
            double d = readPod<double>(a.abs);
            return d;
        }
        case Types::BOOL: {
            bool b = readPod<bool>(a.abs);
            return b;
        }
        case Types::CHAR: {
            char c = readPod<char>(a.abs);
            return c;
        }
        case Types::VOID:
            return std::int32_t(0);
    }
    return std::int32_t(0);
}

void PolizVm::writeTyped(const Address& a, const Value& v) {
    if (a.abs + sizeOf(a.type) > mem_.size()) throw std::runtime_error("VM: memory write OOB");

    Value vv = eval(v);

    switch (a.type) {
        case Types::INT: {
            int x = 0;
            if (std::holds_alternative<std::int32_t>(vv)) x = (int)std::get<std::int32_t>(vv);
            else if (std::holds_alternative<bool>(vv)) x = std::get<bool>(vv) ? 1 : 0;
            else if (std::holds_alternative<char>(vv)) x = (int)std::get<char>(vv);
            else if (std::holds_alternative<double>(vv)) x = (int)std::get<double>(vv);
            else throw std::runtime_error("VM: bad type for INT store");
            writePod<int>(a.abs, x);
            return;
        }
        case Types::DOUBLE: {
            double d = 0.0;
            if (std::holds_alternative<double>(vv)) d = std::get<double>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) d = (double)std::get<std::int32_t>(vv);
            else throw std::runtime_error("VM: bad type for DOUBLE store");
            writePod<double>(a.abs, d);
            return;
        }
        case Types::BOOL: {
            bool b = false;
            if (std::holds_alternative<bool>(vv)) b = std::get<bool>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) b = (std::get<std::int32_t>(vv) != 0);
            else throw std::runtime_error("VM: bad type for BOOL store");
            writePod<bool>(a.abs, b);
            return;
        }
        case Types::CHAR: {
            char c = 0;
            if (std::holds_alternative<char>(vv)) c = std::get<char>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) c = (char)std::get<std::int32_t>(vv);
            else throw std::runtime_error("VM: bad type for CHAR store");
            writePod<char>(a.abs, c);
            return;
        }
        case Types::VOID:
            return;
    }
}

std::optional<std::string> PolizVm::findFuncByAddr(std::size_t addr) const {
    for (int i = 0; i < funcs_.GiveSize(); ++i) {
        TFuncElement f = funcs_.Get(i);
        if ((std::size_t)f.GivePolizIndex() == addr) return f.GiveName();
    }
    return std::nullopt;
}

PolizVm::FuncLayout PolizVm::analyzeLayout(std::size_t storedAddr, const TFuncElement& f) const {
    FuncLayout L{};

    auto e0 = code_.GiveEl((int)storedAddr);

    if (e0.first == POLIZ_Element::POLIZ_LABEL) {
        // label, go, allocate, body...
        L.labelIndex = storedAddr;
        L.allocIndex = storedAddr + 2;
        L.bodyStart  = storedAddr + 3;
    } else {
        // ... label, go, allocate, bodyStart(storedAddr)
        L.bodyStart  = storedAddr;
        L.allocIndex = storedAddr - 1;
        L.labelIndex = storedAddr - 3;
    }

    auto label = code_.GiveEl((int)L.labelIndex);
    L.endIp = (std::size_t)toI64(label.second);

    const auto params = f.GiveParam();
    if (!params.empty()) {
        int maxOff = 0;
        for (auto* p : params) maxOff = std::max(maxOff, p->GiveOffset());
        L.paramBytes = (std::size_t)(maxOff + (int)sizeof(int));
    }

    return L;
}

PolizVm::Value PolizVm::callFunction(std::size_t storedAddr, std::vector<Value> args) {
    auto nameOpt = findFuncByAddr(storedAddr);
    if (!nameOpt) throw std::runtime_error("VM: unknown function address " + std::to_string(storedAddr));
    std::string fname = *nameOpt;

    // Your function table stores "name type1 type2 ..." in GiveName().
    // Here we search by exact name.
    int idx = -1;
    for (int i = 0; i < funcs_.GiveSize(); ++i) {
        TFuncElement f = funcs_.Get(i);
        if (f.GiveName() == fname) { idx = i; break; }
    }
    if (idx < 0) throw std::runtime_error("VM: function not found " + fname);

    TFuncElement f = funcs_.Get(idx);
    FuncLayout L = analyzeLayout(storedAddr, f);

    auto allocEl = code_.GiveEl((int)L.allocIndex);
    std::size_t bytes = (std::size_t)toI64(allocEl.second);

    Frame caller = frames_.back();
    Frame callee;
    callee.bp = caller.sp;
    callee.sp = caller.sp;
    callee.paramBytes = L.paramBytes;

    mem_.resize(callee.sp + bytes);
    callee.sp += bytes;

    frames_.push_back(callee);

    auto params = f.GiveParam();
    if (params.size() != args.size()) {
        throw std::runtime_error("VM: bad arg count for " + fname);
    }

    // params are "by reference": in param slots we store int pointer to actual data
    for (std::size_t i = 0; i < params.size(); ++i) {
        int off = params[i]->GiveOffset();
        std::size_t slot = frames_.back().bp + (std::size_t)off;

        if (std::holds_alternative<Address>(args[i])) {
            Address a = std::get<Address>(args[i]);
            int ptr = (int)a.abs;
            writePod<int>(slot, ptr);
        } else {
            // if a value is passed, create a temp cell in callee memory
            Types t = params[i]->GiveType();
            std::size_t tmpAddr = frames_.back().sp;
            mem_.resize(tmpAddr + sizeOf(t));
            frames_.back().sp += sizeOf(t);

            Address tmp{tmpAddr, t};
            writeTyped(tmp, args[i]);

            int ptr = (int)tmpAddr;
            writePod<int>(slot, ptr);
        }
    }

    std::vector<Value> localStack;
    exec(L.bodyStart, L.endIp, localStack);

    Value ret = std::int32_t(0);
    if (!localStack.empty()) ret = eval(localStack.back());

    // destroy callee frame
    std::size_t restore = frames_.back().bp;
    frames_.pop_back();
    mem_.resize(restore);

    return ret;
}

void PolizVm::exec(std::size_t ipBegin, std::size_t ipEnd, std::vector<Value>& stack) {
    std::size_t ip = ipBegin;

    auto asInt = [&](const Value& v)->std::int32_t{
        if (std::holds_alternative<std::int32_t>(v)) return std::get<std::int32_t>(v);
        if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1 : 0;
        if (std::holds_alternative<char>(v)) return (std::int32_t)std::get<char>(v);
        if (std::holds_alternative<double>(v)) return (std::int32_t)std::get<double>(v);
        throw std::runtime_error("VM: int conversion failed");
    };

    auto asDouble = [&](const Value& v)->double{
        if (std::holds_alternative<double>(v)) return std::get<double>(v);
        if (std::holds_alternative<std::int32_t>(v)) return (double)std::get<std::int32_t>(v);
        throw std::runtime_error("VM: double conversion failed");
    };

    while (ip < ipEnd) {
        auto el = code_.GiveEl((int)ip);

        switch (el.first) {
            case POLIZ_Element::INT:
                stack.push_back((std::int32_t)toI64(el.second));
                break;
            case POLIZ_Element::DOUBLE:
                stack.push_back(std::stod(el.second));
                break;
            case POLIZ_Element::BOOL:
                stack.push_back(el.second == "true" || el.second == "1");
                break;
            case POLIZ_Element::CHAR:
                stack.push_back(decodeChar(el.second));
                break;

            case POLIZ_Element::POLIZ_LABEL:
                stack.push_back((std::int32_t)toI64(el.second));
                break;

            case POLIZ_Element::ALLOCATE: {
                std::size_t n = (std::size_t)toI64(el.second);
                Frame& fr = frames_.back();
                mem_.resize(fr.sp + n);
                fr.sp += n;
                break;
            }

            case POLIZ_Element::FREE: {
                std::size_t n = (std::size_t)toI64(el.second);
                Frame& fr = frames_.back();
                if (fr.sp < fr.bp + n) throw std::runtime_error("VM: bad FREE");
                fr.sp -= n;
                mem_.resize(fr.sp);
                break;
            }

            case POLIZ_Element::ADRESS_INT:
            case POLIZ_Element::ADRESS_BOOL:
            case POLIZ_Element::ADRESS_CHAR:
            case POLIZ_Element::ADRESS_DOUBLE: {
                Types t = Types::INT;
                if (el.first == POLIZ_Element::ADRESS_BOOL) t = Types::BOOL;
                else if (el.first == POLIZ_Element::ADRESS_CHAR) t = Types::CHAR;
                else if (el.first == POLIZ_Element::ADRESS_DOUBLE) t = Types::DOUBLE;

                std::int32_t rel = 0;
                if (!el.second.empty()) {
                    rel = (std::int32_t)toI64(el.second);
                } else {
                    Value offv = eval(stack.back());
                    stack.pop_back();
                    rel = asInt(offv);
                }

                Frame& fr = frames_.back();
                std::size_t abs = fr.bp + (std::size_t)rel;

                if ((std::size_t)rel < fr.paramBytes) {
                    int ptr = readPod<int>(abs);
                    abs = (std::size_t)ptr;
                }

                stack.push_back(Address{abs, t});
                break;
            }

            case POLIZ_Element::OPERATION: {
                std::string op = el.second;

                if (op == "=") {
                    Value rhs = eval(stack.back()); stack.pop_back();
                    Address lhs = asAddress(stack.back()); stack.pop_back();
                    writeTyped(lhs, rhs);
                    stack.push_back(rhs);
                    break;
                }

                Value b = eval(stack.back()); stack.pop_back();
                Value a = eval(stack.back()); stack.pop_back();

                bool useDouble = std::holds_alternative<double>(a) || std::holds_alternative<double>(b);

                if (op == "+") {
                    stack.push_back(useDouble ? Value(asDouble(a) + asDouble(b)) : Value(asInt(a) + asInt(b)));
                } else if (op == "-") {
                    stack.push_back(useDouble ? Value(asDouble(a) - asDouble(b)) : Value(asInt(a) - asInt(b)));
                } else if (op == "*") {
                    stack.push_back(useDouble ? Value(asDouble(a) * asDouble(b)) : Value(asInt(a) * asInt(b)));
                } else if (op == "/") {
                    stack.push_back(useDouble ? Value(asDouble(a) / asDouble(b)) : Value(asInt(a) / asInt(b)));
                } else if (op == "%") {
                    stack.push_back(Value(asInt(a) % asInt(b)));
                } else if (op == "==") {
                    stack.push_back(Value(asInt(a) == asInt(b)));
                } else if (op == "!=") {
                    stack.push_back(Value(asInt(a) != asInt(b)));
                } else if (op == "<") {
                    stack.push_back(Value(asInt(a) < asInt(b)));
                } else if (op == ">") {
                    stack.push_back(Value(asInt(a) > asInt(b)));
                } else if (op == "<=") {
                    stack.push_back(Value(asInt(a) <= asInt(b)));
                } else if (op == ">=") {
                    stack.push_back(Value(asInt(a) >= asInt(b)));
                } else if (op == "&&") {
                    stack.push_back(Value(asInt(a) && asInt(b)));
                } else if (op == "||") {
                    stack.push_back(Value(asInt(a) || asInt(b)));
                } else if (op == ",") {
                    stack.push_back(b);
                } else {
                    throw std::runtime_error("VM: unknown op " + op);
                }
                break;
            }

            case POLIZ_Element::UNARY_OPERATION: {
                std::string op = el.second;
                Value a = eval(stack.back()); stack.pop_back();

                if (op == "!") {
                    bool v = false;
                    if (std::holds_alternative<bool>(a)) v = std::get<bool>(a);
                    else v = (asInt(a) != 0);
                    stack.push_back(!v);
                } else if (op == "-") {
                    if (std::holds_alternative<double>(a)) stack.push_back(-std::get<double>(a));
                    else stack.push_back(-asInt(a));
                } else if (op == "+") {
                    stack.push_back(a);
                } else {
                    throw std::runtime_error("VM: unknown unary op " + op);
                }
                break;
            }

            case POLIZ_Element::POLIZ_GO: {
                std::size_t target = 0;
                if (!el.second.empty()) target = (std::size_t)toI64(el.second);
                else {
                    Value v = eval(stack.back()); stack.pop_back();
                    target = (std::size_t)asInt(v);
                }
                ip = target;
                continue;
            }

            case POLIZ_Element::POLIZ_FGO: {
                Value cond = eval(stack.back()); stack.pop_back();
                bool c = false;
                if (std::holds_alternative<bool>(cond)) c = std::get<bool>(cond);
                else c = (asInt(cond) != 0);

                std::size_t target = 0;
                if (!el.second.empty()) target = (std::size_t)toI64(el.second);
                else {
                    Value v = eval(stack.back()); stack.pop_back();
                    target = (std::size_t)asInt(v);
                }

                if (!c) { ip = target; continue; }
                break;
            }

            case POLIZ_Element::FUNCTION_ADRESS:
                stack.push_back((std::int32_t)toI64(el.second));
                break;

            case POLIZ_Element::CALL_FUNCTION: {
                int argc = (int)toI64(el.second);

                std::vector<Value> args(argc);
                for (int i = argc - 1; i >= 0; --i) {
                    args[i] = stack.back();
                    stack.pop_back();
                }

                Value addrV = eval(stack.back()); stack.pop_back();
                std::size_t faddr = (std::size_t)asInt(addrV);

                Value ret = callFunction(faddr, args);
                stack.push_back(ret);
                break;
            }

            case POLIZ_Element::CALL_PRINT: {
                int argc = (int)toI64(el.second);
                std::vector<Value> args(argc);
                for (int i = argc - 1; i >= 0; --i) { args[i] = eval(stack.back()); stack.pop_back(); }

                for (int i = 0; i < argc; ++i) {
                    if (std::holds_alternative<std::int32_t>(args[i])) std::cout << std::get<std::int32_t>(args[i]);
                    else if (std::holds_alternative<double>(args[i])) std::cout << std::get<double>(args[i]);
                    else if (std::holds_alternative<char>(args[i])) std::cout << std::get<char>(args[i]);
                    else if (std::holds_alternative<bool>(args[i])) std::cout << (std::get<bool>(args[i]) ? "true" : "false");
                    else std::cout << "<?>";
                    if (i + 1 < argc) std::cout << " ";
                }
                std::cout << "\n";
                stack.push_back(std::int32_t(0));
                break;
            }

            case POLIZ_Element::CALL_READ: {
                Address a = asAddress(stack.back());
                stack.pop_back();

                if (a.type == Types::INT) {
                    int x; std::cin >> x;
                    writeTyped(a, (std::int32_t)x);
                } else if (a.type == Types::DOUBLE) {
                    double d; std::cin >> d;
                    writeTyped(a, d);
                } else if (a.type == Types::BOOL) {
                    int b; std::cin >> b;
                    writeTyped(a, (bool)(b != 0));
                } else if (a.type == Types::CHAR) {
                    char c; std::cin >> c;
                    writeTyped(a, c);
                }
                stack.push_back(std::int32_t(0));
                break;
            }

            case POLIZ_Element::END_OF_PROGRAM:
                return;

            default:
                // TO_INT/TO_DOUBLE/TO_CHAR/TO_BOOL can be added later if needed
                break;
        }

        ++ip;
    }
}

void PolizVm::run() {
    std::vector<Value> st;
    exec(0, (std::size_t)code_.GiveSize(), st);
}

void PolizVm::runAuto() {
    // if any function name contains "main" as first word, run it
    int mainIdx = -1;
    for (int i = 0; i < funcs_.GiveSize(); ++i) {
        TFuncElement f = funcs_.Get(i);
        const std::string n = f.GiveName();
        if (n == "main" || (n.size() >= 4 && n.substr(0, 4) == "main")) {
            mainIdx = i;
            break;
        }
    }

    if (mainIdx >= 0) {
        TFuncElement f = funcs_.Get(mainIdx);
        std::size_t addr = (std::size_t)f.GivePolizIndex();
        (void)callFunction(addr, {});
        return;
    }

    run();
}
