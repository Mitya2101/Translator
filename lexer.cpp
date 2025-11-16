#include "lexer.h"
#include <iostream>
#include <cctype>

Lexer::Lexer(const std::string& sourceName,
             const std::string& kwFile)
{
    input.open(sourceName);
    if (!input.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << sourceName << "\n";
        reachedEOF = true;
        return;
    }

    loadKeywordList(kwFile);

    readChar();

    next();
}

Lexer::Lexer(Lexer&& other) noexcept {
    input       = std::move(other.input);
    keywordTrie = std::move(other.keywordTrie);
    ch          = other.ch;
    reachedEOF  = other.reachedEOF;
    row         = other.row;
    col         = other.col;
    activeToken = std::move(other.activeToken);
}

Lexer& Lexer::operator=(Lexer&& other) noexcept {
    if (this != &other) {
        input       = std::move(other.input);
        keywordTrie = std::move(other.keywordTrie);
        ch          = other.ch;
        reachedEOF  = other.reachedEOF;
        row         = other.row;
        col         = other.col;
        activeToken = std::move(other.activeToken);
    }
    return *this;
}

void Lexer::readChar() {
    if (!input.get(ch)) {
        reachedEOF = true;
        ch = '\0';
        return;
    }

    if (ch == '\n') {
        row++;
        col = 0;
    } else {
        col++;
    }
}

void Lexer::skipNoise() {
    while (!reachedEOF) {
        while (!reachedEOF &&
               std::isspace(static_cast<unsigned char>(ch)))
        {
            readChar();
        }

        if (ch == '/' && input.peek() == '/') {
            while (!reachedEOF && ch != '\n')
                readChar();
            continue;
        }

        if (ch == '/' && input.peek() == '*') {
            readChar();
            readChar();

            bool closed = false;
            while (!reachedEOF) {
                if (ch == '*' && input.peek() == '/') {
                    readChar();
                    readChar();
                    closed = true;
                    break;
                }
                readChar();
            }

            if (!closed) {
                std::cerr << "Предупреждение: блоковый комментарий не закрыт\n";
            }

            continue;
        }

        break;
    }
}

void Lexer::loadKeywordList(const std::string& filename) {
    std::ifstream kw(filename);
    if (!kw.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл ключевых слов: "
                  << filename << "\n";
        return;
    }

    std::string word;
    while (kw >> word) {
        keywordTrie.insert(word);
    }
}

const Token& Lexer::currentToken() const {
    return activeToken;
}

Token Lexer::make(Token::Type type, const std::string& lex, int line, int column) {
    Token t;
    t.type = type;
    t.lexeme = lex;
    t.pos = { line, column };
    return t;
}

Token Lexer::next() {
    skipNoise();

    if (reachedEOF) {
        activeToken = make(Token::Type::EndOfFile, "", row, col);
        return activeToken;
    }

    if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
        activeToken = scanWordOrKeyword();
    }
    else if (std::isdigit(static_cast<unsigned char>(ch))) {
        activeToken = scanNumber();
    }
    else if (ch == '\'') {
        activeToken = scanCharLiteral();
    }
    else if (ch == '\"') {
        activeToken = scanStringLiteral();
    }
    else {
        activeToken = scanOperator();
    }

    return activeToken;
}

Token Lexer::peek() {
    auto posBefore = input.tellg();

    char savedChar = ch;
    bool savedEOF  = reachedEOF;
    int savedRow   = row;
    int savedCol   = col;
    Token savedTok = activeToken;

    Token result = const_cast<Lexer*>(this)->next();

    input.clear();
    input.seekg(posBefore);
    ch         = savedChar;
    reachedEOF = savedEOF;
    row        = savedRow;
    col        = savedCol;
    activeToken = savedTok;

    return result;
}

Token Lexer::scanWordOrKeyword() {
    std::string buf;
    int tokLine = row;
    int tokCol  = (col == 0 ? 1 : col);

    while (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
        buf += ch;
        readChar();
    }

    if (keywordTrie.search(buf)) {
        using T = Token::Type;

        if (buf == "int")         return make(T::KwInt, buf, tokLine, tokCol);
        if (buf == "char")        return make(T::KwChar, buf, tokLine, tokCol);
        if (buf == "bool")        return make(T::KwBool, buf, tokLine, tokCol);
        if (buf == "float")       return make(T::KwFloat, buf, tokLine, tokCol);
        if (buf == "void")        return make(T::KwVoid, buf, tokLine, tokCol);

        if (buf == "main")        return make(T::KwMain, buf, tokLine, tokCol);

        if (buf == "if")          return make(T::KwIf, buf, tokLine, tokCol);
        if (buf == "elif")        return make(T::KwElif, buf, tokLine, tokCol);
        if (buf == "else")        return make(T::KwElse, buf, tokLine, tokCol);
        if (buf == "while")       return make(T::KwWhile, buf, tokLine, tokCol);
        if (buf == "for")         return make(T::KwFor, buf, tokLine, tokCol);
        if (buf == "return")      return make(T::KwReturn, buf, tokLine, tokCol);
        if (buf == "break")       return make(T::KwBreak, buf, tokLine, tokCol);
        if (buf == "continue")    return make(T::KwContinue, buf, tokLine, tokCol);

        if (buf == "print")       return make(T::KwPrint, buf, tokLine, tokCol);
        if (buf == "read")        return make(T::KwRead, buf, tokLine, tokCol);

        if (buf == "true")        return make(T::KwTrue, buf, tokLine, tokCol);
        if (buf == "false")       return make(T::KwFalse, buf, tokLine, tokCol);
    }

    return make(Token::Type::Identifier, buf, tokLine, tokCol);
}

Token Lexer::scanNumber() {
    std::string num;
    bool dotUsed = false;

    int tokLine = row;
    int tokCol  = (col == 0 ? 1 : col);

    while (std::isdigit(static_cast<unsigned char>(ch)) ||
           (!dotUsed && ch == '.'))
    {
        if (ch == '.') dotUsed = true;
        num += ch;
        readChar();
    }

    if (dotUsed)
        return make(Token::Type::FloatLiteral, num, tokLine, tokCol);

    return make(Token::Type::IntegerLiteral, num, tokLine, tokCol);
}

Token Lexer::scanCharLiteral() {
    int tokLine = row;
    int tokCol  = (col == 0 ? 1 : col);

    readChar();
    std::string result;

    if (reachedEOF || ch == '\n' || ch == '\'') {
        std::cerr << "Ошибка: пустой символьный литерал " << tokLine
                  << ":" << tokCol << "\n";
        return make(Token::Type::CharLiteral, "", tokLine, tokCol);
    }

    if (ch == '\\') {
        result += ch;
        readChar();
        if (!reachedEOF) {
            result += ch;
            readChar();
        }
    } else {
        result += ch;
        readChar();
    }

    if (ch != '\'') {
        std::cerr << "Ошибка: незавершённый символьный литерал "
                  << tokLine << ":" << tokCol << "\n";
    } else {
        readChar();
    }

    return make(Token::Type::CharLiteral, result, tokLine, tokCol);
}

Token Lexer::scanStringLiteral() {
    int tokLine = row;
    int tokCol  = (col == 0 ? 1 : col);

    readChar();
    std::string str;
    bool escaped = false;

    while (!reachedEOF) {
        if (escaped) {
            switch (ch) {
                case 'n':  str += '\n'; break;
                case 't':  str += '\t'; break;
                case 'r':  str += '\r'; break;
                case '\\': str += '\\'; break;
                case '\"':  str += '\"'; break;
                default:   str += ch;   break;
            }
            escaped = false;
        }
        else if (ch == '\\') {
            escaped = true;
        }
        else if (ch == '\"') {
            readChar();
            return make(Token::Type::StringLiteral, str, tokLine, tokCol);
        }
        else if (ch == '\n') {
            std::cerr << "Предупреждение: незавершенная строка "
                      << tokLine << ":" << tokCol << "\n";
            break;
        }
        else {
            str += ch;
        }
        readChar();
    }

    return make(Token::Type::StringLiteral, str, tokLine, tokCol);
}

Token Lexer::scanOperator() {
    int tokLine = row;
    int tokCol  = (col == 0 ? 1 : col);

    char first = ch;
    char lookahead = input.peek();
    std::string two(1, first);
    two.push_back(lookahead);

    using T = Token::Type;

    auto tryTwo = [&](const char* s, T type) -> bool {
        if (two == s) {
            readChar();
            readChar();
            activeToken = make(type, s, tokLine, tokCol);
            return true;
        }
        return false;
    };

    if (!reachedEOF) {
        if (tryTwo("==", T::EqualEqual)) return activeToken;
        if (tryTwo("!=", T::NotEqual))   return activeToken;
        if (tryTwo("<=", T::LessEqual))  return activeToken;
        if (tryTwo(">=", T::GreaterEqual))return activeToken;
        if (tryTwo("++", T::PlusPlus))   return activeToken;
        if (tryTwo("--", T::MinusMinus)) return activeToken;
        if (tryTwo("&&", T::AmpAmp))     return activeToken;
        if (tryTwo("||", T::PipePipe))   return activeToken;
        if (tryTwo("<<", T::Shl))        return activeToken;
        if (tryTwo(">>", T::Shr))        return activeToken;
    }

    readChar();

    switch (first) {
        case '(': return make(T::LParen, "(", tokLine, tokCol);
        case ')': return make(T::RParen, ")", tokLine, tokCol);
        case '{': return make(T::LBrace, "{", tokLine, tokCol);
        case '}': return make(T::RBrace, "}", tokLine, tokCol);
        case '[': return make(T::LBracket, "[", tokLine, tokCol);
        case ']': return make(T::RBracket, "]", tokLine, tokCol);
        case ',': return make(T::Comma, ",", tokLine, tokCol);
        case ';': return make(T::Semicolon, ";", tokLine, tokCol);
        case '.': return make(T::Dot, ".", tokLine, tokCol);
        case '`': return make(T::Backtick, "`", tokLine, tokCol);
        case '+': return make(T::Plus, "+", tokLine, tokCol);
        case '-': return make(T::Minus, "-", tokLine, tokCol);
        case '*': return make(T::Asterisk, "*", tokLine, tokCol);
        case '/': return make(T::Slash, "/", tokLine, tokCol);
        case '%': return make(T::Percent, "%", tokLine, tokCol);
        case '&': return make(T::Ampersand, "&", tokLine, tokCol);
        case '|': return make(T::VerticalBar, "|", tokLine, tokCol);
        case '^': return make(T::Caret, "^", tokLine, tokCol);
        case '!': return make(T::Exclamation, "!", tokLine, tokCol);
        case '~': return make(T::Tilde, "~", tokLine, tokCol);
        case '=': return make(T::Assign, "=", tokLine, tokCol);
        case '<': return make(T::Less, "<", tokLine, tokCol);
        case '>': return make(T::Greater, ">", tokLine, tokCol);
        default:
            std::cerr << "Неизвестный символ '" << first
                      << "' в позиции " << tokLine << ":" << tokCol << "\n";
            return make(T::Identifier, std::string(1, first),
                        tokLine, tokCol);
    }
}
