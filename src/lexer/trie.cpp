#include "lexer/trie.h"

Trie::Trie() {
    root = new TrieNode();
}

Trie::~Trie() {
    if (root) {
        clear(root);
        root = nullptr;
    }
}

void Trie::clear(TrieNode* node) {
    if (!node) {
        return;
    }

    // Сначала удаляем всех потомков, затем сам узел.
    for (auto& entry : node->children) {
        clear(entry.second);
    }
    delete node;
}

void Trie::insert(const std::string& word) {
    if (!root) {
        root = new TrieNode();
    }

    TrieNode* current = root;
    for (char ch : word) {
        auto it = current->children.find(ch);
        if (it == current->children.end()) {
            TrieNode* next = new TrieNode();
            current->children[ch] = next;
            current = next;
        } else {
            current = it->second;
        }
    }
    current->isEnd = true;
}

bool Trie::search(const std::string& word) const {
    const TrieNode* current = root;
    if (!current) {
        return false;
    }

    for (char ch : word) {
        auto it = current->children.find(ch);
        if (it == current->children.end()) {
            return false;
        }
        current = it->second;
    }
    return current->isEnd;
}
