#pragma once
#include <string>
#include "types.h"

struct Position {
    int line = 1;
    int column = 0;
};

struct Token {
    enum class Type {
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        CharLiteral,
        StringLiteral,

        KwInt,
        KwChar,
        KwBool,
        KwDouble,
        KwVoid,

        KwIf,
        KwElse,
        KwWhile,
        KwFor,
        KwReturn,
        KwBreak,
        KwContinue,
        KwPrint,
        KwRead,
        KwTrue,
        KwFalse,

        Operator,       // + - * / % = == != < <= > >= && || , etc. (lexeme stores exact text)
        Separator,      // ; ,
        OpenBracket,    // ( { [
        CloseBracket,   // ) } ]

        EndOfFile
    };

    Type type = Type::EndOfFile;
    std::string lexeme;
    Position pos;

    std::string typeToString() const;
    std::string toString() const;
};
