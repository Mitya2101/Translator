#include "Syntaxer.h"


SyntaxerNode::SyntaxerNode(Token cur):cur_(cur){};
Token SyntaxerNode::GiveToken(){
    return cur_;
}
std::vector<SyntaxerNode*> SyntaxerNode::GiveChildrens(){
    return childrens_;
}


Syntaxer::Syntaxer(const std::string& sourceName):lexer(sourceName){};


SyntaxerNode* Syntaxer::Start(){
    return Program();
}

