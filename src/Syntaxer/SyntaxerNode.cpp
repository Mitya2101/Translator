#include "Syntaxer/SyntaxerNode.h"
#include<iostream>
#include<cassert>

std::vector<std::shared_ptr<SyntaxerNode>> SyntaxerNode::GiveChildrens(){
    return childrens_;
}

int SyntaxerNode::GiveSize(){
    return childrens_.size();
}

Token SyntaxerNode::GiveToken(){
    return cur_;
}

std::string SyntaxerNode::GiveLexeme(){
    return cur_.lexeme;
}


void SyntaxerNode::AddChildren(const std::shared_ptr<SyntaxerNode>& now){
    childrens_.push_back(now);
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


Position SyntaxerNode::GivePosition(){
    return cur_.pos;
}
