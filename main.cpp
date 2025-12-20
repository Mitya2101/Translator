#include <iostream>
#include "TID.h"
#include "Syntaxer.h"

int main() {
    Syntaxer a("test.txt");
    //  TIDElementArray<int>* now = new TIDElementArray<int>("sdjkf",Types::VOID,3); 
    try{
        a.Start();
        std::cout<<"OK";
    }catch(const std::string & a){
        std::cout<<a;
    }


    return 0;
}
