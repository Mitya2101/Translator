#pragma once

#include <fstream>
#include <string>
#include "tokens.h"
#include "trie.h"

class Lexer {
public:
    explicit Lexer(const std::string& sourceName,
                   const std::string& kwFile = "keywords.txt");

    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    Lexer(Lexer&& other) noexcept;
    Lexer& operator=(Lexer&& other) noexcept;

    const Token& currentToken() const;

    Token next();

    Token peek();

    void loadKeywordList(const std::string& filename);

private:
    std::ifstream input;
    Trie keywordTrie;

    char ch = '\0';
    bool reachedEOF = false;
    int row = 1;
    int col = 0;
    Token activeToken;

    void readChar();
    void skipNoise();

    static Token make(Token::Type type,
                      const std::string& lex,
                      int line, int column);

    Token scanWordOrKeyword();
    Token scanNumber();
    Token scanCharLiteral();
    Token scanStringLiteral();
    Token scanOperator();
};
