#include "tokens.h"

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
        case T::KwClass:         return "KwClass";
        case T::KwConstructor:   return "KwConstructor";
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

        case T::Plus:            return "Plus";
        case T::Minus:           return "Minus";
        case T::Asterisk:        return "Asterisk";
        case T::Slash:           return "Slash";
        case T::Percent:         return "Percent";
        case T::Ampersand:       return "Ampersand";
        case T::VerticalBar:     return "VerticalBar";
        case T::Caret:           return "Caret";
        case T::Exclamation:     return "Exclamation";
        case T::Tilde:           return "Tilde";
        case T::Assign:          return "Assign";
        case T::Less:            return "Less";
        case T::Greater:         return "Greater";
        case T::Dot:             return "Dot";
        case T::Comma:           return "Comma";
        case T::Semicolon:       return "Semicolon";
        case T::LParen:          return "LParen";
        case T::RParen:          return "RParen";
        case T::LBrace:          return "LBrace";
        case T::RBrace:          return "RBrace";
        case T::LBracket:        return "LBracket";
        case T::RBracket:        return "RBracket";
        case T::Backtick:        return "Backtick";

        case T::EqualEqual:      return "EqualEqual";
        case T::NotEqual:        return "NotEqual";
        case T::LessEqual:       return "LessEqual";
        case T::GreaterEqual:    return "GreaterEqual";
        case T::PlusPlus:        return "PlusPlus";
        case T::MinusMinus:      return "MinusMinus";
        case T::AmpAmp:          return "AmpAmp";
        case T::PipePipe:        return "PipePipe";
        case T::Shl:             return "Shl";
        case T::Shr:             return "Shr";

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
