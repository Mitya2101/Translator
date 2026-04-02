#pragma once
#include<memory>
#include "lexer/lexer.h"
#include<vector>


class SyntaxerNode{
    public:

        Token GiveToken();
        void AddChildren(const std::shared_ptr<SyntaxerNode>& now);
        void UpdateType(Token::Type a);
        void UpdateLexeme(std::string now);
        Position GivePosition();
        std::string GiveLexeme();
        void UpdatePos(Position a);
        std::vector<std::shared_ptr<SyntaxerNode>> GiveChildrens();
        int GiveSize();
    private:
        Token cur_;
        std::vector<std::shared_ptr<SyntaxerNode>> childrens_;
};
    