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
    else_node->UpdateLexeme("else");
    else_node->UpdateType(Token::Type::KwElse);
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

    return_node->UpdateLexeme("return");
    return_node->UpdateType(Token::Type::KwReturn);

    lexer.next();
    return return_node;
}

SyntaxerNode* Syntaxer::Break(){
    if(lexer.currentToken().lexeme != "break"){
        throw BuildError({"break"},lexer.currentToken());
    }
    SyntaxerNode* break_node = new SyntaxerNode();

    break_node->UpdateLexeme("break");
    break_node->UpdateType(Token::Type::KwBreak);
    lexer.next();
    return break_node;
}

SyntaxerNode* Syntaxer::Continue(){
    if(lexer.currentToken().lexeme != "continue"){
        throw BuildError({"continue"},lexer.currentToken());
    }
    SyntaxerNode* continue_node = new SyntaxerNode();

    continue_node->UpdateLexeme("continue");
    continue_node->UpdateType(Token::Type::KwContinue);
    lexer.next();
    return continue_node;
}


SyntaxerNode* Syntaxer::While(){
    if(lexer.currentToken().lexeme != "while"){
        throw BuildError({"while"},lexer.currentToken());
    }
    lexer.next();
    if(lexer.currentToken().lexeme != "("){
        throw BuildError({"("},lexer.currentToken());
    }
    lexer.next();
    SyntaxerNode* now = new SyntaxerNode();
    now->AddChildren(Expr());
    if(lexer.currentToken().lexeme != ")"){
        throw BuildError({")"},lexer.currentToken());
    }
    lexer.next();

    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();
    now->AddChildren(ProgramNoCreateFunction());
    if(lexer.currentToken().lexeme != "}"){
        throw BuildError({"}"},lexer.currentToken());
    }
    lexer.next();
    
    now->UpdateLexeme("while");
    now->UpdateType(Token::Type::KwWhile);
    return now;
}






SyntaxerNode* Syntaxer::Type(){
    if(lexer.currentToken().lexeme != "int" && 
    lexer.currentToken().lexeme != "double" && 
    lexer.currentToken().lexeme != "char" && 
    lexer.currentToken().lexeme != "bool"){
        throw BuildError({"int","bool","char","double"},lexer.currentToken());
    }
    SyntaxerNode* now = new SyntaxerNode();
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    lexer.next();
    return now;
}

SyntaxerNode* Syntaxer::Variable(){
    SyntaxerNode* now = new SyntaxerNode();
    if(lexer.currentToken().type == Token::Type::EndOfFile){
        throw BuildError({"digit","char"},lexer.currentToken());
    }   
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    lexer.next();
    return now;
}

SyntaxerNode* Syntaxer::Value(){
    SyntaxerNode* now = new SyntaxerNode();
    if(lexer.currentToken().type != Token::Type::IntegerLiteral){
        throw BuildError({"number"},lexer.currentToken());
    }

    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    lexer.next();

    return now;
}

SyntaxerNode* Syntaxer::DoubleValue(){
    SyntaxerNode* now = new SyntaxerNode();
    if(lexer.currentToken().type != Token::Type::IntegerLiteral ||
    lexer.currentToken().type != Token::Type::FloatLiteral){
        throw BuildError({"number","double number"},lexer.currentToken());
    }
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    lexer.next();
    return now;
}


SyntaxerNode* Syntaxer::Expr(){
    SyntaxerNode* tmp = ExprAssign();

    while(lexer.currentToken().lexeme == ","){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        lexer.next();

        SyntaxerNode* cur = ExprAssign();

       
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprAssign(){
    SyntaxerNode* tmp = ExprLogicOr();

    while(lexer.currentToken().lexeme == "="){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprLogicOr();

        
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}


SyntaxerNode* Syntaxer::ExprLogicOr(){
    SyntaxerNode* tmp = ExprLogicAnd();

    while(lexer.currentToken().lexeme == "||"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprLogicAnd();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}


SyntaxerNode* Syntaxer::ExprLogicAnd(){
    SyntaxerNode* tmp = ExprEquality();

    while(lexer.currentToken().lexeme == "&&"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprEquality();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprEquality(){
    SyntaxerNode* tmp = ExprRel();

    while(lexer.currentToken().lexeme == "==" || 
        lexer.currentToken().lexeme == "!="){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprRel();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}


SyntaxerNode* Syntaxer::ExprRel(){
    SyntaxerNode* tmp = ExprAdd();

    while(lexer.currentToken().lexeme == "<=" || 
        lexer.currentToken().lexeme == ">=" || 
        lexer.currentToken().lexeme == "<" || 
        lexer.currentToken().lexeme == ">"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprAdd();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}


SyntaxerNode* Syntaxer::ExprAdd(){
    SyntaxerNode* tmp = ExprMul();

    while(lexer.currentToken().lexeme == "+" || 
        lexer.currentToken().lexeme == "-"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprMul();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprMul(){
    SyntaxerNode* tmp = ExprPostfix();

    while(lexer.currentToken().lexeme == "*" || 
        lexer.currentToken().lexeme == "%" || 
        lexer.currentToken().lexeme == "/"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);

        lexer.next();
        SyntaxerNode* cur = ExprPostfix();
        
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprPostfix(){
    SyntaxerNode* tmp = ExprUnary();

    if(lexer.currentToken().lexeme == "("){
        while(lexer.currentToken().lexeme != ")"){
            tmp->AddChildren(ExprAssign());
            if(lexer.currentToken().lexeme == ")"){
                break;
            }
            if(lexer.currentToken().lexeme != ","){
                throw BuildError({",",")"},lexer.currentToken());
            }
            lexer.next();
        }
        return tmp;
    }
    //вызов функции или обращение к массиву?
}


SyntaxerNode* Syntaxer::ExprUnary(){
    SyntaxerNode* tmp = new SyntaxerNode();
    if(lexer.currentToken().lexeme == "!" || 
    lexer.currentToken().lexeme == "-" || 
    lexer.currentToken().lexeme == "+"){
        tmp->UpdateLexeme(lexer.currentToken().lexeme);
        tmp->UpdateType(lexer.currentToken().type);
        lexer.next();
        SyntaxerNode* cur = ExprPrimary();
        tmp->AddChildren(cur);
        return tmp;
    }
    return ExprPrimary();
}


SyntaxerNode* Syntaxer::ExprPrimary(){
    SyntaxerNode* tmp = new SyntaxerNode();
    if(lexer.currentToken().lexeme == "("){
        lexer.next();
        SyntaxerNode* cur = Expr();
        if(lexer.currentToken().lexeme != ")"){
            throw BuildError({")"},lexer.currentToken());
        }
        lexer.next();
        return cur;
    }
    if(lexer.currentToken().type != Token::Type::IntegerLiteral 
    && lexer.currentToken().type != Token::Type::FloatLiteral
    && lexer.currentToken().type != Token::Type::CharLiteral
    && lexer.currentToken().type != Token::Type::StringLiteral){
        throw BuildError({"(","digit","char"},lexer.currentToken());
    }
    tmp->UpdateLexeme(lexer.currentToken().lexeme);
    tmp->UpdateType(lexer.currentToken().type);
    return tmp;
}



