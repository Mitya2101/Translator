#include "TID.h"

TIDElement::TIDElement(std::string name,Types cur):
name_(name),cur_(cur){
    if(cur_ == Types::VOID){
        throw "you cannot create var of void type";
    }
};
TIDElement::TIDElement(std::string name,Types cur,std::string num):
name_(name),cur_(cur),num_(num){
    if(cur_ == Types::VOID){
        throw "you cannot create var of void type";
    }
};


bool TIDElement::operator==(TIDElement a){
    return a.name_ == name_;
}

bool TIDElement::operator==(std::string name){
    return name_ == name;
}


bool TID::Find(std::string name){
    for(int i = 0;i < all_.size();i++){
        if(all_[i] == name)return true;
    }
    return false;
}

bool TID::Find(TIDElement a){
    for(int i = 0;i < all_.size();i++){
        if(all_[i] == a)return true;
    }
    return false;
}

void TID::CreateVar(TIDElement a){
    if(Find(a)){
        throw "such var has already exists";
    }
    all_.push_back(a);
}

TIDElement TID::GetVar(std::string name){
    for(int i = 0;i < all_.size();i++){
        if(all_[i] == name)return all_[i];
    }
    throw "such var have not exists yet";
}