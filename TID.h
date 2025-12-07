#pragma once
#include "tokens.h"
#include <vector>



class TIDElement{
private:
    std::string name_;
    Types cur_;
    std::string num_;
public:
    TIDElement(std::string name,Types cur);
    TIDElement(std::string name,Types cur,std::string num);
    bool operator==(TIDElement a);
    bool operator==(std::string name);
};


class TID{
private:
    std::vector<TIDElement> all_;
public:
    void CreateVar(TIDElement a);
    bool Find(std::string name);
    bool Find(TIDElement a);
    TIDElement GetVar(std::string name);
};