#pragma once
#include <string>

// If your project already defines enum class Types elsewhere, you can delete this file
// and include your existing Types definition instead.
enum class Types {
    INT,
    DOUBLE,
    CHAR,
    BOOL,
    VOID
};

inline std::string TypeToString(Types t) {
    switch (t) {
        case Types::INT:    return "int";
        case Types::DOUBLE: return "double";
        case Types::CHAR:   return "char";
        case Types::BOOL:   return "bool";
        case Types::VOID:   return "void";
    }
    return "unknown";
}
