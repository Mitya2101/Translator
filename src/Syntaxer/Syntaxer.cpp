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
    int ind_start = poliz.GiveSize();
    size_counter.push_back(0);
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    stack_tids.push_back({});
    size_counter.push_back(0);
    std::shared_ptr<SyntaxerNode> root_node = Program();

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back())},ind_start);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back())});
    size_counter.pop_back();
    poliz.AddEl({POLIZ_Element::END_OF_PROGRAM,""});
    return root_node;
}


int Syntaxer::CompileTimeCalculation(const std::shared_ptr<SyntaxerNode>& now){
    if(now->GiveToken().type == Token::Type::IntegerLiteral){
        return std::stoi(now->GiveLexeme());
    }
    if(now->GiveToken().type == Token::Type::Separator){
        CompileTimeCalculation(now->GiveChildrens()[0]);
        return CompileTimeCalculation(now->GiveChildrens()[1]);
    }
    if(now->GiveToken().type == Token::Type::Operator){
        int left = CompileTimeCalculation(now->GiveChildrens()[0]);
        int right = CompileTimeCalculation(now->GiveChildrens()[1]);
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
        return CompileTimeCalculation(now->GiveChildrens()[0]);
    }
    if(now->GiveToken().type == Token::Type::CloseBracket){
        return CompileTimeCalculation(now->GiveChildrens()[0]);
    }
    throw "You try to compile time with not constant expression";
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

    int poliz_fgo_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(0)});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});

    assert(stack_calculate_simul.size() != 0);

    if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0){
        throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},if_node);
    }else{
        stack_calculate_simul.pop_back();
    }

    if(lexer.currentToken().lexeme != ")"){
        throw BuildError({")"},lexer.currentToken());
    }

    lexer.next();
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();

    int poliz_allocate_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    size_counter.push_back(size_counter.back());

    std::shared_ptr<SyntaxerNode> program = ProgramNoCreateFunction(true);

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},poliz_allocate_ind);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

    int poliz_go_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz_go_ind + 2)},poliz_fgo_ind);

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

    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},poliz_go_ind);

    return if_node;
}


std::shared_ptr<SyntaxerNode> Syntaxer::Else(){
    if(lexer.currentToken().lexeme != "{"){
        throw BuildError({"{"},lexer.currentToken());
    }
    lexer.next();


    size_counter.push_back(size_counter.back());
    int ind_allocate = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});

    std::shared_ptr<SyntaxerNode> else_node = ProgramNoCreateFunction(true);

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},ind_allocate);
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
    if(func.Get(InFunction).GiveReturnValue() != stack_calculate_simul.back().first 
    || func.Get(InFunction).GiveArraySize() != stack_calculate_simul.back().second){
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
    stack_calculate_simul.pop_back();
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

    int poliz_fgo_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});
    
    
    if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0){
        throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},now);
    }else{
        stack_calculate_simul.pop_back();
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

    int poliz_allocate_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::ALLOCATE,""});
    size_counter.push_back(size_counter.back());

    now->AddChildren(ProgramNoCreateFunction(true));

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
        size_counter[size_counter.size() - 2])},poliz_allocate_ind);
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

    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},poliz_fgo_ind);

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
    std::shared_ptr<SyntaxerNode> assign_part = ExprAssign();

    
    while(lexer.currentToken().lexeme == ","){
        std::shared_ptr<SyntaxerNode> expt_part(new SyntaxerNode());
        expt_part->UpdateLexeme(lexer.currentToken().lexeme);
        expt_part->UpdateType(lexer.currentToken().type);
        expt_part->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        stack_calculate_simul.pop_back();

        std::shared_ptr<SyntaxerNode> cur = ExprAssign();
        expt_part->AddChildren(assign_part);
        expt_part->AddChildren(cur);
        std::swap(assign_part,expt_part);


        poliz.AddEl({POLIZ_Element::OPERATION,","});
    }
    return assign_part;
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

    std::shared_ptr<SyntaxerNode> logic_or_part = ExprLogicOr();

    while(lexer.currentToken().lexeme == "="){
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprLogicOr();
        assert(stack_calculate_simul.size() > 1);

        if(stack_calculate_simul.back() != 
        stack_calculate_simul[stack_calculate_simul.size() - 2]){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            stack_calculate_simul[stack_calculate_simul.size() - 2],cur);
        }else{
            stack_calculate_simul.pop_back();
        }
        if(stack_calculate_simul.back().first == Types::INT){
            poliz.AddEl({POLIZ_Element::TO_INT,""});
        }
        else if(stack_calculate_simul.back().first == Types::CHAR){
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
        }
        else if(stack_calculate_simul.back().first == Types::DOUBLE){
            poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
        }
        else if(stack_calculate_simul.back().first == Types::BOOL){
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
        }
        if(!Find(logic_or_part->GiveLexeme()) 
        || logic_or_part->GiveToken().type != Token::Type::Identifier){
            throw "In right part should be variable or array";
        }
       
        expr_part->AddChildren(logic_or_part);
        expr_part->AddChildren(cur);
        std::swap(expr_part,logic_or_part);
        poliz.AddEl({POLIZ_Element::OPERATION,"="});
    }

    return logic_or_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprLogicOr(){
    std::shared_ptr<SyntaxerNode> logic_and_part = ExprLogicAnd();

    while(lexer.currentToken().lexeme == "||"){
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0){
            throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},expr_part);
        }
        else{
            stack_calculate_simul.pop_back();
        }
        std::shared_ptr<SyntaxerNode> cur = ExprLogicAnd();
        if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0){
            throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},expr_part);
        }

        expr_part->AddChildren(logic_and_part);
        expr_part->AddChildren(cur);
        std::swap(logic_and_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,"||"});
    }

    return logic_and_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprLogicAnd(){

    std::shared_ptr<SyntaxerNode> equality_part = ExprEquality();

    while(lexer.currentToken().lexeme == "&&"){
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0) {
            throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},expr_part);
        }
        else{
            stack_calculate_simul.pop_back();
        }
        std::shared_ptr<SyntaxerNode> cur = ExprEquality();
        if(stack_calculate_simul.back().first != Types::BOOL || stack_calculate_simul.back().second != 0){
            throw BuildSemanticError1(stack_calculate_simul.back(),{Types::BOOL,0},expr_part);
        }


        expr_part->AddChildren(equality_part);
        expr_part->AddChildren(cur);
        std::swap(equality_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,"&&"});
    }

    return equality_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprEquality(){

    std::shared_ptr<SyntaxerNode> rel_part = ExprRel();

    while(lexer.currentToken().lexeme == "==" ||
        lexer.currentToken().lexeme == "!="){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprRel();
        assert(stack_calculate_simul.size() > 1);
        if(stack_calculate_simul.back() != stack_calculate_simul[stack_calculate_simul.size() - 2]){
            throw BuildSemanticError1(stack_calculate_simul.back(),stack_calculate_simul[stack_calculate_simul.size() - 2],expr_part);
        }else{
            stack_calculate_simul.pop_back();
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::BOOL,0});
        }

        expr_part->AddChildren(rel_part);
        expr_part->AddChildren(cur);
        std::swap(rel_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }

    return rel_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprRel(){

    std::shared_ptr<SyntaxerNode> add_part = ExprAdd();

    while(lexer.currentToken().lexeme == "<=" ||
        lexer.currentToken().lexeme == ">=" ||
        lexer.currentToken().lexeme == "<" ||
        lexer.currentToken().lexeme == ">"){
            std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprAdd();
        assert(stack_calculate_simul.size() > 1);
        if(stack_calculate_simul.back() != stack_calculate_simul[stack_calculate_simul.size() - 2]){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            stack_calculate_simul[stack_calculate_simul.size() - 2],expr_part);
        }else{
            stack_calculate_simul.pop_back();
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::BOOL,0});
        }


        expr_part->AddChildren(add_part);
        expr_part->AddChildren(cur);
        std::swap(add_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return add_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprAdd(){
    std::shared_ptr<SyntaxerNode> mul_part = ExprMul();
    while(lexer.currentToken().lexeme == "+" ||
        lexer.currentToken().lexeme == "-"){
            std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        std::shared_ptr<SyntaxerNode> cur = ExprMul();
        assert(stack_calculate_simul.size() > 1);
        if(stack_calculate_simul.back() != stack_calculate_simul[stack_calculate_simul.size() - 2]){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            stack_calculate_simul[stack_calculate_simul.size() - 2],expr_part);
        }else{
            stack_calculate_simul.pop_back();
        }


        expr_part->AddChildren(mul_part);
        expr_part->AddChildren(cur);
        std::swap(mul_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    
    return mul_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprMul(){

    std::shared_ptr<SyntaxerNode> unary_part = ExprUnary();

    while(lexer.currentToken().lexeme == "*" ||
        lexer.currentToken().lexeme == "%" ||
        lexer.currentToken().lexeme == "/"){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());
        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);
        lexer.next();
        expr_part->AddChildren(unary_part);
        expr_part->AddChildren(ExprUnary());
        assert(stack_calculate_simul.size() > 1);
        if(stack_calculate_simul.back() != stack_calculate_simul[stack_calculate_simul.size() - 2]){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            stack_calculate_simul[stack_calculate_simul.size() - 2],expr_part);
        }else{
            stack_calculate_simul.pop_back();
        }
        std::swap(unary_part,expr_part);
        poliz.AddEl({POLIZ_Element::OPERATION,op});
    }
    return unary_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ExprUnary(){

    if(lexer.currentToken().lexeme == "!" ||
    lexer.currentToken().lexeme == "-" ||
    lexer.currentToken().lexeme == "+"){
        std::string op = lexer.currentToken().lexeme;
        std::shared_ptr<SyntaxerNode> expr_part(new SyntaxerNode());

        expr_part->UpdateLexeme(lexer.currentToken().lexeme);
        expr_part->UpdateType(lexer.currentToken().type);
        expr_part->UpdatePos(lexer.currentToken().pos);

        lexer.next();
        std::shared_ptr<SyntaxerNode> primary_part = ExprPrimary();
        if(stack_calculate_simul.back().first == Types::CHAR || 
        stack_calculate_simul.back().first == Types::DOUBLE || 
        stack_calculate_simul.back().first == Types::VOID || stack_calculate_simul.back().second != 0){

            throw BuildSemanticError1(stack_calculate_simul.back(),{Types::INT,0},expr_part);
        }
        expr_part->AddChildren(primary_part);
        poliz.AddEl({POLIZ_Element::UNARY_OPERATION,op});

        return expr_part;
    }
    return ExprPrimary();
}



std::shared_ptr<SyntaxerNode> Syntaxer::ExprPrimary(){
    std::shared_ptr<SyntaxerNode> primary_part(new SyntaxerNode());
    if(lexer.currentToken().lexeme == "("){
        lexer.next();
        if(lexer.currentToken().lexeme == "int"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            std::shared_ptr<SyntaxerNode> expt_part = Expr();
            if(stack_calculate_simul.back().second != 0 || stack_calculate_simul.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::INT,0});
            poliz.AddEl({POLIZ_Element::TO_INT,""});
            return expt_part;
        }else if(lexer.currentToken().lexeme == "double"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            std::shared_ptr<SyntaxerNode> expt_part = Expr();
            if(stack_calculate_simul.back().second != 0 || stack_calculate_simul.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::DOUBLE,0});
            return expt_part;
        }else if(lexer.currentToken().lexeme == "bool"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            std::shared_ptr<SyntaxerNode> expt_part = Expr(); 
            if(stack_calculate_simul.back().second != 0 || stack_calculate_simul.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::BOOL,0});
            poliz.AddEl({POLIZ_Element::TO_BOOL,""});
            return expt_part;
        }else if(lexer.currentToken().lexeme == "char"){
            lexer.next();
            if(lexer.currentToken().lexeme != ")"){
                throw BuildError({")"},lexer.currentToken());
            }
            lexer.next();
            std::shared_ptr<SyntaxerNode> expt_part = Expr();
            if(stack_calculate_simul.back().second != 0 || stack_calculate_simul.back().first == Types::VOID){
                throw "Incorrect type(can not do cast)";
            }
            stack_calculate_simul.pop_back();
            stack_calculate_simul.push_back({Types::CHAR,0});
            poliz.AddEl({POLIZ_Element::TO_CHAR,""});
            return expt_part;
        }
    
        std::shared_ptr<SyntaxerNode> expt_part = Expr();
        if(lexer.currentToken().lexeme != ")"){
            throw BuildError({")"},lexer.currentToken());
        }
        lexer.next();
        return expt_part;
    }


    if(lexer.currentToken().type != Token::Type::IntegerLiteral
    && lexer.currentToken().type != Token::Type::FloatLiteral
    && lexer.currentToken().type != Token::Type::CharLiteral
    && lexer.currentToken().type != Token::Type::StringLiteral
    && lexer.currentToken().type != Token::Type::Identifier){
        throw BuildError({"(","digit","char"},lexer.currentToken());
    }


    primary_part->UpdateLexeme(lexer.currentToken().lexeme);
    primary_part->UpdateType(lexer.currentToken().type);
    primary_part->UpdatePos(lexer.currentToken().pos);


    if(lexer.currentToken().type == Token::Type::Identifier){
        lexer.next();
        if(lexer.currentToken().lexeme == "("){

            lexer.next();
            std::string real_name = primary_part->GiveLexeme();
            std::string name = primary_part->GiveLexeme();
            std::vector<Types> param_type;
            while(lexer.currentToken().lexeme != ")"){
                primary_part->AddChildren(ExprAssign());
                if (stack_calculate_simul.empty()) {
                    throw "internal error: type stack underflow in argument list";
                }
                if(real_name != "read"){
                    if(stack_calculate_simul.back().first == Types::INT){
                        poliz.AddEl({POLIZ_Element::TO_INT,""});
                    }else if(stack_calculate_simul.back().first == Types::DOUBLE){
                        poliz.AddEl({POLIZ_Element::TO_DOUBLE,""});
                    }
                    else if(stack_calculate_simul.back().first == Types::CHAR){
                        poliz.AddEl({POLIZ_Element::TO_CHAR,""});
                    }
                    else if(stack_calculate_simul.back().first == Types::BOOL){
                        poliz.AddEl({POLIZ_Element::TO_BOOL,""});
                    }
                }
            
                param_type.push_back(stack_calculate_simul.back().first);
                name += " ";
                name += TypeToString(stack_calculate_simul.back().first);
                stack_calculate_simul.pop_back();
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
            if(real_name == "print"){
                poliz.AddEl({POLIZ_Element::CALL_PRINT,""});
                stack_calculate_simul.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});

            }else if(real_name == "read"){
                POLIZ_Element last_el = poliz.GiveEl(poliz.GiveSize() - 1).first;
                if(last_el != POLIZ_Element::ADRESS_BOOL && last_el != POLIZ_Element::ADRESS_CHAR
                     && last_el != POLIZ_Element::ADRESS_DOUBLE && last_el != POLIZ_Element::ADRESS_INT && last_el != POLIZ_Element::GLOBAL_VARIABLE){
                    throw "You can read only on variable";
                }
                poliz.AddEl({POLIZ_Element::CALL_READ,""});
                stack_calculate_simul.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});

            }else{
                poliz.AddEl({POLIZ_Element::FUNCTION_ADRESS,std::to_string(func.Get(name).GivePolizIndex())});
                poliz.AddEl({POLIZ_Element::CALL_FUNCTION,""});

                stack_calculate_simul.push_back({func.Get(name).GiveReturnValue(),func.Get(name).GiveArraySize()});
            }
            return primary_part;
        }
        else if(lexer.currentToken().lexeme == "["){
            std::string name = primary_part->GiveLexeme();
            
            if(!Find(name)){
                throw BuildSemanticError(name);
            }
            
            int cur_size = 0;


            std::shared_ptr<TIDElement> now_element = Give(name);
            std::vector<int> all_sizes;
            int numbers_dimension = 0;
            int size_type = 0;
            if(IsArray<int>(now_element) != nullptr){
                numbers_dimension = IsArray<int>(now_element)->GiveSizes().size();
                all_sizes = IsArray<int>(now_element)->GiveSizes();
                size_type = sizeof(int);
            }
            else if(IsArray<char>(now_element) != nullptr){
                numbers_dimension = IsArray<char>(now_element)->GiveSizes().size();
                all_sizes = IsArray<char>(now_element)->GiveSizes();
                size_type = sizeof(char);
            } 
            else if(IsArray<double>(now_element) != nullptr){
                numbers_dimension = IsArray<double>(now_element)->GiveSizes().size();
                all_sizes = IsArray<double>(now_element)->GiveSizes();
                size_type = sizeof(double);
            }else if(IsArray<bool>(now_element) != nullptr){
                numbers_dimension = IsArray<bool>(now_element)->GiveSizes().size();
                all_sizes = IsArray<bool>(now_element)->GiveSizes();
                size_type = sizeof(bool);
            }else{
                throw name + " is varaiable, but not the array";
            }

            int size_of_array = 1;
            for(int i: all_sizes){
                size_of_array *= i;
            }
    

            int cur_ind = 0;

            while(lexer.currentToken().lexeme == "["){
                lexer.next();
                size_of_array /= all_sizes[cur_size];
                cur_size++;
                primary_part->AddChildren(Expr());
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_of_array)});
                poliz.AddEl({POLIZ_Element::OPERATION,"*"});
                if(cur_size != 1){
                    poliz.AddEl({POLIZ_Element::OPERATION,"+"});
                }
                if(stack_calculate_simul.back().first != Types::INT 
                || stack_calculate_simul.back().second != 0){
                    throw "Array indexes should be integer";
                }else{
                    stack_calculate_simul.pop_back();
                }
                if(lexer.currentToken().lexeme != "]"){
                    throw BuildError({"]"},lexer.currentToken());
                }
                lexer.next();
            }
            if(numbers_dimension != cur_size){
                throw "You can not use array in expressions " + name;
            }

            // Индексы массива в POLIZ считаются в элементах, а смещения переменных (offset)
            // хранятся в байтах. Поэтому переводим линейный индекс -> байтовый сдвиг.
            poliz.AddEl({POLIZ_Element::INT, std::to_string(size_type)});
            poliz.AddEl({POLIZ_Element::OPERATION, "*"});

            poliz.AddEl({POLIZ_Element::INT,std::to_string(Give(name)->GiveOffset())});
            poliz.AddEl({POLIZ_Element::OPERATION,"+"});

            

            if(now_element->GiveType() == Types::INT){
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});
            }
            if(now_element->GiveType() == Types::CHAR){
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});
            }
            if(now_element->GiveType() == Types::DOUBLE){
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
            }
            if(now_element->GiveType() == Types::BOOL){
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});
            }
            if(now_element->IsGlobal()){
                poliz.AddEl({POLIZ_Element::GLOBAL_VARIABLE,""});
            }
           
            
            stack_calculate_simul.push_back({now_element->GiveType(),numbers_dimension - cur_size});
            
            return primary_part;
        }
        

        if(!Find(primary_part->GiveLexeme())){
            throw primary_part->GiveLexeme() + " - no such variable";
        }
        std::shared_ptr<TIDElement> now_element = Give(primary_part->GiveLexeme());
        // poliz.AddEl({POLIZ_Element::ADRESS,""});
        poliz.AddEl({POLIZ_Element::INT,std::to_string(now_element->GiveOffset())});
        

        if(IsVariable<int>(now_element) != nullptr){
            stack_calculate_simul.push_back({Types::INT,0});
            poliz.AddEl({POLIZ_Element::ADRESS_INT,""});

        }else if(IsVariable<bool>(now_element) != nullptr){
            stack_calculate_simul.push_back({Types::BOOL,0});
            poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});

        }else if(IsVariable<char>(now_element) != nullptr){
            stack_calculate_simul.push_back({Types::CHAR,0});
            poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});

        }else if(IsVariable<double>(now_element) != nullptr){
            stack_calculate_simul.push_back({Types::DOUBLE,0});
            poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
        }
        if(now_element->IsGlobal()){
            poliz.AddEl({POLIZ_Element::GLOBAL_VARIABLE,""});
        }
        return primary_part;
    }
    
    if(lexer.currentToken().type == Token::Type::CharLiteral){
        stack_calculate_simul.push_back({Types::CHAR,0});
        poliz.AddEl({POLIZ_Element::CHAR,lexer.currentToken().lexeme});
    }

    if(lexer.currentToken().type == Token::Type::IntegerLiteral){
        stack_calculate_simul.push_back({Types::INT,0});
        poliz.AddEl({POLIZ_Element::INT,lexer.currentToken().lexeme});

    }
    if(lexer.currentToken().type == Token::Type::FloatLiteral){
        stack_calculate_simul.push_back({Types::DOUBLE,0});
        poliz.AddEl({POLIZ_Element::DOUBLE,lexer.currentToken().lexeme});
    }

    lexer.next();
    return primary_part;
}

std::shared_ptr<SyntaxerNode> Syntaxer::Program() {
    stack_tids.back().push_back(TID());
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    root->UpdateLexeme("Program");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    while (lexer.currentToken().type != Token::Type::EndOfFile) {
        std::shared_ptr<SyntaxerNode> stmt = Statement();
        root->UpdateLexeme("Program");
        root->AddChildren(stmt);
    }
    stack_tids.back().pop_back();
    return root;
}

std::shared_ptr<SyntaxerNode> Syntaxer::ProgramNoCreateFunction(bool need_create_tid) {
    if(need_create_tid){
        stack_tids.back().push_back({TID()});
    }
    std::shared_ptr<SyntaxerNode> root(new SyntaxerNode());
    root->UpdateLexeme("ProgramNoCreateFunction");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    while (lexer.currentToken().type != Token::Type::EndOfFile &&
           !(lexer.currentToken().lexeme == "}" &&
             lexer.currentToken().type == Token::Type::CloseBracket)) {
        std::shared_ptr<SyntaxerNode> stmt = StatementNoCreationFunction();
        root->AddChildren(stmt);
    }
    if(need_create_tid){
        stack_tids.back().pop_back();
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
    if (!stack_calculate_simul.empty()) {
        stack_calculate_simul.pop_back();
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
    for(int i = stack_tids.back().size() - 1;i >= 0;i--){
        if(stack_tids.back()[i].Find(name)){
            return true;
        }
    }
    return false;
}

std::shared_ptr<TIDElement> Syntaxer::Give(std::string name){
    for(int i = stack_tids.back().size() - 1;i >= 0;i--){
        if(stack_tids.back()[i].Find(name)){
            return stack_tids.back()[i].GetVar(name);
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
    stack_calculate_simul.pop_back();
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
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::INT,std::to_string(size_counter.back())});
                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});
                size_counter.back() += sizeof(double);
        }


        std::shared_ptr<SyntaxerNode> expr = Expr();

        poliz.AddEl({POLIZ_Element::OPERATION,"="});
        
        if(StringToType(typeNode->GiveLexeme()) != stack_calculate_simul.back().first 
        || stack_calculate_simul.back().second != 0){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
        }
        
        
        stack_calculate_simul.pop_back();

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
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
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
        int current_size_array = 1;
        std::string name = nameNode->GiveLexeme();
        int numbers_dimension = 0;
        std::vector<int> sizes;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            std::shared_ptr<SyntaxerNode> dimExpr = Expr();
            int size_of_array = CompileTimeCalculation(dimExpr);
            sizes.push_back(size_of_array);
            std::shared_ptr<SyntaxerNode> size_node(new SyntaxerNode());
            size_node->UpdateLexeme(std::to_string(size_of_array));
            size_node->UpdateType(Token::Type::IntegerLiteral);
            size_node->UpdatePos(lexer.currentToken().pos);

            if(size_of_array <= 0){
                throw "Array size cannot be negative";
            }
            current_size_array *= size_of_array;

            if(stack_calculate_simul.back().first != Types::INT || stack_calculate_simul.back().second != 0){
                throw nameNode->GiveLexeme() + "integer";
            }
            stack_calculate_simul.pop_back();
            numbers_dimension++;
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
        }

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>
                    (name,Types::BOOL,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(bool) * current_size_array;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>
                    (name,Types::DOUBLE,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(double) * current_size_array;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>
                    (name,Types::INT,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int) * current_size_array;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>
                    (name,Types::CHAR,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(char) * current_size_array;
        }

        stack_calculate_simul.push_back({StringToType(typeNode->GiveLexeme()),numbers_dimension});

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
            int was_in_function = InFunction;
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
            int poliz_go_ind = poliz.GiveSize();
            poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
            poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
            int poliz_allocate_ind = poliz.GiveSize();            
            poliz.AddEl({POLIZ_Element::ALLOCATE,""});
            func.CreateFunc(TFuncElement(nameNode->GiveLexeme(),
            cur_func,Types::VOID,root,0,poliz.GiveSize() - 1));
            
            stack_tids.push_back(stack_tids[0]);
            stack_tids.back().push_back(TID());
            for(int i =0 ;i < cur_func.size();i++){
                stack_tids.back().back().CreateVar(cur_func[i]);
            }
            std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(true);

            poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])},poliz_allocate_ind);
                for(auto i:return_helper){
                    poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
                }
            
            poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
                size_counter[size_counter.size() - 2])});
            poliz.AddEl({POLIZ_Element::END_FUNCTION,""});

            poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},poliz_go_ind);
            size_counter.pop_back();
            // std::cout<<size_counter.back()<<"dhfkjl"<<std::endl;
            InFunction = was_in_function;

            stack_tids.pop_back();
    
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
    int numbers_dimension = 0;
    while(lexer.currentToken().lexeme == "["){
        lexer.next();
        numbers_dimension++;
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


        int was_in_function = InFunction;
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
            cur_func,StringToType(typeNode->GiveLexeme()),root,numbers_dimension,poliz.GiveSize()));

        if (lexer.currentToken().lexeme != ")") {
            throw BuildError({")"}, lexer.currentToken());
        }
        lexer.next(); // ')'

        if (lexer.currentToken().lexeme != "{") {
            throw BuildError({"{"}, lexer.currentToken());
        }
        lexer.next(); // '{'

        
        stack_tids.push_back(stack_tids[0]);
        stack_tids.back().push_back(TID());
        for(int i =0 ;i < cur_func.size();i++){
            stack_tids.back().back().CreateVar(cur_func[i]);
        }
        
        int poliz_allocate_ind = poliz.GiveSize();
        poliz.AddEl({POLIZ_Element::ALLOCATE,""});

        std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(true);

        poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])},poliz_allocate_ind);
            for(auto i:return_helper){
                poliz.UpdateEl({POLIZ_Element::POLIZ_GO,std::to_string(poliz.GiveSize())},i);
            }
        poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - 
            size_counter[size_counter.size() - 2])});
        size_counter.pop_back();
        poliz.AddEl({POLIZ_Element::END_FUNCTION,""});

        poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},tmp_ind);

       

        InFunction = was_in_function;

        stack_tids.pop_back();

        if (lexer.currentToken().lexeme != "}") {
            throw BuildError({"}"}, lexer.currentToken());
        }
        lexer.next(); // '}'

        root->AddChildren(params);
        root->AddChildren(body);
        return root;
    }
    if(numbers_dimension != 0){
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
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::ADRESS_BOOL,""});

                size_counter.back() += sizeof(bool);
        }else if(StringToType(typeNode->GiveLexeme())== Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                poliz.AddEl({POLIZ_Element::ADRESS_CHAR,""});

                size_counter.back() += sizeof(char);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));


                poliz.AddEl({POLIZ_Element::ADRESS_INT,""});

                size_counter.back() += sizeof(int);
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));

                poliz.AddEl({POLIZ_Element::ADRESS_DOUBLE,""});

                size_counter.back() += sizeof(double);
        }


        std::shared_ptr<SyntaxerNode> expr = Expr();

        poliz.AddEl({POLIZ_Element::OPERATION,"="});
        
        if(StringToType(typeNode->GiveLexeme()) != stack_calculate_simul.back().first 
        || stack_calculate_simul.back().second != 0){
            throw BuildSemanticError1(stack_calculate_simul.back(),
            {StringToType(typeNode->GiveLexeme()),0},root);      
        }
        
        
        stack_calculate_simul.pop_back();

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
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(bool);
        }else if(declared == Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(char);
        }else if(declared == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(nameNode->GiveLexeme(),
                StringToType(typeNode->GiveLexeme()),
                size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
        }else if(declared == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
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
        int numbers_dimension = 0;
        int all_size_array = 1;
        std::vector<int> sizes;
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            std::shared_ptr<SyntaxerNode> dimExpr = Expr();

            int size_of_array = CompileTimeCalculation(dimExpr);
           std::shared_ptr<SyntaxerNode> size_array_node(new SyntaxerNode());
            size_array_node->UpdateLexeme(std::to_string(size_of_array));
            size_array_node->UpdateType(Token::Type::IntegerLiteral);
            size_array_node->UpdatePos(lexer.currentToken().pos);


            sizes.push_back(size_of_array);
            if(size_of_array <= 0){
                throw "size of array should be positive integer";
            }
            all_size_array *= size_of_array;
            if(stack_calculate_simul.back().first != Types::INT || stack_calculate_simul.back().second != 0){
                throw "size of array should be integer";
            }
            stack_calculate_simul.pop_back();
            numbers_dimension++;
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
        }

        if(StringToType(typeNode->GiveLexeme()) == Types::BOOL){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>
                    (name,Types::BOOL,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(bool) * all_size_array;
        }else if(StringToType(typeNode->GiveLexeme()) == Types::DOUBLE){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>
                    (name,Types::DOUBLE,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(double) * all_size_array;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::INT){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>
                    (name,Types::INT,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int) * all_size_array;

        }else if(StringToType(typeNode->GiveLexeme()) == Types::CHAR){
            stack_tids.back().back().CreateVar(std::dynamic_pointer_cast<TIDElement>(
                std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>
                    (name,Types::CHAR,sizes,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(char) * all_size_array;
        }

        stack_calculate_simul.push_back({StringToType(typeNode->GiveLexeme()),numbers_dimension});


        if(lexer.currentToken().lexeme == "=" && numbers_dimension == 1){
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

    std::shared_ptr<SyntaxerNode> arg_type = Type();
    int number_dimension = 0;
    while(lexer.currentToken().lexeme == "["){
        lexer.next();
        if(lexer.currentToken().lexeme == "]"){
            number_dimension++;
        }else{
            throw BuildError({"["},lexer.currentToken());
        }
        lexer.next();
    }
    std::shared_ptr<SyntaxerNode> arg_variable = Variable();
    std::string name_arg_variable = arg_variable->GiveLexeme();
    cur_function_name_ += " ";
    cur_function_name_ += arg_type->GiveLexeme();
    arg_variable->UpdateLexeme(name_arg_variable);
    if(number_dimension != 0){
        Types type_arg = StringToType(arg_type->GiveLexeme());
        if(arg_type->GiveLexeme() == "int"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
            size_counter.back() += sizeof(int);
        }else if(arg_type->GiveLexeme() == "double"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(arg_type->GiveLexeme() == "bool"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(arg_type->GiveLexeme() == "char"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }
    }else{
        // Scalar parameter: store tightly packed values (byte offsets).
        Types type_arg = StringToType(arg_type->GiveLexeme());
        if(arg_type->GiveLexeme() == "int"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(int);
        }else if(arg_type->GiveLexeme() == "double"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(double);
        }else if(arg_type->GiveLexeme() == "bool"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(bool);
        }else if(arg_type->GiveLexeme() == "char"){
            cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                (std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
            
            size_counter.back() += sizeof(char);
        }
    }
    list->AddChildren(makeParam(arg_type, arg_variable));

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        std::shared_ptr<SyntaxerNode> type_next_arg = Type();
        number_dimension = 0;
        while(lexer.currentToken().lexeme == "["){
            lexer.next();
            if(lexer.currentToken().lexeme == "]"){
                number_dimension++;
            }else{
                throw BuildError({"["},lexer.currentToken());
            }
            lexer.next();
        }
        std::shared_ptr<SyntaxerNode> variable_next_arg = Variable();
        name_arg_variable = variable_next_arg->GiveLexeme();
        cur_function_name_ += " ";
        cur_function_name_ += type_next_arg->GiveLexeme();
        variable_next_arg->UpdateLexeme(name_arg_variable);
        if(number_dimension != 0){
            // Array parameter: we store a pointer (address) in the call frame.
            // Address type in this project is represented as int.
            Types type_arg = StringToType(type_next_arg->GiveLexeme());
            if(type_next_arg->GiveLexeme() == "int"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementArray<int>>(new TIDElementArray<int>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
                size_counter.back() += sizeof(int);
            }else if(type_next_arg->GiveLexeme() == "double"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementArray<double>>(new TIDElementArray<double>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(type_next_arg->GiveLexeme() == "bool"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementArray<bool>>(new TIDElementArray<bool>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(type_next_arg->GiveLexeme() == "char"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementArray<char>>(new TIDElementArray<char>(name_arg_variable,type_arg,number_dimension,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }
        }else{
            // Scalar parameter: store tightly packed values (byte offsets).
            Types type_arg = StringToType(type_next_arg->GiveLexeme());
            if(type_next_arg->GiveLexeme() == "int"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementVariable<int>>(new TIDElementVariable<int>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(int);
            }else if(type_next_arg->GiveLexeme() == "double"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementVariable<double>>(new TIDElementVariable<double>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(double);
            }else if(type_next_arg->GiveLexeme() == "bool"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementVariable<bool>>(new TIDElementVariable<bool>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(bool);
            }else if(type_next_arg->GiveLexeme() == "char"){
                cur_func.push_back(std::dynamic_pointer_cast<TIDElement>
                    (std::shared_ptr<TIDElementVariable<char>>(new TIDElementVariable<char>(name_arg_variable,type_arg,size_counter.back(),InFunction == -1))));
                
                size_counter.back() += sizeof(char);
            }
        }
        list->AddChildren(makeParam(type_next_arg, variable_next_arg));
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
    stack_tids.back().push_back({TID()});
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
    int cur_poliz_size = poliz.GiveSize();
    std::shared_ptr<SyntaxerNode> cond = nullptr;
    if (lexer.currentToken().lexeme != ";") {
        cond = Expr();
    }
    if(stack_calculate_simul.back().first != Types::BOOL){
        throw "Condition in For should be bool";
    }

    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // ';'


    int ind_condition = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_FGO,""});

    int poliz_go_ind = poliz.GiveSize();
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,""});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});

    // --- Шаг: [ Expr ] ---

    int ind_expr = poliz.GiveSize();
    std::shared_ptr<SyntaxerNode> step = nullptr;
    if (lexer.currentToken().lexeme != ")") {
        step = Expr();
    }
    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(cur_poliz_size)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},poliz_go_ind);

    if (lexer.currentToken().lexeme != ")") {
        throw BuildError({")"}, lexer.currentToken());
    }
    lexer.next(); // ')'

    if (lexer.currentToken().lexeme != "{") {
        throw BuildError({"{"}, lexer.currentToken());
    }
    lexer.next(); // '{'

    int was_in_cycle = InCycle;
    InCycle = true;
    std::shared_ptr<SyntaxerNode> body = ProgramNoCreateFunction(false);

    stack_tids.back().pop_back();
    
    


    poliz.AddEl({POLIZ_Element::POLIZ_LABEL,std::to_string(ind_expr)});
    poliz.AddEl({POLIZ_Element::POLIZ_GO,""});
    poliz.UpdateEl({POLIZ_Element::POLIZ_LABEL,std::to_string(poliz.GiveSize())},
    ind_condition);

    poliz.UpdateEl({POLIZ_Element::ALLOCATE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])},alloc);
    poliz.AddEl({POLIZ_Element::FREE,std::to_string(size_counter.back() - size_counter[size_counter.size() - 2])});
    size_counter.pop_back();

    InCycle = was_in_cycle;
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
