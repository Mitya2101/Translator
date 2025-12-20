#pragma once
#include "tokens.h"
#include <vector>
#include<iostream>


class TIDElement{
protected:
    std::string name_;
    Types cur_;
public:
    std::string GiveName(){
        return name_; 
    }
    Types GiveType(){
        return cur_;
    }
    TIDElement(std::string name,Types cur):
    name_(name),cur_(cur){
        if(cur_ == Types::VOID){
            throw "you cannot create var of void type";
        }
    }  
    virtual bool operator==(std::string name){
        return name_ == name;        
    }  
};

template<typename T>
class TIDElementArray:public TIDElement{
private:
    std::vector<int> sizes_;
    std::vector<T> arr_;
public:
    TIDElementArray(std::string name,Types cur,std::vector<int> sizes):
    sizes_(sizes),TIDElement(name,cur){
        int tmp = 1;
        for(int i =0 ;i < sizes_.size();i++){
            tmp *= sizes_[i];
        }
        arr_.resize(tmp);
    };
    
    TIDElementArray(std::string name,Types cur,int size_size):
    TIDElement(name,cur){
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
    TIDElementVariable(std::string name,Types cur):
    TIDElement(name,cur){};
    TIDElementVariable(std::string name,Types cur,T num):
    TIDElement(name,cur),num_(num){};
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