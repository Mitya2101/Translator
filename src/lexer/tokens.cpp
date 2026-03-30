#include "lexer/tokens.h"

std::string Token::typeToString() const {
    using T = Type;

    switch (type) {
        case T::Identifier:      return "Identifier";
        case T::IntegerLiteral:  return "IntegerLiteral";
        case T::FloatLiteral:    return "FloatLiteral";
        case T::CharLiteral:     return "CharLiteral";
        case T::StringLiteral:   return "StringLiteral";

        case T::KwInt:           return "KwInt";
        case T::KwChar:          return "KwChar";
        case T::KwBool:          return "KwBool";
        case T::KwFloat:         return "KwFloat";
        case T::KwVoid:          return "KwVoid";
        case T::KwMain:          return "KwMain";
        case T::KwIf:            return "KwIf";
        case T::KwElif:          return "KwElif";
        case T::KwElse:          return "KwElse";
        case T::KwWhile:         return "KwWhile";
        case T::KwFor:           return "KwFor";
        case T::KwReturn:        return "KwReturn";
        case T::KwBreak:         return "KwBreak";
        case T::KwContinue:      return "KwContinue";
        case T::KwPrint:         return "KwPrint";
        case T::KwRead:          return "KwRead";
        case T::KwTrue:          return "KwTrue";
        case T::KwFalse:         return "KwFalse";

        case T::Operator:        return "Operator";
        case T::OpenBracket:     return "OpenBracket";
        case T::CloseBracket:    return "CloseBracket";
        case T::Separator:       return "Separator";

        case T::EndOfFile:       return "EndOfFile";
    }

    return "Unknown";
}

std::string Token::toString() const {
    std::string repr;

    repr.reserve(32 + lexeme.size());

    repr += typeToString();
    repr += "('";
    repr += lexeme;
    repr += "', line=";
    repr += std::to_string(pos.line);
    repr += ", col=";
    repr += std::to_string(pos.column);
    repr += ")";

    return repr;
}
