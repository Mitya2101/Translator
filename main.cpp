#include <iostream>
#include "lexer.h"
#include "Syntaxer.h"

int main() {
    Syntaxer a("test.txt");

    try{
        a.Start();
        std::cout<<"OK";
    }catch(const std::string & a){
        std::cout<<a;
    }


    return 0;
}
