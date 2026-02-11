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


Syntaxer::Syntaxer(const std::string& sourceName):lexer(sourceName){
    func.CreateFunc(TFuncElement("read",{new TIDElementVariable<int>("a",Types::INT,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{new TIDElementVariable<bool>("a",Types::BOOL,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{new TIDElementVariable<char>("a",Types::CHAR,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{new TIDElementVariable<double>("a",Types::DOUBLE,0)},
    Types::VOID,nullptr,0,0));

    func.CreateFunc(TFuncElement("print",{new TIDElementVariable<int>("a",Types::INT,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{new TIDElementVariable<bool>("a",Types::BOOL,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{new TIDElementVariable<char>("a",Types::CHAR,0)},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{new TIDElementVariable<double>("a",Types::DOUBLE,0)},
    Types::VOID,nullptr,0,0));
}


SyntaxerNode* Syntaxer::Start(){
    size_counter.push_back(0);
    int ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    tids.push_back({});
    size_counter.push_back(0);
    SyntaxerNode* ty = Program();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back())},ind);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back())});
    size_counter.pop_back();
    poliz.AddEl({POLIZ_Element::END_OF_PROGRAM,""});
    return ty;
}


int Syntaxer::dfs(SyntaxerNode* now){
    if(now->GiveToken().type == Token::Type::IntegerLiteral){
        return std::stoi(now->GiveLexeme());
    }
    if(now->GiveToken().type == Token::Type::Separator){
        dfs(now->GiveChildrens()[0]);
        return dfs(now->GiveChildrens()[1]);
    }
    if(now->GiveToken().type == Token::Type::Operator){
        int left = dfs(now->GiveChildrens()[0]);
        int right = dfs(now->GiveChildrens()[1]);
        if(now->GiveLexeme() == "+"){
            return left + right;
        }
        if(now->GiveLexeme() == "-"){
            return left - right;
        }
        if(now->GiveLexeme() == "*"){
            return left * right;
        }
        if(now->GiveLexeme() == "/"){
            return left / right;
        }
        if(now->GiveLexeme() == "%"){
            return left % right;
        }
        if(now->GiveLexeme() == "=="){
            return left == right;
        }
        if(now->GiveLexeme() == "!="){
            return left != right;
        }
        if(now->GiveLexeme() == "<"){
            return left < right;
        }
        if(now->GiveLexeme() == "<="){
            return left <= right;
        }
        if(now->GiveLexeme() == ">"){
            return left > right;
        }
    }
    if(now->GiveToken().type == Token::Type::OpenBracket){
        assert(0);
        return dfs(now->GiveChildrens()[0]);
    }
    if(now->GiveToken().type == Token::Type::CloseBracket){
        assert(0);
        return dfs(now->GiveChildrens()[0]);
    }
    throw "You try to dfs not constant expression";
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
    GiveType(a.first) + " and " + std::to_string(a.second) + " dimensional" + ", but excepted " +GiveType(b.first) + 
    " and " + std::to_string(b.second) + " dimensional";  
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

    int ind2 = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(0)});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});

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

    int ind_help = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    size_counter.push_back(size_counter.back());

    SyntaxerNode* program = ProgramNoCreateFunction();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},ind_help);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

    int ind3 = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind3 + 2)},ind2);

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

    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},ind3);

    return if_node;
}


SyntaxerNode* Syntaxer::Else(){
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();


    size_counter.push_back(size_counter.back());
    int ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});

    SyntaxerNode* else_node = ProgramNoCreateFunction();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},ind);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

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
        throw "Return was not wrtitten function";
    }

    if(lexer.currentToken().lexeme == ";"){
        lexer.next();
        if(func.Get(InFunction).GiveReturnValue() != Types::VOID){
            throw "Return in function " + func.Get(InFunction).GiveName() + 
            " has different type with return value";
        }
        return return_node;
    }
    SyntaxerNode* tu = Expr();
    if(func.Get(InFunction).GiveReturnValue() != all.back().first 
    || func.Get(InFunction).GiveArraySize() != all.back().second){
        throw "Return in function " + func.Get(InFunction).GiveName() + 
        " has different type with return value";
    }
    return_helper.push_back(poliz.GiveSize());
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    all.pop_back();
    return_node->AddChildren(tu);
    if(lexer.currentToken().lexeme != ";"){
        throw BuildError({";"},lexer.currentToken());
    }
    lexer.next();
    return return_node;
}

SyntaxerNode* Syntaxer::Break(){
    helper_break.back().push_back(poliz.GiveSize());
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

    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

    return break_node;
}

SyntaxerNode* Syntaxer::Continue(){
    continue_helper.back().push_back(poliz.GiveSize());
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

    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

    return continue_node;
}


SyntaxerNode* Syntaxer::While(){
    SyntaxerNode* now = new SyntaxerNode();
    helper_break.push_back({});
    continue_helper.push_back({});

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

    int ind_start = poliz.GiveSize();

    SyntaxerNode* help = Expr();
    now->AddChildren(help);

    int ind2 = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});
    
    
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

    int ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    size_counter.push_back(size_counter.back());

    now->AddChildren(ProgramNoCreateFunction());

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},ind);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

    InCycle = was;
    if(lexer.currentToken().lexeme != "}"){
        throw BuildError({"}"},lexer.currentToken());
    }
    lexer.next();

    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_start)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},ind2);

    for(int i: helper_break.back()){
        poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},i);
    }
    for(int i:continue_helper.back()){
        poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_start)},i);
    }

    helper_break.pop_back();
    continue_helper.pop_back();
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
    lexer.currentToken().lexeme == "double" || 
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


        poliz.AddEl({POLIZ_Element::OPERATION,","});
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
        poliz.AddEl({POLIZ_Element::OPERATION,"="});
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
        poliz.AddEl({POLIZ_Element::OPERATION,"||"});
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
        poliz.AddEl({POLIZ_Element::OPERATION,"&&"});
    }

    return tmp;
}

SyntaxerNode* Syntaxer::ExprEquality(){

    SyntaxerNode* tmp = ExprRel();

    while(lexer.currentToken().lexeme == "==" ||
        lexer.currentToken().lexeme == "!="){
        std::string op = lexer.currentToken().lexeme;
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
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }

    return tmp;
}

SyntaxerNode* Syntaxer::ExprRel(){

    SyntaxerNode* tmp = ExprAdd();

    while(lexer.currentToken().lexeme == "<=" ||
        lexer.currentToken().lexeme == ">=" ||
        lexer.currentToken().lexeme == "<" ||
        lexer.currentToken().lexeme == ">"){
            std::string op = lexer.currentToken().lexeme;
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
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprAdd(){
    SyntaxerNode* tmp = ExprMul();

    while(lexer.currentToken().lexeme == "+" ||
        lexer.currentToken().lexeme == "-"){
            std::string op = lexer.currentToken().lexeme;
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
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    
    return tmp;
}

SyntaxerNode* Syntaxer::ExprMul(){

    SyntaxerNode* tmp = ExprUnary();

    while(lexer.currentToken().lexeme == "*" ||
        lexer.currentToken().lexeme == "%" ||
        lexer.currentToken().lexeme == "/"){
        std::string op = lexer.currentToken().lexeme;
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
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprUnary(){

    if(lexer.currentToken().lexeme == "!" ||
    lexer.currentToken().lexeme == "-" ||
    lexer.currentToken().lexeme == "+"){
        std::string op = lexer.currentToken().lexeme;
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
        poliz.AddEl({POLIZ_Element::UNARY_OPERATION,op});
    }
    return ExprPrimary();
}

SyntaxerNode* Syntaxer::ExprPrimary(){
    SyntaxerNode* tmp = new SyntaxerNode();
    if(lexer.currentToken().lexeme == "("){
        lexer.next();
        if(lexer.currentToken().lexeme == "int"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            SyntaxerNode* cur = Expr();
            if(all.back().second != 0 || all.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            all.pop_back();
            all.push_back({Types::INT,0});
            poliz.AddEl({POLIZ_Element::TO_INT,""});
            return cur;
        }else if(lexer.currentToken().lexeme == "double"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            SyntaxerNode* cur = Expr();
            if(all.back().second != 0 || all.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
            all.pop_back();
            all.push_back({Types::DOUBLE,0});
            return cur;
        }else if(lexer.currentToken().lexeme == "bool"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            SyntaxerNode* cur = Expr(); 
            if(all.back().second != 0 || all.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            all.pop_back();
            all.push_back({Types::BOOL,0});
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
            return cur;
        }else if(lexer.currentToken().lexeme == "char"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            SyntaxerNode* cur = Expr();
            if(all.back().second != 0 || all.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            all.pop_back();
            all.push_back({Types::CHAR,0});
            poliz.AddEl({POLIZ_Element::TO_CHAR,""});
            return cur;
        }
    
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
            std::string tr = tmp->GiveLexeme();
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
            if(tr == "print"){
                poliz.AddEl({POLIZ_Element::CALL_PRINT,""});
                all.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});

            }else if(tr == "read"){
                POLIZ_Element help1 = poliz.GiveEl(poliz.GiveSize() - 1).first;
                if(help1 != POLIZ_Element::ADRESS_BOOL && help1 != POLIZ_Element::ADRESS_CHAR
                     && help1 != POLIZ_Element::ADRESS_DOUBLE && help1 != POLIZ_Element::ADRESS_INT){
                    throw "You can read only on variable";
                }
                poliz.AddEl({POLIZ_Element::CALL_READ,""});
                all.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});

            }else{
                poliz.AddEl({POLIZ_Element::FUNCTION_ADRESS,std::to_string(func.Get(name).GivePolizIndex())});
                poliz.AddEl({POLIZ_Element::CALL_FUNCTION,""});

                all.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});
            }
            return tmp;
        }
        else if(lexer.currentToken().lexeme == "["){
            std::string name = tmp->GiveLexeme();
            
            if(!Find(name)){
                throw BuildSemanticError(name);
            }
            
            int cnt = 0;


            TIDElement* help1 = Give(name);
            std::vector<int> all_sizes;
            int was = 0;
            int only_size = 0;
            if(IsArray<int>(help1) != nullptr){
                was = IsArray<int>(help1)->GiveSizes().size();
                all_sizes = IsArray<int>(help1)->GiveSizes();
                only_size = sizeof(int);
            }
            else if(IsArray<char>(help1) != nullptr){
                was = IsArray<char>(help1)->GiveSizes().size();
                all_sizes = IsArray<char>(help1)->GiveSizes();
                only_size = sizeof(char);
            } 
            else if(IsArray<double>(help1) != nullptr){
                was = IsArray<double>(help1)->GiveSizes().size();
                all_sizes = IsArray<double>(help1)->GiveSizes();
                only_size = sizeof(char);
            }else if(IsArray<bool>(help1) != nullptr){
                was = IsArray<bool>(help1)->GiveSizes().size();
                all_sizes = IsArray<bool>(help1)->GiveSizes();
                only_size = sizeof(bool);
            }else{
                throw name + " is varaiable, but not the array";
            }

            int all1 = 1;
            for(int i: all_sizes){
                all1 *= i;
            }
    

            int cur_ind = 0;

            while(lexer.currentToken().lexeme == "["){
                lexer.next();
                all1 /= all_sizes[cnt];
                cnt++;
                tmp->AddChildren(Expr());
                poliz.AddEl({POLIZ_Element::INT,std::to_string(all1)});
                poliz.AddEl({POLIZ_Element::OPERATION,"*"});
                if(cnt != 1){
                    poliz.AddEl({POLIZ_Element::OPERATION,"+"});
                }
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
            if(was != cnt){
                throw "You can not use array in expressions " + name;
            }
            poliz.AddEl({POLIZ_Element::INT,std::to_string(Give(name)->GiveOffset())});
            poliz.AddEl({POLIZ_Element::OPERATION,"+"});

            if(help1->GiveType() == Types::INT){
                poliz.AddEl({POLIZ_Element::INT,std::to_string(sizeof(int))});
                poliz.AddEl({POLIZ_Element::OPERATION, "*"});
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});
            }
            if(help1->GiveType() == Types::CHAR){
                poliz.AddEl({POLIZ_Element::INT,std::to_string(sizeof(char))});
                poliz.AddEl({POLIZ_Element::OPERATION, "*"});
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});
            }
            if(help1->GiveType() == Types::DOUBLE){
                poliz.AddEl({POLIZ_Element::INT,std::to_string(sizeof(double))});
                poliz.AddEl({POLIZ_Element::OPERATION, "*"});
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
            }
            if(help1->GiveType() == Types::BOOL){
                poliz.AddEl({POLIZ_Element::INT,std::to_string(sizeof(bool))});
                poliz.AddEl({POLIZ_Element::OPERATION, "*"});
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});
            }

           
            
            all.push_back({help1->GiveType(),was - cnt});
            
            return tmp;
        }
        

        if(!Find(tmp->GiveLexeme())){
            throw tmp->GiveLexeme() + " - no such variable";
        }
        TIDElement* help1 = Give(tmp->GiveLexeme());
        // poliz.AddEl({POLIZ_Element::ADRESS,""});
        poliz.AddEl({POLIZ_Element::INT,std::to_string(help1->GiveOffset())});

        if(IsVariable<int>(help1) != nullptr){
            all.push_back({Types::INT,0});
            poliz.AddEl({POLIZ_Element::ADRESS_INT,""});

        }else if(IsVariable<bool>(help1) != nullptr){
            all.push_back({Types::BOOL,0});
            poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});

        }else if(IsVariable<char>(help1) != nullptr){
            all.push_back({Types::CHAR,0});
            poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});

        }else if(IsVariable<double>(help1) != nullptr){
            all.push_back({Types::DOUBLE,0});
            poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});

        }
        return tmp;
    }
    
    if(lexer.currentToken().type == Token::Type::CharLiteral){
        all.push_back({Types::CHAR,0});
        poliz.AddEl({POLIZ_Element::CHAR,lexer.currentToken().lexeme});
    }

    if(lexer.currentToken().type == Token::Type::IntegerLiteral){
        all.push_back({Types::INT,0});
        poliz.AddEl({POLIZ_Element::INT,lexer.currentToken().lexeme});

    }
    if(lexer.currentToken().type == Token::Type::FloatLiteral){
        all.push_back({Types::DOUBLE,0});
        poliz.AddEl({POLIZ_Element::DOUBLE,lexer.currentToken().lexeme});
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
    if (lx == "int" || lx == "double" || lx == "char" || lx == "bool" || lx == "void") {
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

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,std::to_string(size_counter.back())});
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,std::to_string(size_counter.back())});
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_INT,std::to_string(size_counter.back())});
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,std::to_string(size_counter.back())});
                size_counter.back() += sizeof(double);
        }


        SyntaxerNode* expr = Expr();

        poliz.AddEl({POLIZ_Element::OPERATION,"="});
        
        if(StringToType(typeNode->GiveLexeme()) != all.back().first 
        || all.back().second != 0){
            throw BuildSemanticError1(all.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
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
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),Types::BOOL,
                size_counter.back())));
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),Types::CHAR,size_counter.back())));
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),Types::INT,size_counter.back())));
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),Types::DOUBLE,size_counter.back())));
                size_counter.back() += sizeof(double);
        }
        // all.pop_back();
        lexer.next();
        return root;
    }

    // --- МАССИВ: "[" Expr "]" { "[" Expr "]" } ";" ---
    if (lx == "[") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + "- such array has already exists";
        }
        int now_size = 1;
        std::string name = nameNode->GiveLexeme();
        int cnt = 0;
        std::vector<int> sizes;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();
            int tmp = dfs(dimExpr);
            sizes.push_back(tmp);
            dimExpr = new SyntaxerNode();
            dimExpr->UpdateLexeme(std::to_string(tmp));
            dimExpr->UpdateType(Token::Type::IntegerLiteral);
            dimExpr->UpdatePos(lexer.currentToken().pos);

            if(tmp <= 0){
                throw "Array size cannot be negative";
            }
            now_size *= tmp;

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
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(name,
                Types::BOOL,sizes,size_counter.back())));
            size_counter.back() += sizeof(bool) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<double>(name,
                Types::DOUBLE,sizes,size_counter.back())));
            size_counter.back() += sizeof(double) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<int>(name,
                Types::INT,sizes,size_counter.back())));
            size_counter.back() += sizeof(int) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new TIDElementArray<char>(name,
                Types::CHAR,sizes,size_counter.back())));
            size_counter.back() += sizeof(char) * now_size;
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
        return_helper.clear();
    
        // --- ФУНКЦИЯ ---
        if (lx == "(") {
            lexer.next(); // съели '('
    
            SyntaxerNode* params = new SyntaxerNode();
            params->UpdateLexeme("Params");
            params->UpdateType(Token::Type::Identifier);
            params->UpdatePos(lexer.currentToken().pos);
            // необязательный список параметров
            size_counter.push_back(size_counter.back());
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
            int tmp_ind = poliz.GiveSize();
            poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
            poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
            int ind_help = poliz.GiveSize();            
            poliz.AddEl({POLIZ_Element::ALLOCATE,""});
            func.CreateFunc(TFuncElement(nameNode->GiveLexeme(),
            cur_func,Types::VOID,root,0,poliz.GiveSize()));
            
            int was = InFunction;
            InFunction = func.GiveSize() - 1;
            tids.push_back(tids[0]);
            tids.back().push_back(TID());
            for(int i =0 ;i < cur_func.size();i++){
                tids.back().back().CreateVar(cur_func[i]);
            }
            SyntaxerNode* body = ProgramNoCreateFunction();

            poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])},ind_help);
                for(auto i:return_helper){
                    poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
                }
            
            poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])});
            poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},tmp_ind);
            size_counter.pop_back();
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
        size_counter.push_back(size_counter.back());



        if (lexer.currentToken().lexeme != ")") {
            SyntaxerNode* list = ArgListType();
            params->AddChildren(list);
        }

        if(func.Find(cur_function_name_)){
            throw "Function with this name has alreasy exists";
        }


        func.CreateFunc(TFuncElement(nameNode->GiveLexeme(),
            cur_func,StringToType(typeNode->GiveLexeme()),root,th,poliz.GiveSize()));

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
        int tmp_ind = poliz.GiveSize();
        poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
        poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
        int ind = poliz.GiveSize();
        poliz.AddEl({POLIZ_Element::ALLOCATE,""});

        SyntaxerNode* body = ProgramNoCreateFunction();

        poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])},ind);
            for(auto i:return_helper){
                poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
            }
        poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])});
        size_counter.pop_back();
        poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},tmp_ind);
       

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
        throw "Bad array init";
    }

    // --- ОДИНОЧНАЯ ПЕРЕМЕННАЯ: = Expr ; ---
    if (lx == "=") {
        if(Find(nameNode->GiveLexeme())){
            throw nameNode->GiveLexeme() + " - such variable has already exists";
        }

        lexer.next();
        poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});

                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});

                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});


                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back())));
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});

                size_counter.back() += sizeof(double);
        }


        SyntaxerNode* expr = Expr();

        poliz.AddEl({POLIZ_Element::OPERATION,"="});
        
        if(StringToType(typeNode->GiveLexeme()) != all.back().first 
        || all.back().second != 0){
            throw BuildSemanticError1(all.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
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
                new TIDElementVariable<bool>(nameNode->GiveLexeme(),Types::BOOL,size_counter.back())));
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<char>(nameNode->GiveLexeme(),Types::CHAR,size_counter.back())));
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<int>(nameNode->GiveLexeme(),Types::INT,size_counter.back())));
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementVariable<double>(nameNode->GiveLexeme(),Types::DOUBLE, size_counter.back())));     
                size_counter.back() += sizeof(double);
        }
        // all.pop_back();
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
        int now_size = 1;
        std::vector<int> sizes;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();

            int tmp = dfs(dimExpr);
            dimExpr = new SyntaxerNode();
            dimExpr->UpdateLexeme(std::to_string(tmp));
            dimExpr->UpdateType(Token::Type::IntegerLiteral);
            dimExpr->UpdatePos(lexer.currentToken().pos);


            sizes.push_back(tmp);
            if(tmp <= 0){
                throw "size of array should be positive integer";
            }
            now_size *= tmp;
            if(all.back().first != Types::INT || all.back().second != 0){
                throw "size of array should be integer";
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
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>
                (new TIDElementArray<bool>(name,Types::BOOL,sizes,size_counter.back())));
            size_counter.back() += sizeof(bool) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementArray<double>(name,Types::DOUBLE,sizes,size_counter.back())));
            size_counter.back() += sizeof(double) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(dynamic_cast<TIDElement*>(
                new TIDElementArray<int>(name,Types::INT,sizes,size_counter.back())));
            size_counter.back() += sizeof(int) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
        tids.back().back().CreateVar(dynamic_cast<TIDElement*>(new 
            TIDElementArray<char>(name,Types::CHAR,sizes,size_counter.back())));
            size_counter.back() += sizeof(char) * now_size;
        }

        all.push_back({StringToType(typeNode->GiveLexeme()),cnt});


        if(lexer.currentToken().lexeme == "=" && cnt == 1){
            if(typeNode->GiveLexeme() != "char"){
                throw "You can init only char array, but " + nameNode->GiveLexeme() + " - " + typeNode->GiveLexeme() + " array";
            }
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
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<int>(now,tr,cnt,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<double>(now,tr,cnt,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(now,tr,cnt,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<char>(now,tr,cnt,size_counter.back())));
            size_counter.back() += sizeof(int);
        }
    }else{
        Types tr = StringToType(t->GiveLexeme());
        if(t->GiveLexeme() == "int"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<int>(now,tr,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<double>(now,tr,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<bool>(now,tr,size_counter.back())));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<char>(now,tr,size_counter.back())));
            size_counter.back() += sizeof(int);
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
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<int>(now,tr,cnt,size_counter.back())));
                size_counter.back() += sizeof(int);
            }else if(t->GiveLexeme() == "double"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<double>(now,tr,cnt,size_counter.back())));
                size_counter.back() += sizeof(int);
            }else if(t->GiveLexeme() == "bool"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<bool>(now,tr,cnt,size_counter.back())));
                size_counter.back() += sizeof(int);
            }else if(t->GiveLexeme() == "char"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementArray<char>(now,tr,cnt,size_counter.back())));
                size_counter.back() += sizeof(int);
            }
        }else{
            Types tr = StringToType(t->GiveLexeme());
            if(t->GiveLexeme() == "int"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<int>(now,tr,size_counter.back())));
                size_counter.back() += sizeof(int);
            }else if(t->GiveLexeme() == "double"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<double>(now,tr,size_counter.back())));
                size_counter.back() += sizeof(int);

            }else if(t->GiveLexeme() == "bool"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<bool>(now,tr,size_counter.back())));
                
                size_counter.back() += sizeof(int);
            }else if(t->GiveLexeme() == "char"){
                cur_func.push_back(dynamic_cast<TIDElement*>(new TIDElementVariable<char>(now,tr,size_counter.back())));
                
                size_counter.back() += sizeof(int);
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
    int ind_help1 = poliz.GiveSize();
    SyntaxerNode* cond = nullptr;
    if (lexer.currentToken().lexeme != ";") {
        cond = Expr();
    }
    if(all.back().first != Types::BOOL){
        throw "Condition in For should be bool";
    }

    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // ';'


    int ind_condition = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});

    int ind_help = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

    // --- Шаг: [ Expr ] ---

    int ind_expr = poliz.GiveSize();
    SyntaxerNode* step = nullptr;
    if (lexer.currentToken().lexeme != ")") {
        step = Expr();
    }
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_help1)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},ind_help);

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
    int ty = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    size_counter.push_back(size_counter.back());
    SyntaxerNode* body = ProgramNoCreateFunction();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])},ty);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_expr)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},
    ind_condition);

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

std::string to_string(POLIZ_Element e) {
    switch (e) {
        case POLIZ_Element::INT:              return "INT";
        case POLIZ_Element::CHAR:             return "CHAR";
        case POLIZ_Element::BOOL:             return "BOOL";
        case POLIZ_Element::DOUBLE:           return "DOUBLE";
        case POLIZ_Element::OPERATION:        return "OPERATION";
        case POLIZ_Element::POLIZ_GO:          return "POLIZ_GO";
        case POLIZ_Element::POLIZ_FGO:         return "POLIZ_FGO";
        case POLIZ_Element::CALL_FUNCTION:    return "CALL_FUNCTION";
        case POLIZ_Element::FUNCTION_ADRESS:  return "FUNCTION_ADRESS";
        case POLIZ_Element::END_OF_PROGRAM:   return "END_OF_PROGRAM";
        case POLIZ_Element::POLIZ_LABEL:       return "POLIZ_LABEL";
        case POLIZ_Element::ALLOCATE:          return "ALLOCATE";
        case POLIZ_Element::FREE:              return "FREE";
        case POLIZ_Element::ADRESS_INT:        return "ADRESS_INT";
        case POLIZ_Element::ADRESS_BOOL:       return "ADRESS_BOOL";
        case POLIZ_Element::ADRESS_CHAR:       return "ADRESS_CHAR";
        case POLIZ_Element::ADRESS_DOUBLE:    return "ADRESS_DOUBLE";
        case POLIZ_Element::UNARY_OPERATION:  return "UNARY_OPERATION";
        case POLIZ_Element::CALL_PRINT:       return "CALL_PRINT";
        case POLIZ_Element::CALL_READ:        return "CALL_READ";
        case POLIZ_Element::TO_BOOL:          return "TO_BOOL";
        case POLIZ_Element::TO_CHAR:          return "TO_CHAR";
        case POLIZ_Element::TO_DOUBLE:        return "TO_DOUBLE";
        case POLIZ_Element::TO_INT:           return "TO_INT";
        default:                              return "UNKNOWN";
    }
}


void Syntaxer::PrintPoliz(){
    for(int i = 0;i < poliz.GiveSize();i++){
        std::pair<POLIZ_Element,std::string> j = poliz.GiveEl(i);
        std::cout<<i<<' '<<to_string(j.first)<<' '<<j.second<<std::endl;
    }
}
