#pragma once

#include "lexer.h"
#include "SyntaxerNode.h"
#include "TFunc.h"
#include "TID.h"
#include "POLIZ.h"


enum NotTerminal{
    Start,
    Program,
    ProgramNoCreateFunction,
    Statement,
    StatementNoCreationFunction,
    ReturnNt,
    BreakNt,
    ContinueNt,
    StringNt,
    StringWithDigitNt,
    VariableNt,
    letter,
    digit,
    ValueNt,
    DoubleValueNt,
    TypeNt,
    CreateVariableOrArrayNt,
    CreateFunctionOrVariableOrArrayNt,
    IfNt,
    ElseNt,
    WhileNt,
    ForNt,
    ExprNt,
    ExprComma,
    ExprAssignNt,
    ExprLogicOrNt,
    ExprLogicAndNt,
    ExprEqualityNt,
    ExprRelNt,
    ExprAddNt,
    ExprMulNt,
    ExprPostfixNt,
    ExprUnaryNt,
    ExprPrimaryNt,
    ArgListNt,
    ArgListTypeNt
};

class Syntaxer{
public:
    explicit Syntaxer(const std::string& sourceName);

    SyntaxerNode* Start();

    Lexer lexer;
    POLIZ poliz;
    std::vector<std::vector<TID>> tids;
    std::vector<std::pair<Types,int>> all;
    std::vector<std::vector<int>> helper_break;
    std::vector<std::vector<int>> continue_helper;
    std::vector<int> size_counter;
    std::vector<int> return_helper;
    

    TFunc func;
    bool InCycle = false;
    int InFunction = -1;
    std::string cur_function_name_;
    std::vector<TIDElement*> cur_func;
    void PrintPoliz();

private:
    SyntaxerNode* Program();
    SyntaxerNode* ProgramNoCreateFunction(bool need);

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

    bool Find(std::string name);
    TIDElement* Give(std::string name);


    int dfs(SyntaxerNode* now);
};
