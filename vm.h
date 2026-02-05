#pragma once

#include "POLIZ.h"
#include "TFunc.h"
#include "types.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <cstdint>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

struct Address {
    std::int64_t offset = 0;
};

enum class VKind { Int, Double, Bool, Char, String, Void };

struct Value {
    VKind kind = VKind::Void;
    std::variant<std::int64_t, double, bool, char, std::string> data;

    static Value makeInt(std::int64_t v) { return {VKind::Int, v}; }
    static Value makeDouble(double v) { return {VKind::Double, v}; }
    static Value makeBool(bool v) { return {VKind::Bool, v}; }
    static Value makeChar(char v) { return {VKind::Char, v}; }
    static Value makeString(std::string v) { return {VKind::String, std::move(v)}; }
    static Value makeVoid() { return {VKind::Void, 0LL}; }
};

using StackItem = std::variant<Value, Address>;

class VM {
public:
    VM(const POLIZ& poliz, const TFunc& funcs);

    void run();
    void setTrace(bool on) { trace_ = on; }

private:
    struct Cell {
        bool is_ref = false;     // reference cell stores a target address
        bool initialized = false;

        VKind kind = VKind::Void;
        std::variant<std::int64_t, double, bool, char, std::string> data;

        std::int64_t ref_target = 0; // where reference points
    };

    struct ParamMeta {
        std::int64_t offset = 0; // where param cell lives (it is a reference cell)
        Types type = Types::INT;
    };

    struct FuncMeta {
        int entry_after_alloc = 0; // what you store in FUNCTION_ADRESS
        int alloc_ip = 0;          // entry_after_alloc - 1 (ALLOCATE just before body)
        int end_ip = 0;            // ip right after FREE (end label)
        Types ret = Types::VOID;
        std::vector<ParamMeta> params;
    };

    struct Frame {
        int return_ip = 0;
        int end_ip = 0;
        std::int64_t sp_base = 0;
        std::size_t eval_base = 0;
        Types ret = Types::VOID;
    };

    const POLIZ& poliz_;
    const TFunc& funcs_;
    std::unordered_map<int, FuncMeta> fmeta_;

    int ip_ = 0;
    std::int64_t sp_ = 0;
    std::vector<Frame> call_stack_;
    std::vector<StackItem> eval_;
    std::unordered_map<std::int64_t, Cell> mem_;

    bool trace_ = false;

    static VKind kindFromTypes(Types t);

    StackItem pop();
    void push(StackItem x);

    Value asValue(const StackItem& x);
    Address asAddress(const StackItem& x);

    Value parseLiteral(const std::string& s);

    Cell& ensureCell(std::int64_t addr);
    const Cell& getCell(std::int64_t addr) const;

    Value loadAt(std::int64_t addr);
    void  storeAt(std::int64_t addr, const Value& v);

    void execOperation(const std::string& op);
    void execUnary(const std::string& op);

    void execCall();
    void maybeReturnFromFunction();
};
