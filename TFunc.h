#pragma once
#include "SyntaxerNode.h"
#include<vector>

std::string UpdateName(std::string name,std::vector<Types> param);

class TFuncElement{
private:
    std::string name_;
    std::vector<Types> param_types_;
    Types return_value_;
    SyntaxerNode* create_;
public:
    std::string GiveName();
    std::vector<Types> GiveParam();
    Types GiveReturnValue();
    SyntaxerNode* GiveCreate();
    TFuncElement(std::string name,
        std::vector<Types> param_types,
        Types return_value,SyntaxerNode* create_);
};

class TFunc{
private:
    std::vector<TFuncElement> all_;
public:
    void CreateFunc(TFuncElement a);
    bool Find(std::string name);
    TFuncElement Get(std::string name);
    TFuncElement Get(int ind);
    int GiveSize();
};