#ifndef LEXER_TOKENS_H
#define LEXER_TOKENS_H

#include <string>

struct Position {
    int line  = 1;
    int column = 0;
};

enum Types{
    VOID,
    INT,
    DOUBLE,
    CHAR,
    BOOL
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
        KwFloat,
        KwVoid,
        KwMain,
        KwIf,
        KwElif,
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

        Operator,
        OpenBracket,
        CloseBracket,
        Separator,

        EndOfFile
    };

    Type type = Type::EndOfFile;
    std::string lexeme;
    Position pos;

    std::string typeToString() const;
    std::string toString() const;
};

#endif
