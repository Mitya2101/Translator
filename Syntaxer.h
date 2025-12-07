
#include "lexer.h"
#include "TFunc.h"
#include "TID.h"

enum NotTerminal{
    Start,
    Program,
    ProgramNoCreateFunction,
    Statement,
    StatementNoCreationFunction,
    Return,
    Break,
    Continue,
    String,
    StringWithDigit,
    Variable,
    letter,
    digit,
    Value,
    DoubleValue,
    Type,
    CreateVariableOrArray,
    CreateFunctionOrVariableOrArray,
    If,
    Else,
    While,
    For,
    Expr,
    ExprComma,
    ExprAssign,
    ExprLogicOr,
    ExprLogicAnd,
    ExprEquality,
    ExprRel,
    ExprAdd,
    ExprMul,
    ExprPostfix,
    ExprUnary,
    ExprPrimary,
    ArgList,
    ArgListType
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
    SyntaxerNode* StringWithDigit();
    SyntaxerNode* Variable();
    SyntaxerNode* Value();
    SyntaxerNode* DoubleValue();
    SyntaxerNode* Type();
    SyntaxerNode* CreateVariableOrArray();
    SyntaxerNode* CreateFunctionOrVariableOrArray();
    SyntaxerNode* Expr();
    // SyntaxerNode* ExprComma();
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
};