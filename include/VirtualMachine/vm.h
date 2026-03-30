#pragma once

#include "Syntaxer/POLIZ.h"
#include "Syntaxer/TFunc.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <variant>
#include <vector>

class PolizVm {
public:
    // VM operates on produced POLIZ and function table.
    // We keep non-const refs because the original project API does not provide const accessors.
    PolizVm(POLIZ& code, TFunc& funcs);

    // Выполнить начиная с 0 (если в файле есть top-level код)
    void run();

    // Если функция main существует — выполнить её,
    // иначе run()
    void runAuto();

private:
    struct Address {
        std::size_t abs = 0;
        Types type = Types::INT;
    };

    using Value = std::variant<std::int32_t, double, char, bool, std::string, Address>;

    struct Frame {
        std::size_t bp = 0;
        std::size_t sp = 0;
        // Scalar parameters are stored by VALUE in the callee frame.
        // Array parameters are stored as POINTERS (int) to the caller memory.
        // We store offsets (relative to bp) of pointer-parameters to know when an
        // "address-of" operation must be dereferenced.
        std::vector<std::size_t> ptrParamOffsets;
    };

    POLIZ& code_;
    TFunc& funcs_;

    std::vector<std::uint8_t> mem_;
    std::vector<Frame> frames_;

private:
    static std::int64_t toI64(const std::string& s);
    static char decodeChar(const std::string& s);

    std::size_t sizeOf(Types t) const;

    template<typename T>
    T readPod(std::size_t abs) const {
        T v{};
        std::memcpy(&v, mem_.data() + abs, sizeof(T));
        return v;
    }

    template<typename T>
    void writePod(std::size_t abs, const T& v) {
        std::memcpy(mem_.data() + abs, &v, sizeof(T));
    }

    Value eval(Value v);
    Address asAddress(const Value& v);

    Value readTyped(const Address& a);
    void writeTyped(const Address& a, const Value& v);

    std::optional<std::string> findFuncByAddr(std::size_t addr);

    struct FuncLayout {
        std::size_t labelIndex = 0;
        std::size_t allocIndex = 0;
        std::size_t bodyStart = 0;
        std::size_t endIp = 0;
        std::vector<std::size_t> ptrParamOffsets;
    };

    FuncLayout analyzeLayout(std::size_t storedAddr, TFuncElement f);

    Value callFunction(std::size_t storedAddr, std::vector<Value> args);

    void exec(std::size_t ipBegin, std::size_t ipEnd, std::vector<Value>& stack);
};
