#include "TID.h"


TIDElement::TIDElement(std::string name,Types cur):
name_(name),cur_(cur){
    if(cur_ == Types::VOID){
        throw "you cannot create var of void type";
    }
};


std::string TIDElement::GiveName(){
    return name_;
}


Types TIDElement::GiveType(){
    return cur_;
}

bool TIDElement::operator==(std::string name){
    return name_ == name;
}


template<typename T>
TIDElementArray<T>::TIDElementArray(std::string name,
    Types cur,std::vector<int> sizes):
sizes_(sizes),TIDElement(name,cur){
    int tmp = 1;
    for(int i =0 ;i < sizes_.size();i++){
        tmp *= sizes_[i];
    }
    arr_.resize(tmp);
};

template<typename T>
const std::vector<int>& 
TIDElementArray<T>::GiveSizes(){
    return sizes_;
}

template<typename T>
const std::vector<T>& TIDElementArray<T>::GiveArr(){
    return arr_;
} 


template<typename T>
TIDElementArray<T>::TIDElementArray(std::string name,Types cur,int size_size):name(name),cur(cur){
    sizes_.reisze(size_size);
}


template<typename T>
TIDElementVariable<T>::TIDElementVariable(std::string name,Types cur):TIDElement(name,cur){};


template<typename T>
TIDElementVariable<T>::TIDElementVariable(std::string name,Types cur,T num):TIDElement(name,cur),num_(num){};

template<typename T>
const T& TIDElementVariable<T>::GiveNum()
{
    return num_;
}



void TID::CreateVar(TIDElement* a){
    if(Find(a->GiveName())){
        throw "such element has already exists";
    }
    all_.push_back(a);
}

bool TID::Find(std::string name){
    for(int i =0 ;i < all_.size();i++){
        if(all_[i]->GiveName() == name)return true;
    }
    return false;
}

TIDElement* TID::GetVar(std::string name){
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


TID::~TID(){
    for(int i = 0;i < all_.size();i++){
        delete all_[i];
    }
}