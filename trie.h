#ifndef LEXER_TRIE_H
#define LEXER_TRIE_H

#include <string>
#include <unordered_map>

class TrieNode {
public:
    bool isEnd = false;
    std::unordered_map<char, TrieNode*> children;
};

class Trie {
public:
    Trie();
    ~Trie();

    void insert(const std::string& word);
    bool search(const std::string& word) const;
    void clear(TrieNode* node);

private:
    TrieNode* root = nullptr;
};

#endif
