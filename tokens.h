#ifndef LEXER_TOKENS_H
#define LEXER_TOKENS_H

#include <string>

struct Position {
    int line  = 1;
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
        KwFloat,
        KwVoid,
        // KwMain,
        KwIf,
        // KwElif,
        KwElse,
        KwWhile,
        KwFor,
        KwReturn,
        KwBreak,
        KwContinue,
        KwPrint,
        KwRead,
        // KwTrue,
        // KwFalse,

        Plus,
        Minus,
        Asterisk,
        Slash,
        Percent,
        // Ampersand,
        // VerticalBar,
        // Caret,
        Exclamation,
        // Tilde,
        Assign,
        Less,
        Greater,
        Dot,
        Comma,
        Semicolon,
        LParen,
        RParen,
        LBrace,
        RBrace,
        LBracket,
        RBracket,
        Backtick,

        EqualEqual,
        NotEqual,
        LessEqual,
        GreaterEqual,
        // PlusPlus,
        // MinusMinus,
        AmpAmp,
        PipePipe,
        // Shl,
        // Shr,

        EndOfFile
    };

    Type type = Type::EndOfFile;
    std::string lexeme;
    Position pos;

    std::string typeToString() const;
    std::string toString() const;
};

#endif
