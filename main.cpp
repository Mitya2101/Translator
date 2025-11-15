#include <iostream>
#include "lexer.h"
#include "tokens.h"

int main() {
    Lexer lexer("test.txt", "keywords.txt");

    while (true) {
        const Token& tok = lexer.currentToken();

        std::cout << tok.toString() << '\n';

        if (tok.type == Token::Type::EndOfFile) {
            break;
        }

        lexer.next();
    }

    return 0;
}
