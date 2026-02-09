#pragma once
#include "SyntaxerNode.h"
#include<vector>
#include "TID.h"

std::string UpdateName(std::string name,std::vector<Types> param);

class TFuncElement{
private:
    std::string name_;
    std::vector<TIDElement*> param_types_;
    Types return_value_;
    int size_ = 0;
    SyntaxerNode* create_;
    int poliz_index;
public:
    std::string GiveName() const;
    std::vector<TIDElement*> GiveParam() const;
    Types GiveReturnValue() const;
    int GiveArraySize() const;
    SyntaxerNode* GiveCreate() const;
    int GivePolizIndex() const;
    TFuncElement(std::string name,
        std::vector<TIDElement*> param_types,
        Types return_value,SyntaxerNode* create_,int size,int poliz_index);
};

class TFunc{
private:
    std::vector<TFuncElement> all_;
public:
    void CreateFunc(TFuncElement a);
    bool Find(const std::string& name) const;
    TFuncElement Get(const std::string& name) const;
    TFuncElement Get(int ind) const;
    int GiveSize() const;
};