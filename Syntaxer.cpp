#include "Syntaxer.h"


std::vector<SyntaxerNode*> SyntaxerNode::GiveChildrens(){
    return childrens_;
}

Syntaxer::Syntaxer(const std::string& sourceName):lexer(sourceName){};


SyntaxerNode* Syntaxer::Start(){
    return Program();
}


std::string BuildError(std::vector<std::string> excepted,Token now){
    std::string ans;
    for(int i = 0;i < excepted.size();i++){
        ans += excepted[i];
        if(i != excepted.size() - 1){
            ans += " or ";
        }
    }
    return "Bad lexeme excepted " + ans + ", but received " + now.lexeme + 
    ".In " + std::to_string(now.pos.line) + " line " + std::to_string(now.pos.column) + " colum.";
}

SyntaxerNode* Syntaxer::If(){
    SyntaxerNode* if_node = new SyntaxerNode();
    if_node->UpdateLexeme(lexer.currentToken().lexeme);
    if_node->UpdateType(lexer.currentToken().type);

    if(lexer.currentToken().lexeme != "if"){
        throw BuildError({"if"},lexer.currentToken());
    }
    lexer.next();
    if(lexer.currentToken().lexeme != "("){
        throw BuildError({"("},lexer.currentToken());
    }
    lexer.next();

    SyntaxerNode* expr = Expr();

    if(lexer.currentToken().lexeme != ")"){
        throw BuildError({")"},lexer.currentToken());
    }

    lexer.next();
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();

    
    SyntaxerNode* program = ProgramNoCreateFunction();

    if(lexer.currentToken().lexeme != "}"){
        throw BuildError({"}"},lexer.currentToken());
    }
    lexer.next();

    if_node->AddChildren(expr);
    if_node->AddChildren(program);

   

    if(lexer.currentToken().lexeme == "else"){
        lexer.next();
        if(lexer.currentToken().lexeme == "if"){
            lexer.next();
            SyntaxerNode* if_node_1 = If();
            if_node->AddChildren(if_node_1);
        }else if(lexer.currentToken().lexeme == "{"){
            lexer.next();
            SyntaxerNode* end_node = Else();
            if_node->AddChildren(end_node);
        }else{
            throw BuildError({"if","{"},lexer.currentToken());
        }
    } 
    lexer.next(); 
    return if_node;
}


SyntaxerNode* Syntaxer::Else(){  
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    SyntaxerNode* else_node = new SyntaxerNode();
    // else_node->UpdateLexeme();
    // else_node->UpdateType();??
    lexer.next();

    else_node->AddChildren(ProgramNoCreateFunction());

    if(lexer.currentToken().lexeme != "}"){
        throw BuildError({"}"},lexer.currentToken());
    }
    lexer.next();
    return else_node;
}


SyntaxerNode* Syntaxer::Return(){
    if(lexer.currentToken().lexeme != "return"){
        throw BuildError({"return"},lexer.currentToken());
    }
    SyntaxerNode* return_node = new SyntaxerNode();
    return_node->AddChildren(Expr());

    // return_node->UpdateLexeme();
    // return_node->UpdateType();??

    lexer.next();
    return return_node;
}

SyntaxerNode* Syntaxer::Break(){
    if(lexer.currentToken().lexeme != "break"){
        throw BuildError({"break"},lexer.currentToken());
    }
    SyntaxerNode* break_node = new SyntaxerNode();

    // break_node->UpdateLexeme();
    // break_node->UpdateType();??
    lexer.next();
    return break_node;
}

SyntaxerNode* Syntaxer::Continue(){
    if(lexer.currentToken().lexeme != "continue"){
        throw BuildError({"continue"},lexer.currentToken());
    }
    SyntaxerNode* continue_node = new SyntaxerNode();

    // continue_node->UpdateLexeme();
    // continue_node->UpdateType();??
    lexer.next();
    return continue_node;
}


