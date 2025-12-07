#pragma once
#include "SyntaxerNode.h"
#include<vector>

class TFuncElement{
private:
    std::string name_;
    std::vector<Types> param_types_;
    Types return_value_;
    SyntaxerNode* create_;
public:
    TFuncElement(std::string name,
        std::vector<Types> param_types,
        Types return_value);
    TFuncElement(std::string name,
        std::vector<Types> param_types,
        Types return_value,SyntaxerNode* create_);
    bool operator==(TFuncElement a);
    bool IsFind(std::string name,
        std::vector<Types> help);
};

class TFunc{
private:
    std::vector<TFuncElement> all_;
public:
    void CreateFund(TFuncElement a);
    bool Find(TFuncElement a);
    bool Find(std::string name,
        std::vector<Types> param_types);
    TFuncElement Get(std::string name,
        std::vector<Types> param_types);
};