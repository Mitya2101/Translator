#ifndef LEXER_TRIE_H
#define LEXER_TRIE_H

#include <string>
#include <unordered_map>

// Узел префиксного дерева (trie), хранящий переходы по символам.
class TrieNode {
public:
    bool isEnd = false;                             // признак окончания слова
    std::unordered_map<char, TrieNode*> children;   // ребра по символам
};

// Простая реализация trie для хранения набора ключевых слов.
class Trie {
public:
    Trie();
    ~Trie();

    // Добавить слово в структуру.
    void insert(const std::string& word);

    // Проверить, содержится ли слово в trie.
    bool search(const std::string& word) const;

    // Рекурсивное удаление всех узлов.
    void clear(TrieNode* node);

private:
    TrieNode* root = nullptr;
};

#endif // LEXER_TRIE_H
