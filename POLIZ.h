#pragma once
#include<vector>
#include <string>


enum class POLIZ_Element{
    INT,//
    CHAR,
    BOOL,
    DOUBLE,
    OPERATION,//
    POLIZ_GO,//
    POLIZ_FGO,//
    CALL_FUNCTION,//
    FUNCTION_ADRESS,//
    END_OF_PROGRAM,//
    POLIZ_LABEL,//
    ALLOCATE,//
    FREE,//
    ADRESS_INT,
    ADRESS_BOOL,
    ADRESS_CHAR,
    ADRESS_DOUBLE,
    UNARY_OPERATION,
    CALL_PRINT,
    CALL_READ,

    TO_DOUBLE,
    TO_INT,
    TO_CHAR,
    TO_BOOL,
    END_FUNCTION
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

