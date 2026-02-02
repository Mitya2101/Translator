#include <iostream>
#include "TID.h"
#include "Syntaxer.h"

int main() {
    Syntaxer a("test.txt");
    
    //  TIDElementArray<int>* now = new TIDElementArray<int>("sdjkf",Types::VOID,3); 
    try{
        a.Start();
        
        std::cout<<"OK"<<std::endl;
        a.PrintPoliz();
    }catch(const std::string & a){
        std::cout<<a;
    }catch(const char* msg){
        std::cout<<msg;
    }


    return 0;
}
