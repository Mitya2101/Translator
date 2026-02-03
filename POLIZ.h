#pragma once
#include<vector>


enum POLIZ_Element{
    NUM,//
    OPERATION,//
    SYMBOL,//
    POLIZ_GO,//
    POLIZ_FGO,//
    CALL_FUNCTION,//
    FUNCTION_ADRESS,//
    END_OF_PROGRAM,//
    POLIZ_LABEL,//
    ALLOCATE,//
    FREE,//
    UNARY_OPERATION,
    CALL_ARRAY
};


class POLIZ{
    std::vector<std::pair<POLIZ_Element,std::string>> poliz_;
public:
    void UpdateEl(std::pair<POLIZ_Element,std::string> s,int ind){
        if(poliz_.size() <= ind || ind < 0){
            throw "Inccoect index";
        }
        poliz_[ind] = s;
    }

    void AddEl(std::pair<POLIZ_Element,std::string> s){
        poliz_.push_back(s);
    }

    std::pair<POLIZ_Element,std::string> GiveEl(int ind){
        if(poliz_.size() <= ind || ind < 0){
            throw "Inccoect index";
        }
        return poliz_[ind];
    }
    int GiveSize(){
        return poliz_.size();
    }
};

