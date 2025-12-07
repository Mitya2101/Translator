#include "lexer.h"

class SyntaxerNode{
    public:
        Token GiveToken();
        void AddChildren(SyntaxerNode* now);
        void UpdateType(Token::Type a);
        void UpdateLexeme(std::string now);
        void UpdatePos(Position a);
        std::vector<SyntaxerNode*> GiveChildrens();
    private:
        Token cur_;
        std::vector<SyntaxerNode*> childrens_;
};
    