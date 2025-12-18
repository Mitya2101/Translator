#pragma once
#include "tokens.h"
#include <vector>


class TIDElement{
protected:
    std::string name_;
    Types cur_;
public:
    std::string GiveName();
    Types GiveType();
    TIDElement(std::string name,Types cur);
    virtual bool operator==(std::string name);
    
};

template<typename T>
class TIDElementArray:public TIDElement{
private:
    std::vector<int> sizes_;
    std::vector<T> arr_;
public:
    TIDElementArray(std::string name,Types cur,std::vector<int> sizes);
    TIDElementArray(std::string name,Types cur,int size_size);
    const std::vector<int>& GiveSizes();
    const std::vector<T>& GiveArr();
};

template<typename T>
class TIDElementVariable:public TIDElement{
private:
    T num_;
public:
    TIDElementVariable(std::string name,Types cur);
    TIDElementVariable(std::string name,Types cur,T num);
    const T& GiveNum();
};

class TID{
private:
    std::vector<TIDElement*> all_;
public:
    void CreateVar(TIDElement* a);
    bool Find(std::string name);
    TIDElement* GetVar(std::string name);
    ~TID();
};