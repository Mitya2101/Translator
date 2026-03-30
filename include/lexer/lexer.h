#pragma once

#include <fstream>
#include <string>
#include "tokens.h"
#include "trie.h"

//
// Лексический анализатор:
// отвечает за пошаговое преобразование входного файла
// в поток токенов. Файл читается посимвольно.
//
class Lexer {
public:
    // Создание лексера на основе имени файла и файла со списком ключевых слов.
    explicit Lexer(const std::string& sourceName,
                   const std::string& kwFile = "keywords.txt");

    // Запрет копирований — поток и состояние копировать нельзя.
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    // Разрешаем перемещение.
    Lexer(Lexer&& other) noexcept;
    Lexer& operator=(Lexer&& other) noexcept;

    // Получить токен, который сейчас находится под курсором.
    const Token& currentToken() const;

    // Считать и вернуть следующий токен.
    Token next();

    // Посмотреть на следующий токен, но не продвигать состояние.
    Token peek();

    // Загрузка набора ключевых слов из внешнего файла.
    void loadKeywordList(const std::string& filename);

private:
    // --- Внутреннее состояние ---
    std::ifstream input;
    Trie keywordTrie;

    char ch = '\0';         // последний считанный символ
    bool reachedEOF = false;
    int row = 1;            // номер строки
    int col = 0;            // номер колонки
    Token activeToken;      // токен, созданный последним вызовом next()

    // --- Вспомогательные методы чтения ---
    void readChar();              // прочитать один символ
    void skipNoise();             // пропустить пробелы, табы, комментарии

    // Создание токена.
    static Token make(Token::Type type,
                      const std::string& lex,
                      int line, int column);

    // Разбор различных видов лексем.
    Token scanWordOrKeyword();
    Token scanNumber();
    Token scanCharLiteral();
    Token scanStringLiteral();
    Token scanOperator();
};

