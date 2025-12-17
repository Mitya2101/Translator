#define _GLIBCXX_DEBUG
#include "SyntaxerNode.h"
#include<iostream>
#include<cassert>

SyntaxerNode** SyntaxerNode::GiveChildrens(){
    return childrens_;
}

int SyntaxerNode::GiveSize(){
    return size;
}

Token SyntaxerNode::GiveToken(){
    return cur_;
}

void SyntaxerNode::AddChildren(SyntaxerNode* now){
    if(size == capacity){
        resize();
    }
    childrens_[size++] = now;
}

void SyntaxerNode::UpdateLexeme(std::string now){
    cur_.lexeme = now;
}

void SyntaxerNode::UpdateType(Token::Type a){
    cur_.type = a;
}

void SyntaxerNode::UpdatePos(Position a){
    cur_.pos = a;
}
