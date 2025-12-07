#include "SyntaxerNode.h"

std::vector<SyntaxerNode*> SyntaxerNode::GiveChildrens(){
    return childrens_;
}

Token SyntaxerNode::GiveToken(){
    return cur_;
}

void SyntaxerNode::AddChildren(SyntaxerNode* now){
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
