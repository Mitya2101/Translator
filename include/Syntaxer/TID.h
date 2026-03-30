#pragma once
#include "lexer/tokens.h"
#include <vector>
#include<iostream>


class TIDElement{
protected:
    std::string name_;
    Types cur_;
    int offset_;
    bool is_global;
public:
    std::string GiveName(){
        return name_; 
    }
    Types GiveType(){
        return cur_;
    }
    TIDElement(std::string name,Types cur,int offset,bool is_global):offset_(offset),
    name_(name),cur_(cur),is_global(is_global){
        if(cur_ == Types::VOID){
            throw "you cannot create var of void type";
        }
    }  
    virtual bool operator==(std::string name){
        return name_ == name;        
    }  
    int GiveOffset(){
        return offset_;
    }
    bool IsGlobal(){
        return is_global;
    }
};

template<typename T>
class TIDElementArray:public TIDElement{
private:
    std::vector<int> sizes_;
    std::vector<T> arr_;
public:
    TIDElementArray(std::string name,Types cur,std::vector<int> sizes,int offset,bool is_global):
    sizes_(sizes),TIDElement(name,cur,offset,is_global){
    };
    
    TIDElementArray(std::string name,Types cur,int size_size,int offset,bool is_global):
    TIDElement(name,cur,offset,is_global){
        sizes_.resize(size_size);
    }
    
    const std::vector<int>& GiveSizes(){
        return sizes_;
    }
    const std::vector<T>& GiveArr(){
        return arr_;
    }

    
};

template<typename T>
class TIDElementVariable:public TIDElement{
private:
    T num_;
public:
    TIDElementVariable(std::string name,Types cur,int offset,bool is_global):
    TIDElement(name,cur,offset,is_global){};
    TIDElementVariable(std::string name,Types cur,T num,int offset,bool is_global):
    TIDElement(name,cur,offset,is_global),num_(num){};
    const T& GiveNum(){
        return num_;    
    }
};

class TID{
private:
    std::vector<TIDElement*> all_;
public:
    void CreateVar(TIDElement* a){   
        if(Find(a->GiveName())){
            throw "such element has already exists";
        }
        all_.push_back(a);
        
    }
    bool Find(std::string name){
        for(int i =0 ;i < all_.size();i++){
            if(all_[i]->GiveName() == name)return true;
        }
        return false;
    }
    
    TIDElement* GetVar(std::string name){
        if(!Find(name)){
            throw "You try to get variable, which does not exists";
        }
        for(int i = 0;i < all_.size();i++){
            if(all_[i]->GiveName() == name){
                return all_[i];
            }
        }
        return nullptr;
    }
    ~TID() = default;
};