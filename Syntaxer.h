
#include "lexer.h"

class SyntaxerNode{
public:
    SyntaxerNode(Token cur);
    Token GiveToken();
    std::vector<SyntaxerNode*> GiveChildrens();
private:
    Token cur_;
    std::vector<SyntaxerNode*> childrens_;
};


class Syntaxer{
public:
    Lexer lexer;
    Syntaxer(const std::string& sourceName);

    SyntaxerNode* Start();
private:
    SyntaxerNode* Program();
    SyntaxerNode* ProgramNoCreateFunction();
    SyntaxerNode* Statement();
    SyntaxerNode* StatementNoCreationFunction();
    SyntaxerNode* If();
    SyntaxerNode* Else();
    SyntaxerNode* While();
    SyntaxerNode* For();
    SyntaxerNode* Return();
    SyntaxerNode* Break();
    SyntaxerNode* Continue();
    SyntaxerNode* String();
    SyntaxerNode* StringWithDigit();
    SyntaxerNode* Variable();
    SyntaxerNode* Value();
    SyntaxerNode* DoubleValue();
    SyntaxerNode* Type();
    SyntaxerNode* CreateVariableOrArray();
    SyntaxerNode* CreateFunctionOrVariableOrArray();
    SyntaxerNode* Expr();
    SyntaxerNode* ExprComma();
    SyntaxerNode* ExprAssign();
    SyntaxerNode* ExprLogicOr();
    SyntaxerNode* ExprLogicAnd();
    SyntaxerNode* ExprEquality();
    SyntaxerNode* ExprRel();
    SyntaxerNode* ExprAdd();
    SyntaxerNode* ExprMul();
    SyntaxerNode* ExprPostfix();
    SyntaxerNode* ExprUnary();
    SyntaxerNode* ExprPrimary();
    SyntaxerNode* ArgList();
    SyntaxerNode* ArgListType();
    SyntaxerNode* letter();
    SyntaxerNode* digit();
};