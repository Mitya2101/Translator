#include "Syntaxer.h"
#include<iostream>
#include<cassert>

Types StringToType(std::string a){
    if(a == "int"){
        return Types::INT;
    }
    if(a == "double"){
        return Types::DOUBLE;
    }
    if(a == "bool"){
        return Types::BOOL;
    }
    return Types::CHAR;
}

std::string TypeToString(Types a){
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


Syntaxer::Syntaxer(const std::string& sourceName):lexer(sourceName){}


SyntaxerNode* Syntaxer::Start(){
    tids.push_back({});
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


std::string BuildSemanticError(std::string name){
    return "You try to get variable " + name + ", which does not exists";
}

std::string GiveType(Types b){
    if(b == Types::VOID){
        return "void";
    }
    if(b == Types::BOOL){
        return "bool";
    }
    if(b == Types::CHAR){
        return "char";
    }
    if(b == Types::DOUBLE){
        return "double";
    }
    return "int";
}

std::string BuildSemanticError1(std::pair<Types,int> a,std::pair<Types,int> b,SyntaxerNode* help){
    return "You have different types in " + std::to_string(help->GivePosition().line) + " line and " + 
    std::to_string(help->GivePosition().line) + ". Have " + 
    GiveType(a.first) + "and " + std::to_string(a.second) + "dimensional" + ", but excepted " +GiveType(b.first) + 
    "and " + std::to_string(b.second) + "dimensional";  
}


SyntaxerNode* Syntaxer::If(){
    SyntaxerNode* if_node = new SyntaxerNode();
    if_node->UpdateLexeme(lexer.currentToken().lexeme);
    if_node->UpdateType(lexer.currentToken().type);
    if_node->UpdatePos(lexer.currentToken().pos);

    if(lexer.currentToken().lexeme != "if"){
        throw BuildError({"if"},lexer.currentToken());
    }
    lexer.next();
    if(lexer.currentToken().lexeme != "("){
        throw BuildError({"("},lexer.currentToken());
    }
    lexer.next();

    SyntaxerNode* expr = Expr();

    assert(all.size() != 0);

    if(all.back().first != Types::BOOL || all.back().second != 0){
        throw BuildSemanticError1(all.back(),{Types::BOOL,0},if_node);
    }else{
        all.pop_back();
    }

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
        Position now = lexer.currentToken().pos;
        lexer.next();
        if(lexer.currentToken().lexeme == "if"){
            SyntaxerNode* if_node_1 = If();
            if_node->AddChildren(if_node_1);
        }else if(lexer.currentToken().lexeme == "{"){
            SyntaxerNode* end_node = Else();
            end_node->UpdatePos(now);
            if_node->AddChildren(end_node);
        }else{
            throw BuildError({"if","{"},lexer.currentToken());
        }
    }
    return if_node;
}


SyntaxerNode* Syntaxer::Else(){
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();

    SyntaxerNode* else_node = ProgramNoCreateFunction();

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

    return_node->UpdateLexeme("return");
    return_node->UpdateType(Token::Type::KwReturn);
    return_node->UpdatePos(lexer.currentToken().pos);
    lexer.next();

    if(InFunction == -1){
        throw "Return was wrtitten not in function";
    }

    if(lexer.currentToken().lexeme == ";"){
        lexer.next();
        if(func.Get(InFunction).GiveReturnValue() != Types::VOID){
            throw "Return in function " + func.Get(InFunction).GiveName() + 
            "has different type with return value";
        }
        return return_node;
    }
    
    SyntaxerNode* tu = Expr();
    if(func.Get(InFunction).GiveReturnValue() != all.back().first 
    || func.Get(InFunction).GiveArraySize() != all.back().second){
        throw "Return in function " + func.Get(InFunction).GiveName() + 
        "has different type with return value";
    }
    all.pop_back();
    return_node->AddChildren(tu);
    if(lexer.currentToken().lexeme != ";"){
        throw BuildError({";"},lexer.currentToken());
    }
    lexer.next();
    return return_node;
}

SyntaxerNode* Syntaxer::Break(){
    if(lexer.currentToken().lexeme != "break"){
        throw BuildError({"break"},lexer.currentToken());
    }
    SyntaxerNode* break_node = new SyntaxerNode();

    if(!InCycle){
        throw "Break is not in cycle";
    }
    break_node->UpdateLexeme("break");
    break_node->UpdateType(Token::Type::KwBreak);
    break_node->UpdatePos(lexer.currentToken().pos);

    lexer.next();
    if(lexer.currentToken().lexeme != ";"){
        throw BuildError({";"},lexer.currentToken());
    }
    lexer.next();
    return break_node;
}

SyntaxerNode* Syntaxer::Continue(){
    if(lexer.currentToken().lexeme != "continue"){
        throw BuildError({"continue"},lexer.currentToken());
    }
    SyntaxerNode* continue_node = new SyntaxerNode();

    if(!InCycle){
        throw "Continue is not in cycle";
    }

    continue_node->UpdateLexeme("continue");
    continue_node->UpdateType(Token::Type::KwContinue);
    continue_node->UpdatePos(lexer.currentToken().pos);

    lexer.next();

    if(lexer.currentToken().lexeme != ";"){
        throw BuildError({";"},lexer.currentToken());

    }
    lexer.next();
    return continue_node;
}


SyntaxerNode* Syntaxer::While(){
    SyntaxerNode* now = new SyntaxerNode();

    if(lexer.currentToken().lexeme != "while"){
        throw BuildError({"while"},lexer.currentToken());
    }
    now->UpdateLexeme("while");
    now->UpdateType(Token::Type::KwWhile);
    now->UpdatePos(lexer.currentToken().pos);

    lexer.next();
    if(lexer.currentToken().lexeme != "("){
        throw BuildError({"("},lexer.currentToken());
    }
    lexer.next();
    SyntaxerNode* help = Expr();
    now->AddChildren(help);
    
    
    if(all.back().first != Types::BOOL || all.back().second != 0){
        throw BuildSemanticError1(all.back(),{Types::BOOL,0},now);
    }else{
        all.pop_back();
    }

    if(lexer.currentToken().lexeme != ")"){
        throw BuildError({")"},lexer.currentToken());
    }
    lexer.next();

    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();
    bool was = InCycle;
    InCycle = true;
    now->AddChildren(ProgramNoCreateFunction());
    InCycle = was;
    if(lexer.currentToken().lexeme != "}"){
        throw BuildError({"}"},lexer.currentToken());
    }
    lexer.next();


    return now;
}

SyntaxerNode* Syntaxer::Type(){
    if(lexer.currentToken().lexeme != "int" &&
    lexer.currentToken().lexeme != "double" &&
    lexer.currentToken().lexeme != "char" &&
    lexer.currentToken().lexeme != "bool"){
        throw BuildError({"int","bool","char","double"},
            lexer.currentToken());
    }
    SyntaxerNode* now = new SyntaxerNode();
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    now->UpdatePos(lexer.currentToken().pos);
    lexer.next();
    return now;
}

SyntaxerNode* Syntaxer::Variable(){
    if(lexer.currentToken().lexeme == "int" || 
    lexer.currentToken().lexeme == "float" || 
    lexer.currentToken().lexeme == "bool" || 
    lexer.currentToken().lexeme == "void" || 
    lexer.currentToken().lexeme == "for" || 
    lexer.currentToken().lexeme == "while" || 
    lexer.currentToken().lexeme == "if" || 
    lexer.currentToken().lexeme == "else" ||
    lexer.currentToken().lexeme == "print" || 
    lexer.currentToken().lexeme == "read" || 
    lexer.currentToken().lexeme == "return" || 
    lexer.currentToken().lexeme == "break" || 
    lexer.currentToken().lexeme == "continue"
    ){
        throw BuildError({"name of variable"},lexer.currentToken());
    }
    SyntaxerNode* now = new SyntaxerNode();
    if(lexer.currentToken().type == Token::Type::EndOfFile){
        throw BuildError({"digit","char"},lexer.currentToken());
    }
    if(lexer.currentToken().type != Token::Type::Identifier){
        throw BuildError({"digit","char"},lexer.currentToken());
    }
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    now->UpdatePos(lexer.currentToken().pos);
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
    now->UpdatePos(lexer.currentToken().pos);
    lexer.next();

    return now;
}

SyntaxerNode* Syntaxer::DoubleValue(){
    SyntaxerNode* now = new SyntaxerNode();
    if(lexer.currentToken().type != Token::Type::IntegerLiteral &&
    lexer.currentToken().type != Token::Type::FloatLiteral){
        throw BuildError({"number","double number"},lexer.currentToken());
    }
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    now->UpdatePos(lexer.currentToken().pos);
    lexer.next();
    return now;
}


SyntaxerNode* Syntaxer::Expr(){
    SyntaxerNode* tmp = ExprAssign();
    SyntaxerNode* last = tmp;

    while(lexer.currentToken().lexeme == ","){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        all.pop_back();

        SyntaxerNode* cur = ExprAssign();
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        last = cur;
        tmp = tmp1;
    }
    return tmp;
}

template<typename T>
TIDElementArray<T>* IsArray(TIDElement* a){
    return dynamic_cast<TIDElementArray<T>*>(a);
}

Types CheckIsEqualTypeArray(TIDElement* a){
    if(IsArray<bool>(a) != nullptr){
        return Types::BOOL;
    }
    if(IsArray<int>(a) != nullptr){
        return Types::INT;
    }
    if(IsArray<double>(a) != nullptr){
        return Types::DOUBLE;
    }
    if(IsArray<char>(a) != nullptr){
        return Types::CHAR;
    }
    return Types::VOID;
}



template<typename T>
TIDElementVariable<T>* IsVariable(TIDElement* a){
    return dynamic_cast<TIDElementVariable<T>*>(a);
}


Types CheckIsEqualTypeVariable(TIDElement* a){
    if(IsVariable<bool>(a) != nullptr){
        return Types::BOOL;
    }
    if(IsVariable<int>(a) != nullptr){
        return Types::INT;
    }
    if(IsVariable<double>(a) != nullptr){
        return Types::DOUBLE;
    }
    if(IsVariable<char>(a) != nullptr){
        return Types::CHAR;
    }
    return Types::VOID;
}



SyntaxerNode* Syntaxer::ExprAssign(){

    SyntaxerNode* tmp = ExprLogicOr();

    while(lexer.currentToken().lexeme == "="){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        SyntaxerNode* cur = ExprLogicOr();
        assert(all.size() > 1);

        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),all[all.size() - 2],cur);
        }else{
            all.pop_back();
        }
        if(!Find(tmp->GiveLexeme()) 
        || tmp->GiveToken().type != Token::Type::Identifier){
            throw "In right part should be variable or array";
        }
       
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
        tmp1->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        if(all.back().first != Types::BOOL || all.back().second != 0){
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }
        else{
            all.pop_back();
        }
        SyntaxerNode* cur = ExprLogicAnd();
        if(all.back().first != Types::BOOL || all.back().second != 0){
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }

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
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        if(all.back().first != Types::BOOL || all.back().second != 0) {
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }
        else{
            all.pop_back();
        }
        SyntaxerNode* cur = ExprEquality();
        if(all.back().first != Types::BOOL || all.back().second != 0){
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }


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
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        SyntaxerNode* cur = ExprRel();
        assert(all.size() > 1);
        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),all[all.size() - 2],tmp1);
        }else{
            all.pop_back();
            all.pop_back();
            all.push_back({Types::BOOL,0});
        }

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
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        SyntaxerNode* cur = ExprAdd();
        assert(all.size() > 1);
        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),all[all.size() - 2],tmp1);
        }else{
            all.pop_back();
            all.pop_back();
            all.push_back({Types::BOOL,0});
        }


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
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        SyntaxerNode* cur = ExprMul();
        assert(all.size() > 1);
        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),
            all[all.size() - 2],tmp1);
        }else{
            all.pop_back();
        }


        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    
    return tmp;
}

SyntaxerNode* Syntaxer::ExprMul(){

    SyntaxerNode* tmp = ExprUnary();

 

    while(lexer.currentToken().lexeme == "*" ||
        lexer.currentToken().lexeme == "%" ||
        lexer.currentToken().lexeme == "/"){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(ExprUnary());
        assert(all.size() > 1);
        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),
            all[all.size() - 2],tmp1);
        }else{
            all.pop_back();
        }
        tmp = tmp1;
    }
    return tmp;
}




SyntaxerNode* Syntaxer::ExprUnary(){

    if(lexer.currentToken().lexeme == "!" ||
    lexer.currentToken().lexeme == "-" ||
    lexer.currentToken().lexeme == "+"){
        SyntaxerNode* tmp = new SyntaxerNode();

        tmp->UpdateLexeme(lexer.currentToken().lexeme);
        tmp->UpdateType(lexer.currentToken().type);
        tmp->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        SyntaxerNode* cur = ExprPrimary();
        if(all.back().first == Types::CHAR || 
        all.back().first == Types::DOUBLE || 
        all.back().first == Types::VOID || all.back().second != 0){

            throw BuildSemanticError1(all.back(),{Types::INT,0},tmp);
        }
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
    && lexer.currentToken().type != Token::Type::StringLiteral
    && lexer.currentToken().type != Token::Type::Identifier){
        throw BuildError({"(","digit","char"},lexer.currentToken());
    }


    tmp->UpdateLexeme(lexer.currentToken().lexeme);
    tmp->UpdateType(lexer.currentToken().type);
    tmp->UpdatePos(lexer.currentToken().pos);


    if(lexer.currentToken().type == Token::Type::Identifier){
        lexer.next();
        if(lexer.currentToken().lexeme == "("){
            lexer.next();
            std::string name = tmp->GiveLexeme();
            std::vector<Types> param_type;
            while(lexer.currentToken().lexeme != ")"){
                tmp->AddChildren(ExprAssign());
                param_type.push_back(all.back().first);
                name += " ";
                name += TypeToString(all.back().first);
                all.pop_back();
                if(lexer.currentToken().lexeme == ")"){
                    lexer.next();
                    break;
                }
                if(lexer.currentToken().lexeme != ","){
                    throw BuildError({",",")"},lexer.currentToken());
                }
                lexer.next();
            }
            if(!func.Find(name)){
                throw BuildSemanticError(name);
            }
            all.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});
            return tmp;
        }
        else if(lexer.currentToken().lexeme == "["){
            std::string name = tmp->GiveLexeme();
            
            int cnt = 0;
            while(lexer.currentToken().lexeme == "["){
                lexer.next();
                cnt++;
                tmp->AddChildren(Expr());
                if(all.back().first != Types::INT || all.back().second != 0){
                    throw "Array indexes should be integer";
                }else{
                    all.pop_back();
                }
                if(lexer.currentToken().lexeme != "]"){
                    throw BuildError({"]"},lexer.currentToken());
                }
                lexer.next();
            }
            if(!Find(name)){
                throw BuildSemanticError(name);
            }
            TIDElement* help1 = Give(name);
            int was = 0;
            if(IsArray<int>(help1) != nullptr){
                was = IsArray<int>(help1)->GiveSizes().size();
            }

            else if(IsArray<char>(help1) != nullptr){
                was = IsArray<char>(help1)->GiveSizes().size();
            }   

            else if(IsArray<double>(help1) != nullptr){
                was = IsArray<double>(help1)->GiveSizes().size();
            }else if(IsArray<bool>(help1) != nullptr){
                was = IsArray<bool>(help1)->GiveSizes().size();
            }else{
                throw name + "is varaiable, but not the array";
            }
            if(was < cnt){
                throw "Incorrect size of array " + name;
            }
            all.push_back({help1->GiveType(),was - cnt});
            
            return tmp;
        }
        

        if(!Find(tmp->GiveLexeme())){
            throw tmp->GiveLexeme() + " - no such variable";
        }
        TIDElement* help1 = Give(tmp->GiveLexeme());
        if(IsVariable<int>(help1) != nullptr){
            all.push_back({Types::INT,0});
        }else if(IsVariable<bool>(help1) != nullptr){
            all.push_back({Types::BOOL,0});
        }else if(IsVariable<char>(help1) != nullptr){
            all.push_back({Types::CHAR,0});
        }else if(IsVariable<double>(help1) != nullptr){
            all.push_back({Types::DOUBLE,0});
        }else if(IsArray<int>(help1) != nullptr){
            all.push_back({Types::INT,IsArray<int>(help1)->GiveSizes().size()});
        }else if(IsArray<bool>(help1) != nullptr){
            all.push_back({Types::BOOL,IsArray<bool>(help1)->GiveSizes().size()});
        }else if(IsArray<char>(help1) != nullptr){
            all.push_back({Types::CHAR,IsArray<char>(help1)->GiveSizes().size()});
        }else if(IsArray<double>(help1) != nullptr){
            all.push_back({Types::DOUBLE,IsArray<double>(help1)->GiveSizes().size()});
        }else{
            assert(0);
        }
        return tmp;
    }
    

    if(lexer.currentToken().type == Token::Type::CharLiteral){
        all.push_back({Types::CHAR,0});
    }
    if(lexer.currentToken().type == Token::Type::IntegerLiteral){
        all.push_back({Types::INT,0});
    }
    if(lexer.currentToken().type == Token::Type::FloatLiteral){
        all.push_back({Types::DOUBLE,0});
    }

    lexer.next();
    return tmp;
}

SyntaxerNode* Syntaxer::Program() {
    tids.back().push_back(TID());
    SyntaxerNode* root = new SyntaxerNode();
    root->UpdateLexeme("Program");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    while (lexer.currentToken().type != Token::Type::EndOfFile) {
        SyntaxerNode* stmt = Statement();
        root->UpdateLexeme("Program");
        root->AddChildren(stmt);
    }
    tids.back().pop_back();
    return root;
}

SyntaxerNode* Syntaxer::ProgramNoCreateFunction() {
    tids.back().push_back({TID()});
    SyntaxerNode* root = new SyntaxerNode();
    root->UpdateLexeme("ProgramNoCreateFunction");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    // тело блока выполняется пока не встретили закрывающую фигурную скобку или EOF
    while (lexer.currentToken().type != Token::Type::EndOfFile &&
           !(lexer.currentToken().lexeme == "}" &&
             lexer.currentToken().type == Token::Type::CloseBracket)) {
        SyntaxerNode* stmt = StatementNoCreationFunction();
        root->AddChildren(stmt);
    }
    tids.back().pop_back();
    return root;
}
SyntaxerNode* Syntaxer::Statement() {

    const Token& tok = lexer.currentToken();
    const std::string& lx = tok.lexeme;

    if (tok.type == Token::Type::EndOfFile) {
        throw BuildError({"statement"}, tok);
    }

    // if / while / for / return / break / continue

    if (lx == "if")       return If();
    if (lx == "while")    return While();
    if (lx == "for")      return For();
    if (lx == "return")   return Return();
    if (lx == "break")    return Break();
    if (lx == "continue") return Continue();


    // начало объявления функции или переменной/массива
    if (lx == "int" || lx == "float" || lx == "char" || lx == "bool" || lx == "void") {
        return CreateFunctionOrVariableOrArray();
    }


    // иначе — просто выражение с точкой с запятой: <Expr> ";"
    SyntaxerNode* expr = Expr();
    all.pop_back();



    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // съели ';'

    return expr;
}

bool Syntaxer::Find(std::string name){
    for(int i =tids.back().size() - 1;i >= 0;i--){
        if(tids.back()[i].Find(name)){
            return true;
        }
    }
    return false;
}

TIDElement* Syntaxer::Give(std::string name){
    for(int i =tids.back().size() - 1;i >= 0;i--){
        if(tids.back()[i].Find(name)){
            return tids.back()[i].GetVar(name);
        }
    }
    throw BuildSemanticError(name);
}

SyntaxerNode* Syntaxer::StatementNoCreationFunction() {
    const Token& tok = lexer.currentToken();
    const std::string& lx = tok.lexeme;

    if (tok.type == Token::Type::EndOfFile ||
        (tok.type == Token::Type::CloseBracket && lx == "}")) {
        throw BuildError({"statement inside block"}, tok);
    }
    if (lx == "if")       return If();
    if (lx == "while")    return While();
    if (lx == "for")      return For();
    if (lx == "return")   return Return();
    if (lx == "break")    return Break();
    if (lx == "continue") return Continue();

    // Внутри блока запрещено объявлять функции, только переменные/массивы
    if (lx == "int" || lx == "double" || lx == "char" || lx == "bool") {
        SyntaxerNode* decl = CreateVariableOrArray();
        return decl;
    }

    // Просто выражение с ';'
    SyntaxerNode* expr = Expr();
    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    all.pop_back();
    lexer.next();
    return expr;
}

SyntaxerNode* Syntaxer::CreateVariableOrArray() {
    // <Type> <Variable> ("=" <Expr> | ("[" Expr "]", { "[" Expr "]" }))
    SyntaxerNode* root = new SyntaxerNode();
    SyntaxerNode* typeNode = Type();
    SyntaxerNode* nameNode = Variable();

    root->UpdateLexeme("Decl");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(typeNode->GiveToken().pos);
    root->AddChildren(typeNode);
    root->AddChildren(nameNode);

    const std::string& lx = lexer.currentToken().lexeme;    

    // --- ОДИНОЧНАЯ ПЕРЕМЕННАЯ: = Expr ; ---
    if (lx == "=") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + " - such variable has already exists";
        }

        lexer.next();
        SyntaxerNode* expr = Expr();
        if(StringToType(typeNode->GiveLexeme()) != all.back().first 
        || all.back().second != 0){
            throw BuildSemanticError1(all.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
        }
        
        if(all.back().first == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }
        all.pop_back();

        root->AddChildren(expr);

        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return root;
    }
    if(lx == ";"){  
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + "- such variable does not exists";
        }
        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),all.back().first)));
        }
        all.pop_back();
        lexer.next();
        return root;
    }

    // --- МАССИВ: "[" Expr "]" { "[" Expr "]" } ";" ---
    if (lx == "[") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + "- such array has already exists";
        }
        std::string name = nameNode->GiveLexeme();
        int cnt = 0;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();
            if(all.back().first != Types::INT || all.back().second != 0){
                throw nameNode->GiveLexeme() + "integer";
            }
            all.pop_back();
            cnt++;
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
        }

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(name,Types::BOOL,cnt)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<double>(name,Types::DOUBLE,cnt)));

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<int>(name,Types::INT,cnt)));

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<char>(name,Types::CHAR,cnt)));
        }

        all.push_back({StringToType(typeNode->GiveLexeme()),cnt});

        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return root;
    }

    throw BuildError({"=","["}, lexer.currentToken());
}




SyntaxerNode* Syntaxer::CreateFunctionOrVariableOrArray() {
    // <Type> <Variable> ( "(" ... | "=" Expr ";" | "[" Expr "]" ... ";" )
    if(lexer.currentToken().lexeme == "void"){
        SyntaxerNode* root = new SyntaxerNode();
        SyntaxerNode* typeNode = new SyntaxerNode();
        typeNode->UpdateLexeme("void");
        typeNode->UpdateType(lexer.currentToken().type);
        typeNode->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        SyntaxerNode* nameNode = Variable();

        cur_function_name_.clear();
        cur_func.clear();
        cur_function_name_ += nameNode->GiveLexeme();
    
        root->UpdateLexeme("Decl");
        root->UpdateType(Token::Type::Identifier);
        root->UpdatePos(typeNode->GiveToken().pos);
        root->AddChildren(typeNode);
        root->AddChildren(nameNode);
    
        const std::string& lx = lexer.currentToken().lexeme;
    
        // --- ФУНКЦИЯ ---
        if (lx == "(") {
            lexer.next(); // съели '('
    
            SyntaxerNode* params = new SyntaxerNode();
            params->UpdateLexeme("Params");
            params->UpdateType(Token::Type::Identifier);
            params->UpdatePos(lexer.currentToken().pos);
    
            // необязательный список параметров
            if (lexer.currentToken().lexeme != ")") {
                SyntaxerNode* list = ArgListType();
                params->AddChildren(list);
            }

            if(func.Find(cur_function_name_)){
                throw "Function with this name has alreasy exists";
            }

    
            if (lexer.currentToken().lexeme != ")") {
                throw BuildError({")"}, lexer.currentToken());
            }
            lexer.next(); // ')'
    
            if (lexer.currentToken().lexeme != "{") {
                throw BuildError({"{"}, lexer.currentToken());
            }
            lexer.next(); // '{'

            func.CreateFunc(TFuncElement(nameNode->GiveLexeme(),
            cur_func,Types::VOID,root,0));
            
            int was = InFunction;
            InFunction = func.GiveSize() - 1;
            tids.push_back(tids[0]);
            tids.back().push_back(TID());
            for(int i =0 ;i < cur_func.size();i++){
                tids.back().back().CreateVar(cur_func[i]);
            }

            SyntaxerNode* body = ProgramNoCreateFunction();
            InFunction = was;

            tids.pop_back();
    
            if (lexer.currentToken().lexeme != "}") {
                throw BuildError({"}"}, lexer.currentToken());
            }
            lexer.next(); // '}'
    
            root->AddChildren(params);
            root->AddChildren(body);
            return root;
        }else{
            throw BuildError({"("},lexer.currentToken());
        }
    }
    SyntaxerNode* root = new SyntaxerNode();
    
    SyntaxerNode* typeNode = Type();
    int th = 0;
    while(lexer.currentToken().lexeme == "["){
        lexer.next();
        th++;
        if(lexer.currentToken().lexeme != "]"){
            throw "[ - but no ] in line " + std::to_string(lexer.currentToken().pos.line) + 
            "and in " + std::to_string(lexer.currentToken().pos.column) + "column";  
        }
        lexer.next();
    }
    SyntaxerNode* nameNode = Variable();

    root->UpdateLexeme("Decl");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(typeNode->GiveToken().pos);
    root->AddChildren(typeNode);
    root->AddChildren(nameNode);

    const std::string& lx = lexer.currentToken().lexeme;

    // --- ФУНКЦИЯ ---
    if (lx == "(") {

        lexer.next(); // убрали '('

        cur_function_name_.clear();
        cur_func.clear();
        cur_function_name_ += nameNode->GiveLexeme();



        SyntaxerNode* params = new SyntaxerNode();
        params->UpdateLexeme("Params");
        params->UpdateType(Token::Type::Identifier);
        params->UpdatePos(lexer.currentToken().pos);

        

        // необязательный список параметров

        if (lexer.currentToken().lexeme != ")") {
            SyntaxerNode* list = ArgListType();
            params->AddChildren(list);
        }



        if(func.Find(cur_function_name_)){
            throw "Function with this name has alreasy exists";
        }

        func.CreateFunc(TFuncElement(nameNode->GiveLexeme(),
            cur_func,StringToType(typeNode->GiveLexeme()),root,th));

        if (lexer.currentToken().lexeme != ")") {
            throw BuildError({")"}, lexer.currentToken());
        }
        lexer.next(); // ')'

        if (lexer.currentToken().lexeme != "{") {
            throw BuildError({"{"}, lexer.currentToken());
        }
        lexer.next(); // '{'

        int was = InFunction;
        InFunction = func.GiveSize() - 1;
        tids.push_back(tids[0]);
        tids.back().push_back(TID());
        for(int i =0 ;i < cur_func.size();i++){
            tids.back().back().CreateVar(cur_func[i]);
        }
        SyntaxerNode* body = ProgramNoCreateFunction();
        InFunction = was;

        tids.pop_back();

        if (lexer.currentToken().lexeme != "}") {
            throw BuildError({"}"}, lexer.currentToken());
        }
        lexer.next(); // '}'

        root->AddChildren(params);
        root->AddChildren(body);
        return root;
    }
    if(th != 0){
        throw "Bad bracket";
    }

    // --- ОДИНОЧНАЯ ПЕРЕМЕННАЯ: = Expr ; ---
    if (lx == "=") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + " - such variable has already exists";
        }

        lexer.next();
        SyntaxerNode* expr = Expr();
        if(StringToType(typeNode->GiveLexeme()) != all.back().first || all.back().second != 0){
            throw BuildSemanticError1(all.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
        }
        
        if(all.back().first == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),all.back().first)));
        }else if(all.back().first == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }
        all.pop_back();

        root->AddChildren(expr);

        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return root;
    }
    if(lx == ";"){  
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + "- such variable does not exists";
        }
        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),all.back().first)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),all.back().first)));
        }
        all.pop_back();
        lexer.next();
        return root;
    }

    // --- МАССИВ: "[" Expr "]" { "[" Expr "]" } ";" ---
    if (lx == "[") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + "- such array has already exists";
        }
        std::string name = nameNode->GiveLexeme();
        int cnt = 0;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();
            if(all.back().first != Types::INT || all.back().second != 0){
                throw nameNode->GiveLexeme() + "integer";
            }
            all.pop_back();
            cnt++;
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
        }

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(name,Types::BOOL,cnt)));
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<double>(name,Types::DOUBLE,cnt)));

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<int>(name,Types::INT,cnt)));

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<char>(name,Types::CHAR,cnt)));
        }

        all.push_back({StringToType(typeNode->GiveLexeme()),cnt});


        if(lexer.currentToken().lexeme == "=" && cnt == 1){
            lexer.next();
            if(lexer.currentToken().type != Token::Type::StringLiteral){
                throw BuildError({"String literal"},lexer.currentToken());
            }
            lexer.next();
        }

        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return root;
    }

    throw BuildError({"(","=","["}, lexer.currentToken());
}
SyntaxerNode* Syntaxer::ArgListType() {
    // <Type> <Variable> { "," <Type> <Variable> }
    SyntaxerNode* list = new SyntaxerNode();
    list->UpdateLexeme("ArgListType");
    list->UpdateType(Token::Type::Identifier);
    list->UpdatePos(lexer.currentToken().pos);

    auto makeParam = [this](SyntaxerNode* t, SyntaxerNode* v) {
        SyntaxerNode* p = new SyntaxerNode();
        p->UpdateLexeme("Param");
        p->UpdateType(Token::Type::Identifier);
        p->UpdatePos(t->GiveToken().pos);
        p->AddChildren(t);
        p->AddChildren(v);
        return p;
    };

    SyntaxerNode* t = Type();
    int cnt = 0;
    while(lexer.currentToken().lexeme == "["){
        lexer.next();
        if(lexer.currentToken().lexeme == "]"){
            cnt++;
        }else{
            throw BuildError({"["},lexer.currentToken());
        }
        lexer.next();
    }
    SyntaxerNode* v = Variable();
    std::string now = v->GiveLexeme();
    cur_function_name_ += " ";
    cur_function_name_ += t->GiveLexeme();
    v->UpdateLexeme(now);
    if(cnt != 0){
        Types tr = StringToType(t->GiveLexeme());
        if(t->GiveLexeme() == "int"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<int>(now,tr,cnt)));
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<double>(now,tr,cnt)));
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(now,tr,cnt)));
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<char>(now,tr,cnt)));
        }
    }else{
        Types tr = StringToType(t->GiveLexeme());
        if(t->GiveLexeme() == "int"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<int>(now,tr)));
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<double>(now,tr)));
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<bool>(now,tr)));
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<char>(now,tr)));
        }
    }
    list->AddChildren(makeParam(t, v));

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        SyntaxerNode* t2 = Type();
        cnt = 0;
        while(lexer.currentToken().lexeme == "["){
            lexer.next();
            if(lexer.currentToken().lexeme == "]"){
                cnt++;
            }else{
                throw BuildError({"["},lexer.currentToken());
            }
            lexer.next();
        }
        SyntaxerNode* v2 = Variable();
        now = v2->GiveLexeme();
        cur_function_name_ += " ";
        cur_function_name_ += t2->GiveLexeme();
        v->UpdateLexeme(now);
        if(cnt != 0){
            Types tr = StringToType(t->GiveLexeme());
            if(t->GiveLexeme() == "int"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<int>(now,tr,cnt)));
            }else if(t->GiveLexeme() == "double"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<double>(now,tr,cnt)));
            }else if(t->GiveLexeme() == "bool"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(now,tr,cnt)));
            }else if(t->GiveLexeme() == "char"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<char>(now,tr,cnt)));
            }
        }else{
            Types tr = StringToType(t->GiveLexeme());
            if(t->GiveLexeme() == "int"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<int>(now,tr)));
            }else if(t->GiveLexeme() == "double"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<double>(now,tr)));
            }else if(t->GiveLexeme() == "bool"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<bool>(now,tr)));
            }else if(t->GiveLexeme() == "char"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<char>(now,tr)));
            }
        }
        list->AddChildren(makeParam(t2, v2));
    }

    return list;
}

SyntaxerNode* Syntaxer::ArgList() {
    // <ExprAssign> { "," <ExprAssign> }
    SyntaxerNode* list = new SyntaxerNode();
    list->UpdateLexeme("ArgList");
    list->UpdateType(Token::Type::Identifier);
    list->UpdatePos(lexer.currentToken().pos);

    SyntaxerNode* first = ExprAssign();
    list->AddChildren(first);

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        SyntaxerNode* next = ExprAssign();
        list->AddChildren(next);
    }

    return list;
}
SyntaxerNode* Syntaxer::For() {
    if (lexer.currentToken().lexeme != "for") {
        throw BuildError({"for"}, lexer.currentToken());
    }

    SyntaxerNode* node = new SyntaxerNode();
    node->UpdateLexeme("for");
    node->UpdateType(Token::Type::KwFor);
    node->UpdatePos(lexer.currentToken().pos);

    lexer.next(); // 'for'

    if (lexer.currentToken().lexeme != "(") {
        throw BuildError({"("}, lexer.currentToken());
    }
    lexer.next(); // '('

    // --- Инициализация: [ CreateVariableOrArray | Expr ] ";" ---
    SyntaxerNode* init = nullptr;
    if (lexer.currentToken().lexeme != ";") {
        const std::string& lx = lexer.currentToken().lexeme;
        if (lx == "int" || lx == "double" || lx == "char" || lx == "bool") {
            init = CreateVariableOrArray();
        } else {
            init = Expr();
            if (lexer.currentToken().lexeme != ";") {
                throw BuildError({";"}, lexer.currentToken());
            }
            lexer.next(); // ';'

        }
    }

    // --- Условие: [ Expr ] ";" ---
    SyntaxerNode* cond = nullptr;
    if (lexer.currentToken().lexeme != ";") {
        cond = Expr();
    }

    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // ';'

    // --- Шаг: [ Expr ] ---
    SyntaxerNode* step = nullptr;
    if (lexer.currentToken().lexeme != ")") {
        step = Expr();
    }

    if (lexer.currentToken().lexeme != ")") {
        throw BuildError({")"}, lexer.currentToken());
    }
    lexer.next(); // ')'

    if (lexer.currentToken().lexeme != "{") {
        throw BuildError({"{"}, lexer.currentToken());
    }
    lexer.next(); // '{'

    int was = InCycle;
    InCycle = true;
    SyntaxerNode* body = ProgramNoCreateFunction();
    InCycle = was;
    if (lexer.currentToken().lexeme != "}") {
        throw BuildError({"}"}, lexer.currentToken());
    }
    lexer.next(); // '}'

    if (init) node->AddChildren(init);
    if (cond) node->AddChildren(cond);
    if (step) node->AddChildren(step);
    node->AddChildren(body);

    return node;
}
