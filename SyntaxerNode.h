#pragma once
#include "lexer.h"
#include<vector>


class SyntaxerNode{
    public:

        Token GiveToken();
        void AddChildren(SyntaxerNode* now);
        void UpdateType(Token::Type a);
        void UpdateLexeme(std::string now);
        void UpdatePos(Position a);
        SyntaxerNode** GiveChildrens();
        int GiveSize();

        void resize(){
            SyntaxerNode** new_childs = new SyntaxerNode*[capacity* 2];
            capacity *= 2;
            for(int i =0 ;i < size;i++){
                new_childs[i] = childrens_[i];
            }
            childrens_ = new_childs;
        }
    private:
        Token cur_;
        SyntaxerNode** childrens_ = new SyntaxerNode*[1];
        int size = 0;
        int capacity = 1;
};
    