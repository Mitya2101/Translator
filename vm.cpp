#include "vm.h"

#include <cstring>
#include <iostream>
#include <stdexcept>



PolizVm::PolizVm(POLIZ& code, TFunc& funcs)
    : code_(code), funcs_(funcs) {
    frames_.push_back(Frame{0, 0, {}});
}

std::int64_t PolizVm::toI64(const std::string& s) {
    if (s.empty()) return 0;
    return std::stoll(s);
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

PolizVm::Value PolizVm::eval(Value v) {
    if (std::holds_alternative<Address>(v)) {
        return readTyped(std::get<Address>(v));
    }
    return v;
}

PolizVm::Address PolizVm::asAddress(const Value& v) {
    if (!std::holds_alternative<Address>(v)) {
        throw std::runtime_error("Expected address on stack");
    }
    return std::get<Address>(v);
}

PolizVm::Value PolizVm::readTyped(const Address& a) {
    const std::size_t need = a.abs + sizeOf(a.type);
    if (need > mem_.size()) {
        throw std::runtime_error(
            "Memory read OOB (abs=" + std::to_string(a.abs) +
            ", type=" + std::to_string((int)a.type) +
            ", need=" + std::to_string(need) +
            ", mem=" + std::to_string(mem_.size()) + ")");
    }

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
    if (a.abs + sizeOf(a.type) > mem_.size()) throw std::runtime_error("Memory write OOB");

    Value vv = eval(v);

    switch (a.type) {
        case Types::INT: {
            int x = 0;
            if (std::holds_alternative<std::int32_t>(vv)) x = (int)std::get<std::int32_t>(vv);
            else if (std::holds_alternative<bool>(vv)) x = std::get<bool>(vv) ? 1 : 0;
            else if (std::holds_alternative<char>(vv)) x = (int)std::get<char>(vv);
            else if (std::holds_alternative<double>(vv)) x = (int)std::get<double>(vv);
            else throw std::runtime_error("Bad type for INT store");
            writePod<int>(a.abs, x);
            return;
        }
        case Types::DOUBLE: {
            double d = 0.0;
            if (std::holds_alternative<double>(vv)) d = std::get<double>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) d = (double)std::get<std::int32_t>(vv);
            else throw std::runtime_error("Bad type for DOUBLE store");
            writePod<double>(a.abs, d);
            return;
        }
        case Types::BOOL: {
            bool b = false;
            if (std::holds_alternative<bool>(vv)) b = std::get<bool>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) b = (std::get<std::int32_t>(vv) != 0);
            else throw std::runtime_error("Bad type for BOOL store");
            writePod<bool>(a.abs, b);
            return;
        }
        case Types::CHAR: {
            char c = 0;
            if (std::holds_alternative<char>(vv)) c = std::get<char>(vv);
            else if (std::holds_alternative<std::int32_t>(vv)) c = (char)std::get<std::int32_t>(vv);
            else throw std::runtime_error("Bad type for CHAR store");
            writePod<char>(a.abs, c);
            return;
        }
        case Types::VOID:
            return;
    }
}

std::optional<std::string> PolizVm::findFuncByAddr(std::size_t addr) {
    for (int i = 0; i < funcs_.GiveSize(); ++i) {
        auto f = funcs_.Get(i);
        if ((std::size_t)f.GivePolizIndex() == addr) return f.GiveName();
    }
    return std::nullopt;
}

PolizVm::FuncLayout PolizVm::analyzeLayout(std::size_t storedAddr, TFuncElement f) {
    FuncLayout L{};

    // Два варианта адреса:
    // 1) storedAddr указывает на POLIZ_LABEL (перед телом функции)
    // 2) storedAddr указывает на первую инструкцию тела (после ALLOCATE)

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

    // Parameters: in this project scalars are stored by VALUE, arrays are stored as pointers.
    // Remember which parameter offsets are pointers so ADRESS_* can dereference only those.
    const auto& params = f.GiveParam();
    for (auto* p : params) {
        const int off = p->GiveOffset();
        // Array parameters are represented by TIDElementArray<...>
        if (dynamic_cast<TIDElementArray<int>*>(p) ||
            dynamic_cast<TIDElementArray<double>*>(p) ||
            dynamic_cast<TIDElementArray<bool>*>(p) ||
            dynamic_cast<TIDElementArray<char>*>(p)) {
            L.ptrParamOffsets.push_back((std::size_t)off);
        }
    }

    return L;
}

PolizVm::Value PolizVm::callFunction(std::size_t storedAddr, std::vector<Value> args) {
    auto nameOpt = findFuncByAddr(storedAddr);
    if (!nameOpt) throw std::runtime_error("Unknown function address: " + std::to_string(storedAddr));
    std::string fname = *nameOpt;

    if (!funcs_.Find(fname)) {
        throw std::runtime_error("Unknown function: " + fname);
    }
    auto f = funcs_.Get(fname);

    FuncLayout L = analyzeLayout(storedAddr, f);
    auto allocEl = code_.GiveEl((int)L.allocIndex);
    std::size_t bytes = (std::size_t)toI64(allocEl.second);

    // (debug prints removed)

    // IMPORTANT:
    // In this project, the compiler may emit ALLOCATE size that does NOT include
    // the space needed for function parameters (only locals). But parameter offsets
    // are absolute within the frame (starting from 0), so we must ensure the frame
    // is at least large enough to store all parameters.
    std::size_t paramBytesNeed = 0;
    {
        const auto& params = f.GiveParam();
        for (auto* p : params) {
            const int off = p->GiveOffset();
            if (off < 0) continue;
            std::size_t sz = sizeOf(p->GiveType());
            // Array parameters are stored as pointers (int) in the frame.
            if (dynamic_cast<TIDElementArray<int>*>(p) ||
                dynamic_cast<TIDElementArray<double>*>(p) ||
                dynamic_cast<TIDElementArray<bool>*>(p) ||
                dynamic_cast<TIDElementArray<char>*>(p)) {
                sz = sizeof(int);
            }
            paramBytesNeed = std::max(paramBytesNeed, (std::size_t)off + sz);
        }
    }
    if (bytes < paramBytesNeed) bytes = paramBytesNeed;

    Frame& caller = frames_.back();

    Frame callee;
    callee.bp = caller.sp;
    callee.sp = caller.sp;
    callee.ptrParamOffsets = L.ptrParamOffsets;

    mem_.resize(callee.sp + bytes);
    callee.sp += bytes;

    // кладём frame
    frames_.push_back(callee);

    // записываем параметры:
    // - scalar params: by VALUE
    // - array params: as POINTER (int) to caller memory
    const auto& params = f.GiveParam();
    if (params.size() != args.size()) {
        throw std::runtime_error("Bad arg count for function " + fname);
    }

    for (std::size_t i = 0; i < params.size(); ++i) {
        int off = params[i]->GiveOffset();
        std::size_t slot = frames_.back().bp + (std::size_t)off;

        const bool isPtrParam = (dynamic_cast<TIDElementArray<int>*>(params[i]) ||
                                 dynamic_cast<TIDElementArray<double>*>(params[i]) ||
                                 dynamic_cast<TIDElementArray<bool>*>(params[i]) ||
                                 dynamic_cast<TIDElementArray<char>*>(params[i]));

        if (isPtrParam) {
            // Expect an address (base of array). Store pointer as int.
            if (!std::holds_alternative<Address>(args[i])) {
                throw std::runtime_error("Array parameter expects address in function " + fname);
            }
            auto a = std::get<Address>(args[i]);
            int ptr = (int)a.abs;
            writePod<int>(slot, ptr);
        } else {
            // Scalar: store by value directly into the frame slot.
            Address dst{slot, params[i]->GiveType()};
            writeTyped(dst, args[i]);
        }
    }

    std::vector<Value> localStack;
    // The compiler emits a final FREE at the end of each function and stores
    // the end label after that FREE. We manage frames ourselves in the VM,
    // so executing that last FREE would shrink the frame memory before we
    // can read the return value (leading to OOB / heap corruption symptoms).
    // Therefore, we stop right before the final instruction.
    std::size_t fnEnd = L.endIp;
    if (fnEnd > L.bodyStart) fnEnd -= 1;
    exec(L.bodyStart, fnEnd, localStack);

    Value ret = std::int32_t(0);
    if (!localStack.empty()) {
        ret = eval(localStack.back());
    }

    // гарантированно очищаем память функции
    mem_.resize(frames_.back().bp);
    frames_.pop_back();

    return ret;
}

void PolizVm::exec(std::size_t ipBegin, std::size_t ipEnd, std::vector<Value>& stack) {
    std::size_t ip = ipBegin;
    // std::cout<<ipBegin<<' '<<ipEnd<<std::endl;

    // Debug helpers (local): show what types are currently on the VM stack when it crashes.
    auto valueTypeName = [](const Value& v) -> const char* {
        if (std::holds_alternative<std::int32_t>(v)) return "int";
        if (std::holds_alternative<double>(v)) return "double";
        if (std::holds_alternative<char>(v)) return "char";
        if (std::holds_alternative<bool>(v)) return "bool";
        if (std::holds_alternative<std::string>(v)) return "string";
        if (std::holds_alternative<Address>(v)) return "address";
        return "?";
    };
    auto stackTypesDump = [&](std::size_t maxN = 8) -> std::string {
        std::string out;
        out += "[";
        std::size_t n = stack.size();
        std::size_t begin = (n > maxN) ? (n - maxN) : 0;
        if (begin != 0) out += "..., ";
        for (std::size_t i = begin; i < n; ++i) {
            if (i != begin) out += ", ";
            out += valueTypeName(stack[i]);
        }
        out += "]";
        return out;
    };

    while (ip < ipEnd) {
        // std::cout<<ip<<std::endl;
        auto el = code_.GiveEl((int)ip);

        try {
            switch (el.first) {
            case POLIZ_Element::END_FUNCTION:
                return;
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
                if (fr.sp < fr.bp + n) throw std::runtime_error("Bad FREE");
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
                    if (!std::holds_alternative<std::int32_t>(offv)) throw std::runtime_error("Offset must be int");
                    rel = std::get<std::int32_t>(offv);
                }

                Frame& fr = frames_.back();
                std::size_t abs = fr.bp + (std::size_t)rel;

                // If this offset corresponds to an array parameter, the slot stores a pointer (int)
                // to caller memory. Only in this case we dereference.
                for (std::size_t po : fr.ptrParamOffsets) {
                    if (po == (std::size_t)rel) {
                        int ptr = readPod<int>(abs);
                        abs = (std::size_t)ptr;
                        break;
                    }
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

                auto getInt = [](const Value& v)->std::int32_t{
                    if (std::holds_alternative<std::int32_t>(v)) return std::get<std::int32_t>(v);
                    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1 : 0;
                    if (std::holds_alternative<char>(v)) return (std::int32_t)std::get<char>(v);
                    if (std::holds_alternative<double>(v)) return (std::int32_t)std::get<double>(v);
                    throw std::runtime_error("Bad int conversion");
                };

                auto getDouble = [](const Value& v)->double{
                    if (std::holds_alternative<double>(v)) return std::get<double>(v);
                    if (std::holds_alternative<std::int32_t>(v)) return (double)std::get<std::int32_t>(v);
                    throw std::runtime_error("Bad double conversion");
                };

                bool useDouble = std::holds_alternative<double>(a) || std::holds_alternative<double>(b);

                if (op == "+") {
                    if (useDouble) stack.push_back(getDouble(a) + getDouble(b));
                    else stack.push_back(getInt(a) + getInt(b));
                } else if (op == "-") {
                    if (useDouble) stack.push_back(getDouble(a) - getDouble(b));
                    else stack.push_back(getInt(a) - getInt(b));
                } else if (op == "*") {
                    if (useDouble) stack.push_back(getDouble(a) * getDouble(b));
                    else stack.push_back(getInt(a) * getInt(b));
                } else if (op == "/") {
                    if (useDouble) stack.push_back(getDouble(a) / getDouble(b));
                    else stack.push_back(getInt(a) / getInt(b));
                } else if (op == "%") {
                    stack.push_back(getInt(a) % getInt(b));
                } else if (op == "==") {
                    stack.push_back(getInt(a) == getInt(b));
                } else if (op == "!=") {
                    stack.push_back(getInt(a) != getInt(b));
                } else if (op == "<") {
                    stack.push_back(getInt(a) < getInt(b));
                } else if (op == ">") {
                    stack.push_back(getInt(a) > getInt(b));
                } else if (op == "<=") {
                    stack.push_back(getInt(a) <= getInt(b));
                } else if (op == ">=") {
                    stack.push_back(getInt(a) >= getInt(b));
                } else if (op == "&&") {
                    stack.push_back(getInt(a) && getInt(b));
                } else if (op == "||") {
                    stack.push_back(getInt(a) || getInt(b));
                } else if (op == ",") {
                    stack.push_back(b);
                } else {
                    throw std::runtime_error("Unknown op: " + op);
                }
                break;
            }

            case POLIZ_Element::UNARY_OPERATION: {
                std::string op = el.second;
                Value a = eval(stack.back()); stack.pop_back();

                auto getInt = [](const Value& v)->std::int32_t{
                    if (std::holds_alternative<std::int32_t>(v)) return std::get<std::int32_t>(v);
                    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? 1 : 0;
                    if (std::holds_alternative<char>(v)) return (std::int32_t)std::get<char>(v);
                    if (std::holds_alternative<double>(v)) return (std::int32_t)std::get<double>(v);
                    throw std::runtime_error("Bad int conversion");
                };

                if (op == "!") {
                    bool v = false;
                    if (std::holds_alternative<bool>(a)) v = std::get<bool>(a);
                    else if (std::holds_alternative<std::int32_t>(a)) v = std::get<std::int32_t>(a) != 0;
                    else throw std::runtime_error("Bad unary !");
                    stack.push_back(!v);
                } else if (op == "-") {
                    if (std::holds_alternative<double>(a)) stack.push_back(-std::get<double>(a));
                    else stack.push_back(-getInt(a));
                } else if (op == "+") {
                    stack.push_back(a);
                } else {
                    throw std::runtime_error("Unknown unary op");
                }
                break;
            }

            case POLIZ_Element::POLIZ_GO: {
                std::size_t target;
                if (!el.second.empty()) target = (std::size_t)toI64(el.second);
                else {
                    Value v = eval(stack.back()); stack.pop_back();
                    // label может приходить как int/bool/char/double (или адрес на int)
                    auto asIp = [](const Value& vv) -> std::size_t {
                        if (std::holds_alternative<std::int32_t>(vv)) return (std::size_t)std::get<std::int32_t>(vv);
                        if (std::holds_alternative<bool>(vv)) return (std::size_t)(std::get<bool>(vv) ? 1 : 0);
                        if (std::holds_alternative<char>(vv)) return (std::size_t)(unsigned char)std::get<char>(vv);
                        if (std::holds_alternative<double>(vv)) return (std::size_t)std::get<double>(vv);
                        if (std::holds_alternative<std::string>(vv)) return (std::size_t)PolizVm::toI64(std::get<std::string>(vv));
                        throw std::runtime_error("GO expects label");
                    };
                    target = asIp(v);
                }
                ip = target;
                continue;
            }

            case POLIZ_Element::POLIZ_FGO: {
                // В ПОЛИЗЕ условного перехода у вас порядок такой:
                //   <cond_expr> POLIZ_LABEL <target> POLIZ_FGO
                // т.е. на стеке сверху лежит label, под ним условие.

                auto asIp = [](const Value& vv) -> std::size_t {
                    if (std::holds_alternative<std::int32_t>(vv)) return (std::size_t)std::get<std::int32_t>(vv);
                    if (std::holds_alternative<bool>(vv)) return (std::size_t)(std::get<bool>(vv) ? 1 : 0);
                    if (std::holds_alternative<char>(vv)) return (std::size_t)(unsigned char)std::get<char>(vv);
                    if (std::holds_alternative<double>(vv)) return (std::size_t)std::get<double>(vv);
                    if (std::holds_alternative<std::string>(vv)) return (std::size_t)PolizVm::toI64(std::get<std::string>(vv));
                    throw std::runtime_error("FGO expects label");
                };

                std::size_t target = 0;
                if (!el.second.empty()) {
                    // Переход задан в аргументе операции, на стеке только условие.
                    target = (std::size_t)toI64(el.second);

                    Value condV = eval(stack.back());
                    stack.pop_back();
                    bool c = false;
                    if (std::holds_alternative<bool>(condV)) c = std::get<bool>(condV);
                    else if (std::holds_alternative<std::int32_t>(condV)) c = (std::get<std::int32_t>(condV) != 0);
                    else if (std::holds_alternative<char>(condV)) c = (std::get<char>(condV) != 0);
                    else if (std::holds_alternative<double>(condV)) c = (std::get<double>(condV) != 0.0);
                    else throw std::runtime_error("Bad FGO condition type");

                    if (!c) { ip = target; continue; }
                    break;
                }

                // el.second пустой: сначала снимаем label, затем условие.
                Value labelV = eval(stack.back());
                stack.pop_back();
                target = asIp(labelV);

                Value condV = eval(stack.back());
                stack.pop_back();

                bool c = false;
                if (std::holds_alternative<bool>(condV)) c = std::get<bool>(condV);
                else if (std::holds_alternative<std::int32_t>(condV)) c = (std::get<std::int32_t>(condV) != 0);
                else if (std::holds_alternative<char>(condV)) c = (std::get<char>(condV) != 0);
                else if (std::holds_alternative<double>(condV)) c = (std::get<double>(condV) != 0.0);
                else throw std::runtime_error("Bad FGO condition type");

                if (!c) { ip = target; continue; }
                break;
            }

            case POLIZ_Element::FUNCTION_ADRESS:
                stack.push_back((std::int32_t)toI64(el.second));
                break;

            case POLIZ_Element::CALL_FUNCTION: {
                // Синтаксер НЕ записывает количество аргументов в el.second.
                // В POLIZ лежит: <args...> FUNCTION_ADRESS <addr> CALL_FUNCTION
                // Поэтому читаем адрес функции со стека, затем узнаём argc из таблицы функций.
                Value addrV = eval(stack.back());
                stack.pop_back();
                if (!std::holds_alternative<std::int32_t>(addrV)) throw std::runtime_error("Bad function address");
                std::size_t faddr = (std::size_t)std::get<std::int32_t>(addrV);

                auto nameOpt = findFuncByAddr(faddr);
                if (!nameOpt) throw std::runtime_error("Unknown function address: " + std::to_string(faddr));
                auto f = funcs_.Get(*nameOpt);

                int argc = (int)f.GiveParam().size();
                std::vector<Value> args(argc);
                for (int i = argc - 1; i >= 0; --i) {
                    args[i] = eval(stack.back());
                    stack.pop_back();
                }

                Value ret = callFunction(faddr, args);
                stack.push_back(ret);
                break;
            }

            case POLIZ_Element::CALL_PRINT: {
                // CALL_PRINT генерируется синтаксером для идентификатора print(...)
                // и ВСЕГДА имеет ровно один аргумент (по таблице встроенных функций).
                Value v = eval(stack.back());
                stack.pop_back();

                if (std::holds_alternative<std::int32_t>(v)) std::cout << std::get<std::int32_t>(v);
                else if (std::holds_alternative<double>(v)) std::cout << std::get<double>(v);
                else if (std::holds_alternative<char>(v)) std::cout << std::get<char>(v);
                else if (std::holds_alternative<bool>(v)) std::cout << (std::get<bool>(v) ? "true" : "false");
                else if (std::holds_alternative<std::string>(v)) std::cout << std::get<std::string>(v);
                else std::cout << "<?>";

                std::cout << "\n";
                // void-значение
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
                // TO_INT/TO_DOUBLE/TO_CHAR/TO_BOOL пока можно не реализовывать,
                // если синтаксер уже приводит типы заранее.
                break;
            }
        }
        catch (const std::bad_variant_access&) {
            std::cerr << "VM bad_variant_access at ip=" << ip
                      << " op=" << (int)el.first << " arg=\"" << el.second << "\""
                      << " stack=" << stackTypesDump() << "\n";
            throw;
        }
        catch (const std::exception& e) {
            std::cerr << "VM exception at ip=" << ip
                      << " op=" << (int)el.first << " arg=\"" << el.second << "\""
                      << " stack=" << stackTypesDump() << " : " << e.what() << "\n";
            throw;
        }

        ++ip;
    }
}

void PolizVm::run() {
    std::vector<Value> st;
    exec(0, (std::size_t)code_.GiveSize(), st);
}

void PolizVm::runAuto() {
    // если main существует — вызовем её
    if (funcs_.Find("main")) {
        auto f = funcs_.Get("main");
        std::size_t addr = (std::size_t)f.GivePolizIndex();
        callFunction(addr, {});
    } else {
        run();
    }
}
