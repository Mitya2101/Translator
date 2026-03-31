#include "Syntaxer/Syntaxer.h"
#include<iostream>
#include<cassert>

Types StringToType(std::string a){
    if(a == "int"){
        return Types::INT;
    }
    else if(a == "double"){
        return Types::DOUBLE;
    }
    else if(a == "bool"){
        return Types::BOOL;
    }else if(a == "char"){
        return Types::CHAR;
    }
    throw "Unknown type - " + a; 
}

std::string TypeToString(Types a){
    if(a == Types::BOOL){
        return "bool";
    }
    else if(a == Types::CHAR){
        return "char";
    }
    else if(a == Types::DOUBLE){
        return "double";
    }
    else if(a == Types::INT){
        return "int";
    }
    else if(a == Types::VOID){
        return "void";
    }
    return "";
}


Syntaxer::Syntaxer(const std::string& sourceName):lexer(sourceName){
    func.CreateFunc(TFuncElement("read",{std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>("a",Types::INT,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>("a",Types::BOOL,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>("a",Types::CHAR,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("read",{std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>("a",Types::DOUBLE,0,false))},
    Types::VOID,nullptr,0,0));

    func.CreateFunc(TFuncElement("print",{std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>("a",Types::INT,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>("a",Types::BOOL,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>("a",Types::CHAR,0,false))},
    Types::VOID,nullptr,0,0));
    func.CreateFunc(TFuncElement("print",{std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>("a",Types::DOUBLE,0,false))},
    Types::VOID,nullptr,0,0));
}


std::shared_ptr<SyntaxerNode> Syntaxer::Start(){
    int ind = poliz.GiveSize();
    size_counter.push_back(0);
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    tids.push_back({});
    size_counter.push_back(0);
    std::shared_ptr<SyntaxerNode> root_node = Program();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back())},ind);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back())});
    size_counter.pop_back();
    poliz.AddEl({POLIZ_Element::END_OF_PROGRAM,""});
    return root_node;
}


int Syntaxer::dfs(const std::shared_ptr<SyntaxerNode>& now){
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

std::string BuildSemanticError1(std::pair<Types,int> a,std::pair<Types,int> b,const std::shared_ptr<SyntaxerNode>& help){
    return "You have different types in " + std::to_string(help->GivePosition().line) + " line and " + 
    std::to_string(help->GivePosition().line) + ". Have " + 
    GiveType(a.first) + " and " + std::to_string(a.second) + " dimensional" + ", but excepted " +GiveType(b.first) + 
    " and " + std::to_string(b.second) + " dimensional";  
}


std::shared_ptr<SyntaxerNode> Syntaxer::If(){
    std::shared_ptr<SyntaxerNode> if_node(new SyntaxerNode());
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

    std::shared_ptr<SyntaxerNode> expr = Expr();

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

    std::shared_ptr<SyntaxerNode> program = ProgramNoCreateFunction(true);

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
            std::shared_ptr<SyntaxerNode> if_node_1 = If();
            if_node->AddChildren(if_node_1);
        }else if(lexer.currentToken().lexeme == "{"){
            std::shared_ptr<SyntaxerNode> end_node = Else();
            end_node->UpdatePos(now);
            if_node->AddChildren(end_node);
        }else{
            throw BuildError({"if","{"},lexer.currentToken());
        }
    }

    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},ind3);

    return if_node;
}


std::shared_ptr<SyntaxerNode> Syntaxer::Else(){
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();


    size_counter.push_back(size_counter.back());
    int ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});

    std::shared_ptr<SyntaxerNode> else_node = ProgramNoCreateFunction(true);

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


std::shared_ptr<SyntaxerNode> Syntaxer::Return(){
    if(lexer.currentToken().lexeme != "return"){
        throw BuildError({"return"},lexer.currentToken());
    }

    std::shared_ptr<SyntaxerNode> return_node(new SyntaxerNode());

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
        return_helper.push_back(poliz.GiveSize());
        poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
        poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
        return return_node;
    }
    std::shared_ptr<SyntaxerNode> tu = Expr();
    if(func.Get(InFunction).GiveReturnValue() != all.back().first 
    || func.Get(InFunction).GiveArraySize() != all.back().second){
        throw "Return in function " + func.Get(InFunction).GiveName() + 
        " has different type with return value";
    }
    if(poliz.GiveEl(poliz.GiveSize() - 1).first == POLIZ_Element::ADRESS_BOOL){
        poliz.AddEl({POLIZ_Element::TO_BOOL,""});
    }else if(poliz.GiveEl(poliz.GiveSize() - 1).first == POLIZ_Element::ADRESS_CHAR){
        poliz.AddEl({POLIZ_Element::TO_CHAR,""});
    }else if(poliz.GiveEl(poliz.GiveSize() - 1).first == POLIZ_Element::ADRESS_INT){
        poliz.AddEl({POLIZ_Element::TO_INT,""});
    }else if(poliz.GiveEl(poliz.GiveSize() - 1).first == POLIZ_Element::ADRESS_DOUBLE){
        poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
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

std::shared_ptr<SyntaxerNode> Syntaxer::Break(){
    helper_break.back().push_back(poliz.GiveSize());
    if(lexer.currentToken().lexeme != "break"){
        throw BuildError({"break"},lexer.currentToken());
    }
    std::shared_ptr<SyntaxerNode> break_node(new SyntaxerNode());

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

std::shared_ptr<SyntaxerNode> Syntaxer::Continue(){
    continue_helper.back().push_back(poliz.GiveSize());
    if(lexer.currentToken().lexeme != "continue"){
        throw BuildError({"continue"},lexer.currentToken());
    }
    std::shared_ptr<SyntaxerNode> continue_node(new SyntaxerNode());

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


std::shared_ptr<SyntaxerNode> Syntaxer::While(){
    std::shared_ptr<SyntaxerNode> now(new SyntaxerNode());
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

    std::shared_ptr<SyntaxerNode> help = Expr();
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

    now->AddChildren(ProgramNoCreateFunction(true));

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

std::shared_ptr<SyntaxerNode> Syntaxer::Type(){
    if(lexer.currentToken().lexeme != "int" &&
    lexer.currentToken().lexeme != "double" &&
    lexer.currentToken().lexeme != "char" &&
    lexer.currentToken().lexeme != "bool"){
        throw BuildError({"int","bool","char","double"},
            lexer.currentToken());
    }
    std::shared_ptr<SyntaxerNode> now(new SyntaxerNode());
    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    now->UpdatePos(lexer.currentToken().pos);
    lexer.next();
    return now;
}

std::shared_ptr<SyntaxerNode> Syntaxer::Variable(){
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
    std::shared_ptr<SyntaxerNode> now(new SyntaxerNode());
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

std::shared_ptr<SyntaxerNode> Syntaxer::Value(){
    std::shared_ptr<SyntaxerNode> now(new SyntaxerNode());
    if(lexer.currentToken().type != Token::Type::IntegerLiteral){
        throw BuildError({"number"},lexer.currentToken());
    }

    now->UpdateLexeme(lexer.currentToken().lexeme);
    now->UpdateType(lexer.currentToken().type);
    now->UpdatePos(lexer.currentToken().pos);
    lexer.next();

    return now;
}

std::shared_ptr<SyntaxerNode> Syntaxer::DoubleValue(){
    std::shared_ptr<SyntaxerNode> now(new SyntaxerNode());
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


std::shared_ptr<SyntaxerNode> Syntaxer::Expr(){
    std::shared_ptr<SyntaxerNode> tmp = ExprAssign();

    
    while(lexer.currentToken().lexeme == ","){
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        all.pop_back();

        std::shared_ptr<SyntaxerNode> cur = ExprAssign();
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        std::swap(tmp,tmp1);


        poliz.AddEl({POLIZ_Element::OPERATION,","});
    }
    return tmp;
}

template<typename T>
std::shared_ptr<TIDElementArray<T>> IsArray(std::shared_ptr<TIDElement> a){
    return std::dynamic_pointer_cast<TIDElementArray<T>>(a);
}

Types CheckIsEqualTypeArray(std::shared_ptr<TIDElement> a){
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
std::shared_ptr<TIDElementVariable<T>> IsVariable(std::shared_ptr<TIDElement> a){
    return std::dynamic_pointer_cast<TIDElementVariable<T>>(a);
}


Types CheckIsEqualTypeVariable(std::shared_ptr<TIDElement> a){
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



std::shared_ptr<SyntaxerNode> Syntaxer::ExprAssign(){

    std::shared_ptr<SyntaxerNode> tmp = ExprLogicOr();

    while(lexer.currentToken().lexeme == "="){
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprLogicOr();
        assert(all.size() > 1);

        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),all[all.size() - 2],cur);
        }else{
            all.pop_back();
        }
        if(all.back().first == Types::INT){
            poliz.AddEl({POLIZ_Element::TO_INT,""});
        }
        else if(all.back().first == Types::CHAR){
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
        }
        else if(all.back().first == Types::DOUBLE){
            poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
        }
        else if(all.back().first == Types::BOOL){
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
        }
        if(!Find(tmp->GiveLexeme()) 
        || tmp->GiveToken().type != Token::Type::Identifier){
            throw "In right part should be variable or array";
        }
       
        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,"="});
    }

    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprLogicOr(){
    std::shared_ptr<SyntaxerNode> tmp = ExprLogicAnd();

    while(lexer.currentToken().lexeme == "||"){
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
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
        std::shared_ptr<SyntaxerNode> cur = ExprLogicAnd();
        if(all.back().first != Types::BOOL || all.back().second != 0){
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }

        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,"||"});
    }

    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprLogicAnd(){

    std::shared_ptr<SyntaxerNode> tmp = ExprEquality();

    while(lexer.currentToken().lexeme == "&&"){
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
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
        std::shared_ptr<SyntaxerNode> cur = ExprEquality();
        if(all.back().first != Types::BOOL || all.back().second != 0){
            throw BuildSemanticError1(all.back(),{Types::BOOL,0},tmp1);
        }


        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,"&&"});
    }

    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprEquality(){

    std::shared_ptr<SyntaxerNode> tmp = ExprRel();

    while(lexer.currentToken().lexeme == "==" ||
        lexer.currentToken().lexeme == "!="){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprRel();
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
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }

    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprRel(){

    std::shared_ptr<SyntaxerNode> tmp = ExprAdd();

    while(lexer.currentToken().lexeme == "<=" ||
        lexer.currentToken().lexeme == ">=" ||
        lexer.currentToken().lexeme == "<" ||
        lexer.currentToken().lexeme == ">"){
            std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprAdd();
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
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprAdd(){
    std::shared_ptr<SyntaxerNode> tmp = ExprMul();
    while(lexer.currentToken().lexeme == "+" ||
        lexer.currentToken().lexeme == "-"){
            std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprMul();
        assert(all.size() > 1);
        if(all.back() != all[all.size() - 2]){
            throw BuildSemanticError1(all.back(),
            all[all.size() - 2],tmp1);
        }else{
            all.pop_back();
        }


        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    
    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprMul(){

    std::shared_ptr<SyntaxerNode> tmp = ExprUnary();

    while(lexer.currentToken().lexeme == "*" ||
        lexer.currentToken().lexeme == "%" ||
        lexer.currentToken().lexeme == "/"){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> tmp1(new SyntaxerNode());
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
        std::swap(tmp,tmp1);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return tmp;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprUnary(){

    if(lexer.currentToken().lexeme == "!" ||
    lexer.currentToken().lexeme == "-" ||
    lexer.currentToken().lexeme == "+"){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> tmp(new SyntaxerNode());

        tmp->UpdateLexeme(lexer.currentToken().lexeme);
        tmp->UpdateType(lexer.currentToken().type);
        tmp->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprPrimary();
        if(all.back().first == Types::CHAR || 
        all.back().first == Types::DOUBLE || 
        all.back().first == Types::VOID || all.back().second != 0){

            throw BuildSemanticError1(all.back(),{Types::INT,0},tmp);
        }
        tmp->AddChildren(cur);
        poliz.AddEl({POLIZ_Element::UNARY_OPERATION,op});

        return tmp;
    }
    return ExprPrimary();
}



std::shared_ptr<SyntaxerNode> Syntaxer::ExprPrimary(){
    std::shared_ptr<SyntaxerNode> tmp(new SyntaxerNode());
    if(lexer.currentToken().lexeme == "("){
        lexer.next();
        if(lexer.currentToken().lexeme == "int"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            std::shared_ptr<SyntaxerNode> cur = Expr();
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
            std::shared_ptr<SyntaxerNode> cur = Expr();
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
            std::shared_ptr<SyntaxerNode> cur = Expr(); 
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
            std::shared_ptr<SyntaxerNode> cur = Expr();
            if(all.back().second != 0 || all.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            all.pop_back();
            all.push_back({Types::CHAR,0});
            poliz.AddEl({POLIZ_Element::TO_CHAR,""});
            return cur;
        }
    
        std::shared_ptr<SyntaxerNode> cur = Expr();
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
                if (all.empty()) {
                    throw "internal error: type stack underflow in argument list";
                }
                if(tr != "read"){
                    if(all.back().first == Types::INT){
                        poliz.AddEl({POLIZ_Element::TO_INT,""});
                    }else if(all.back().first == Types::DOUBLE){
                        poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
                    }
                    else if(all.back().first == Types::CHAR){
                        poliz.AddEl({POLIZ_Element::TO_CHAR,""});
                    }
                    else if(all.back().first == Types::BOOL){
                        poliz.AddEl({POLIZ_Element::TO_BOOL,""});
                    }
                }
            
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
                     && help1 != POLIZ_Element::ADRESS_DOUBLE && help1 != POLIZ_Element::ADRESS_INT && help1 != POLIZ_Element::GLOBAL_VARIABLE){
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


            std::shared_ptr<TIDElement> help1 = Give(name);
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
                only_size = sizeof(double);
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

            // Индексы массива в POLIZ считаются в элементах, а смещения переменных (offset)
            // хранятся в байтах. Поэтому переводим линейный индекс -> байтовый сдвиг.
            poliz.AddEl({POLIZ_Element::INT, std::to_string(only_size)});
            poliz.AddEl({POLIZ_Element::OPERATION, "*"});

            poliz.AddEl({POLIZ_Element::INT,std::to_string(Give(name)->GiveOffset())});
            poliz.AddEl({POLIZ_Element::OPERATION,"+"});

            

            if(help1->GiveType() == Types::INT){
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});
            }
            if(help1->GiveType() == Types::CHAR){
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});
            }
            if(help1->GiveType() == Types::DOUBLE){
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
            }
            if(help1->GiveType() == Types::BOOL){
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});
            }
            if(help1->IsGlobal()){
                poliz.AddEl({POLIZ_Element::GLOBAL_VARIABLE,""});
            }
           
            
            all.push_back({help1->GiveType(),was - cnt});
            
            return tmp;
        }
        

        if(!Find(tmp->GiveLexeme())){
            throw tmp->GiveLexeme() + " - no such variable";
        }
        std::shared_ptr<TIDElement> help1 = Give(tmp->GiveLexeme());
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
        if(help1->IsGlobal()){
            poliz.AddEl({POLIZ_Element::GLOBAL_VARIABLE,""});
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

std::shared_ptr<SyntaxerNode> Syntaxer::Program() {
    tids.back().push_back(TID());
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    root->UpdateLexeme("Program");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    while (lexer.currentToken().type != Token::Type::EndOfFile) {
        std::shared_ptr<SyntaxerNode> stmt = Statement();
        root->UpdateLexeme("Program");
        root->AddChildren(stmt);
    }
    tids.back().pop_back();
    return root;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ProgramNoCreateFunction(bool need) {
    if(need){
        tids.back().push_back({TID()});
    }
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    root->UpdateLexeme("ProgramNoCreateFunction");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    // тело блока выполняется пока не встретили закрывающую фигурную скобку или EOF
    while (lexer.currentToken().type != Token::Type::EndOfFile &&
           !(lexer.currentToken().lexeme == "}" &&
             lexer.currentToken().type == Token::Type::CloseBracket)) {
        std::shared_ptr<SyntaxerNode> stmt = StatementNoCreationFunction();
        root->AddChildren(stmt);
    }
    if(need){
        tids.back().pop_back();
    }
    return root;
}
std::shared_ptr<SyntaxerNode> Syntaxer::Statement() {

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
    std::shared_ptr<SyntaxerNode> expr = Expr();
    // Expr() should push its resulting type to 'all'. Guard against underflow
    // to avoid UB/heap corruption on malformed programs.
    if (!all.empty()) {
        all.pop_back();
    } else {
        throw "internal error: type stack underflow after expression";
    }



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

std::shared_ptr<TIDElement> Syntaxer::Give(std::string name){
    for(int i =tids.back().size() - 1;i >= 0;i--){
        if(tids.back()[i].Find(name)){
            return tids.back()[i].GetVar(name);
        }
    }
    throw BuildSemanticError(name);
}

std::shared_ptr<SyntaxerNode> Syntaxer::StatementNoCreationFunction() {
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
        std::shared_ptr<SyntaxerNode> decl = CreateVariableOrArray();
        return decl;
    }

    // Просто выражение с ';'
    std::shared_ptr<SyntaxerNode> expr = Expr();
    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    all.pop_back();
    lexer.next();
    return expr;
}

std::shared_ptr<SyntaxerNode> Syntaxer::CreateVariableOrArray() {
    // <Type> <Variable> ("=" <Expr> | ("[" Expr "]", { "[" Expr "]" }))
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    std::shared_ptr<SyntaxerNode> typeNode = Type();
    std::shared_ptr<SyntaxerNode> nameNode = Variable();

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
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
                size_counter.back() += sizeof(double);
        }


        std::shared_ptr<SyntaxerNode> expr = Expr();

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
        // std::cout<<"dhf "<<all.size()<<std::endl;

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(double);
        }
        // Nothing was pushed to 'all' for a plain declaration.
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
            std::shared_ptr<SyntaxerNode> dimExpr = Expr();
            int tmp = dfs(dimExpr);
            sizes.push_back(tmp);
            std::shared_ptr<SyntaxerNode> tr(new SyntaxerNode());
            tr->UpdateLexeme(std::to_string(tmp));
            tr->UpdateType(Token::Type::IntegerLiteral);
            tr->UpdatePos(lexer.currentToken().pos);

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
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>
                    (name,Types::BOOL,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(bool) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>
                    (name,Types::DOUBLE,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(double) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>
                    (name,Types::INT,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>
                    (name,Types::CHAR,sizes,size_counter.back(),InFunction == -1))));
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




std::shared_ptr<SyntaxerNode> Syntaxer::CreateFunctionOrVariableOrArray() {
    // <Type> <Variable> ( "(" ... | "=" Expr ";" | "[" Expr "]" ... ";" )
    if(lexer.currentToken().lexeme == "void"){
        std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
        std::shared_ptr<SyntaxerNode> typeNode(new SyntaxerNode());
        typeNode->UpdateLexeme("void");
        typeNode->UpdateType(lexer.currentToken().type);
        typeNode->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        std::shared_ptr<SyntaxerNode> nameNode = Variable();

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
    
            std::shared_ptr<SyntaxerNode> params(new SyntaxerNode());
            params->UpdateLexeme("Params");
            params->UpdateType(Token::Type::Identifier);
            params->UpdatePos(lexer.currentToken().pos);
            // необязательный список параметров
            int was = InFunction;
            InFunction = func.GiveSize();
            size_counter.push_back(size_counter.back());
            if (lexer.currentToken().lexeme != ")") {
                std::shared_ptr<SyntaxerNode> list = ArgListType();
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
            cur_func,Types::VOID,root,0,poliz.GiveSize() - 1));
            
            tids.push_back(tids[0]);
            tids.back().push_back(TID());
            for(int i =0 ;i < cur_func.size();i++){
                tids.back().back().CreateVar(cur_func[i]);
            }
            std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(true);

            poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])},ind_help);
                for(auto i:return_helper){
                    poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
                }
            
            poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])});
            poliz.AddEl({POLIZ_Element::END_FUNCTION,""});

            poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},tmp_ind);
            size_counter.pop_back();
            // std::cout<<size_counter.back()<<"dhfkjl"<<std::endl;
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
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    
    std::shared_ptr<SyntaxerNode> typeNode = Type();
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
    std::shared_ptr<SyntaxerNode> nameNode = Variable();

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



        std::shared_ptr<SyntaxerNode> params(new SyntaxerNode());
        params->UpdateLexeme("Params");
        params->UpdateType(Token::Type::Identifier);
        params->UpdatePos(lexer.currentToken().pos);

        

        // необязательный список параметров
        size_counter.push_back(size_counter.back());


        int was = InFunction;
        InFunction = func.GiveSize();
        if (lexer.currentToken().lexeme != ")") {
            std::shared_ptr<SyntaxerNode> list = ArgListType();
            params->AddChildren(list);
        }

        if(func.Find(cur_function_name_)){
            throw "Function with this name has alreasy exists";
        }
        int tmp_ind = poliz.GiveSize();
        poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
        poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

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

        
        tids.push_back(tids[0]);
        tids.back().push_back(TID());
        for(int i =0 ;i < cur_func.size();i++){
            tids.back().back().CreateVar(cur_func[i]);
        }
        
        int ind = poliz.GiveSize();
        poliz.AddEl({POLIZ_Element::ALLOCATE,""});

        std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(true);

        poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])},ind);
            for(auto i:return_helper){
                poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
            }
        poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])});
        size_counter.pop_back();
        poliz.AddEl({POLIZ_Element::END_FUNCTION,""});

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
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});

                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});

                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));


                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});

                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));

                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});

                size_counter.back() += sizeof(double);
        }


        std::shared_ptr<SyntaxerNode> expr = Expr();

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
        // 'all' is an expression type stack and may be empty for plain declarations like "int x;".
        // Use the declared type directly.
        const Types declared = StringToType(typeNode->GiveLexeme());

        if(declared == Types::BOOL){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(bool);
        }else if(declared == Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(char);
        }else if(declared == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
        }else if(declared == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));   
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
            std::shared_ptr<SyntaxerNode> dimExpr = Expr();

            int tmp = dfs(dimExpr);
           std::shared_ptr<SyntaxerNode> tr(new SyntaxerNode());
            tr->UpdateLexeme(std::to_string(tmp));
            tr->UpdateType(Token::Type::IntegerLiteral);
            tr->UpdatePos(lexer.currentToken().pos);


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
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>
                    (name,Types::BOOL,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(bool) * now_size;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>
                    (name,Types::DOUBLE,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(double) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>
                    (name,Types::INT,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int) * now_size;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>
                    (name,Types::CHAR,sizes,size_counter.back(),InFunction == -1))));
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
std::shared_ptr<SyntaxerNode> Syntaxer::ArgListType() {
    // std::cout<<InFunction<<std::endl;
    // <Type> <Variable> { "," <Type> <Variable> }
    std::shared_ptr<SyntaxerNode> list(new SyntaxerNode());
    list->UpdateLexeme("ArgListType");
    list->UpdateType(Token::Type::Identifier);
    list->UpdatePos(lexer.currentToken().pos);

    auto makeParam = [this](std::shared_ptr<SyntaxerNode> t, std::shared_ptr<SyntaxerNode> v) {
        std::shared_ptr<SyntaxerNode> p(new SyntaxerNode());
        p->UpdateLexeme("Param");
        p->UpdateType(Token::Type::Identifier);
        p->UpdatePos(t->GiveToken().pos);
        p->AddChildren(t);
        p->AddChildren(v);
        return p;
    };

    std::shared_ptr<SyntaxerNode> t = Type();
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
    std::shared_ptr<SyntaxerNode> v = Variable();
    std::string now = v->GiveLexeme();
    cur_function_name_ += " ";
    cur_function_name_ += t->GiveLexeme();
    v->UpdateLexeme(now);
    if(cnt != 0){
        // Array parameter: we store a pointer (address) in the call frame.
        // Address type in this project is represented as int.
        Types tr = StringToType(t->GiveLexeme());
        if(t->GiveLexeme() == "int"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>(now,tr,cnt,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>(now,tr,cnt,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>(now,tr,cnt,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>(now,tr,cnt,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }
    }else{
        // Scalar parameter: store tightly packed values (byte offsets).
        Types tr = StringToType(t->GiveLexeme());
        if(t->GiveLexeme() == "int"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(now,tr,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(t->GiveLexeme() == "double"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(now,tr,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(double);
        }else if(t->GiveLexeme() == "bool"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(now,tr,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(bool);
        }else if(t->GiveLexeme() == "char"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(now,tr,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(char);
        }
    }
    list->AddChildren(makeParam(t, v));

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        std::shared_ptr<SyntaxerNode> t2 = Type();
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
        std::shared_ptr<SyntaxerNode> v2 = Variable();
        now = v2->GiveLexeme();
        cur_function_name_ += " ";
        cur_function_name_ += t2->GiveLexeme();
        v2->UpdateLexeme(now);
        if(cnt != 0){
            // Array parameter: we store a pointer (address) in the call frame.
            // Address type in this project is represented as int.
            Types tr = StringToType(t2->GiveLexeme());
            if(t2->GiveLexeme() == "int"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>(now,tr,cnt,size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
            }else if(t2->GiveLexeme() == "double"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>(now,tr,cnt,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(t2->GiveLexeme() == "bool"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>(now,tr,cnt,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(t2->GiveLexeme() == "char"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>(now,tr,cnt,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }
        }else{
            // Scalar parameter: store tightly packed values (byte offsets).
            Types tr = StringToType(t2->GiveLexeme());
            if(t2->GiveLexeme() == "int"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(now,tr,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(t2->GiveLexeme() == "double"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(now,tr,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(double);
            }else if(t2->GiveLexeme() == "bool"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(now,tr,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(bool);
            }else if(t2->GiveLexeme() == "char"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>(std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(now,tr,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(char);
            }
        }
        list->AddChildren(makeParam(t2, v2));
    }

    return list;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ArgList() {
    // <ExprAssign> { "," <ExprAssign> }
    std::shared_ptr<SyntaxerNode> list(new SyntaxerNode());
    list->UpdateLexeme("ArgList");
    list->UpdateType(Token::Type::Identifier);
    list->UpdatePos(lexer.currentToken().pos);

    std::shared_ptr<SyntaxerNode> first = ExprAssign();
    list->AddChildren(first);

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        std::shared_ptr<SyntaxerNode> next = ExprAssign();
        list->AddChildren(next);
    }

    return list;
}
std::shared_ptr<SyntaxerNode> Syntaxer::For() {
    if (lexer.currentToken().lexeme != "for") {
        throw BuildError({"for"}, lexer.currentToken());
    }

    std::shared_ptr<SyntaxerNode> node(new SyntaxerNode());
    node->UpdateLexeme("for");
    node->UpdateType(Token::Type::KwFor);
    node->UpdatePos(lexer.currentToken().pos);

    lexer.next(); // 'for'

    if (lexer.currentToken().lexeme != "(") {
        throw BuildError({"("}, lexer.currentToken());
    }
    lexer.next(); // '('
    size_counter.push_back(size_counter.back());
    int alloc = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    // --- Инициализация: [ CreateVariableOrArray | Expr ] ";" ---
    tids.back().push_back({TID()});
    std::shared_ptr<SyntaxerNode> init = nullptr;
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
    std::shared_ptr<SyntaxerNode> cond = nullptr;
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
    std::shared_ptr<SyntaxerNode> step = nullptr;
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
    std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(false);

    tids.back().pop_back();
    
    


    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_expr)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},
    ind_condition);

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])},alloc);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

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
