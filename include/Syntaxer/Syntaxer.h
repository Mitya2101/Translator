#pragma once

#include "lexer/lexer.h"
#include "Syntaxer/SyntaxerNode.h"
#include "Syntaxer/TFunc.h"
#include "Syntaxer/TID.h"
#include "Syntaxer/POLIZ.h"


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

    std::shared_ptr<SyntaxerNode> Start();

    Lexer lexer;
    POLIZ poliz;
    std::vector<std::vector<TID>> stack_tids;
    std::vector<std::pair<Types,int>> stack_calculate_simul;
    std::vector<std::vector<int>> helper_break;
    std::vector<std::vector<int>> continue_helper;
    std::vector<int> size_counter;
    std::vector<int> return_helper;
    

    TFunc func;
    bool InCycle = false;
    int InFunction = -1;
    std::string cur_function_name_;
    std::vector<std::shared_ptr<TIDElement>> cur_func;
    void PrintPoliz();

private:
    std::shared_ptr<SyntaxerNode> Program();
    std::shared_ptr<SyntaxerNode> ProgramNoCreateFunction(bool need_create_tid);

    std::shared_ptr<SyntaxerNode> Statement();
    std::shared_ptr<SyntaxerNode> StatementNoCreationFunction();

    std::shared_ptr<SyntaxerNode> If();
    std::shared_ptr<SyntaxerNode> Else();
    std::shared_ptr<SyntaxerNode> While();
    std::shared_ptr<SyntaxerNode> For();

    std::shared_ptr<SyntaxerNode> Return();
    std::shared_ptr<SyntaxerNode> Break();
    std::shared_ptr<SyntaxerNode> Continue();

    std::shared_ptr<SyntaxerNode> StringWithDigit();
    std::shared_ptr<SyntaxerNode> Variable();
    std::shared_ptr<SyntaxerNode> Value();
    std::shared_ptr<SyntaxerNode> DoubleValue();
    std::shared_ptr<SyntaxerNode> Type();

    std::shared_ptr<SyntaxerNode> CreateVariableOrArray();
    std::shared_ptr<SyntaxerNode> CreateFunctionOrVariableOrArray();

    std::shared_ptr<SyntaxerNode> Expr();
    std::shared_ptr<SyntaxerNode> ExprAssign();
    std::shared_ptr<SyntaxerNode> ExprLogicOr();
    std::shared_ptr<SyntaxerNode> ExprLogicAnd();
    std::shared_ptr<SyntaxerNode> ExprEquality();
    std::shared_ptr<SyntaxerNode> ExprRel();
    std::shared_ptr<SyntaxerNode> ExprAdd();
    std::shared_ptr<SyntaxerNode> ExprMul();
    std::shared_ptr<SyntaxerNode> ExprPostfix();
    std::shared_ptr<SyntaxerNode> ExprUnary();
    std::shared_ptr<SyntaxerNode> ExprPrimary();

    std::shared_ptr<SyntaxerNode> ArgList();
    std::shared_ptr<SyntaxerNode> ArgListType();

    bool Find(std::string name);
    std::shared_ptr<TIDElement> Give(std::string name);


    int CompileTimeCalculation(const std::shared_ptr<SyntaxerNode>& now);
};
