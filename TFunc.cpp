#include "TFunc.h"

std::string TypeToString1(Types a){
    if(a == Types::BOOL){
        return "bool";
    }
    if(a == Types::CHAR){
        return "char";
    }
    if(a == Types::DOUBLE){
        return "double";
    }
    if(a == Types::INT){
        return "int";
    }
    if(a == Types::VOID){
        return "void";
    }
    return "";
}

std::string UpdateName(std::string name,std::vector<TIDElement*> param){
    name += " ";
    for(int i = 0;i < param.size();i++){
        name += TypeToString1(param[i]->GiveType());
        name += " ";
    }
    name.pop_back();
    return name;
}


TFuncElement::TFuncElement(std::string name,
    std::vector<TIDElement*> param_types,Types return_value,SyntaxerNode* create,int size,int poliz_index)
    :name_(UpdateName(name,param_types)),
param_types_(param_types),return_value_(return_value),create_(create),size_(size),poliz_index(poliz_index){};

int TFuncElement::GivePolizIndex(){
    return poliz_index;
}

std::string TFuncElement::GiveName(){
    return name_;
}

std::vector<TIDElement*> TFuncElement::GiveParam(){
    return param_types_;
}

Types TFuncElement::GiveReturnValue(){
    return return_value_;
}

SyntaxerNode* TFuncElement::GiveCreate(){
    return create_;
}


bool TFunc::Find(std::string name){
    for(int i =0 ;i < all_.size();i++){
        if(all_[i].GiveName() == name){
            return true;
        }
    }
    return false;
}

void TFunc::CreateFunc(TFuncElement a){
    if(Find(a.GiveName())){
        throw "Such function has already exists";
    }
    all_.push_back(a);
}


TFuncElement TFunc::Get(int ind){
    if(all_.size() <= ind){
        throw "Get in TFunc";
    }
    return all_[ind];
}

int TFuncElement::GiveArraySize(){
    return size_;
}

TFuncElement TFunc::Get(std::string name){
    if(!Find(name)){
        throw "Such function have not exists yet";
    }
    for(int i =0 ;i < all_.size();i++){
        if(all_[i].GiveName() == name){
            return all_[i];
        }
    }
    throw "Incorrect find";
}


int TFunc::GiveSize(){
    return all_.size();
}