#include <iostream>
#include "lexer.h"
#include "Syntaxer.h"

int main() {
    Syntaxer a("test.txt");

    try{
        a.Start();
    }catch(const std::string & a){
    }


    return 0;
}
