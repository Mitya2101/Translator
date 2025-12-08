#include "Syntaxer.h"






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
    lexer.next();
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

    return_node->AddChildren(Expr());

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
    break_node->UpdatePos(lexer.currentToken().pos);

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
    continue_node->UpdatePos(lexer.currentToken().pos);

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
    now->UpdatePos(lexer.currentToken().pos);
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
    if(lexer.currentToken().type != Token::Type::IntegerLiteral ||
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

    while(lexer.currentToken().lexeme == ","){
        SyntaxerNode* tmp1 = new SyntaxerNode();
        tmp1->UpdateLexeme(lexer.currentToken().lexeme);
        tmp1->UpdateType(lexer.currentToken().type);
        tmp1->UpdatePos(lexer.currentToken().pos);
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
        tmp1->UpdatePos(lexer.currentToken().pos);

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
        tmp1->UpdatePos(lexer.currentToken().pos);

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
        tmp1->UpdatePos(lexer.currentToken().pos);


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
        tmp1->UpdatePos(lexer.currentToken().pos);


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
        tmp1->UpdatePos(lexer.currentToken().pos);


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
        tmp1->UpdatePos(lexer.currentToken().pos);


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
        tmp1->UpdatePos(lexer.currentToken().pos);


        lexer.next();
        SyntaxerNode* cur = ExprPostfix();

        tmp1->AddChildren(tmp);
        tmp1->AddChildren(cur);
        tmp = tmp1;
    }
    return tmp;
}

SyntaxerNode* Syntaxer::ExprPostfix(){
    return ExprUnary();
}


SyntaxerNode* Syntaxer::ExprUnary(){
    SyntaxerNode* tmp = new SyntaxerNode();
    if(lexer.currentToken().lexeme == "!" ||
    lexer.currentToken().lexeme == "-" ||
    lexer.currentToken().lexeme == "+"){
        tmp->UpdateLexeme(lexer.currentToken().lexeme);
        tmp->UpdateType(lexer.currentToken().type);
        tmp->UpdatePos(lexer.currentToken().pos);

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
    tmp->UpdatePos(lexer.currentToken().pos);

    if(lexer.currentToken().type == Token::Type::StringLiteral){
        lexer.next();
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
        while(lexer.currentToken().lexeme == "["){
            lexer.next();
            tmp->AddChildren(Expr());
            if(lexer.currentToken().lexeme != "]"){
                throw BuildError({"]"},lexer.currentToken());
            }
            lexer.next();
        }
        return tmp;
    }
    return tmp;
}



SyntaxerNode* Syntaxer::Program() {
    SyntaxerNode* root = new SyntaxerNode();
    root->UpdateLexeme("Program");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(lexer.currentToken().pos);

    while (lexer.currentToken().type != Token::Type::EndOfFile) {
        SyntaxerNode* stmt = Statement();
        root->AddChildren(stmt);
    }

    return root;
}

SyntaxerNode* Syntaxer::ProgramNoCreateFunction() {
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
    if (lx == "int" || lx == "double" || lx == "char" || lx == "bool") {
        return CreateFunctionOrVariableOrArray();
    }

    // иначе — просто выражение с точкой с запятой: <Expr> ";"
    SyntaxerNode* expr = Expr();
    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // съели ';'
    return expr;
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
        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return decl;
    }

    // Просто выражение с ';'
    SyntaxerNode* expr = Expr();
    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next();
    return expr;
}
SyntaxerNode* Syntaxer::CreateVariableOrArray() {
    // <Type> <Variable> ("=" <Expr> | ("[" Expr "]", { "[" Expr "]" }))
    SyntaxerNode* root = new SyntaxerNode();
    SyntaxerNode* typeNode = Type();      // съедает тип
    SyntaxerNode* varNode  = Variable();  // съедает имя

    root->UpdateLexeme("VarDecl");
    root->UpdateType(Token::Type::Identifier);
    root->UpdatePos(typeNode->GiveToken().pos);

    root->AddChildren(typeNode);
    root->AddChildren(varNode);

    if (lexer.currentToken().lexeme == "=") {
        lexer.next();
        SyntaxerNode* expr = Expr();
        root->AddChildren(expr);
        return root;
    }

    if (lexer.currentToken().lexeme == "[") {
        // одно или несколько измерений массива
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
        }
        return root;
    }

    throw BuildError({"=","["}, lexer.currentToken());
}
SyntaxerNode* Syntaxer::CreateFunctionOrVariableOrArray() {
    // <Type> <Variable> ( "(" ... | "=" Expr ";" | "[" Expr "]" ... ";" )
    SyntaxerNode* root = new SyntaxerNode();
    SyntaxerNode* typeNode = Type();
    SyntaxerNode* nameNode = Variable();

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

        if (lexer.currentToken().lexeme != ")") {
            throw BuildError({")"}, lexer.currentToken());
        }
        lexer.next(); // ')'

        if (lexer.currentToken().lexeme != "{") {
            throw BuildError({"{"}, lexer.currentToken());
        }
        lexer.next(); // '{'

        SyntaxerNode* body = ProgramNoCreateFunction();

        if (lexer.currentToken().lexeme != "}") {
            throw BuildError({"}"}, lexer.currentToken());
        }
        lexer.next(); // '}'

        root->AddChildren(params);
        root->AddChildren(body);
        return root;
    }

    // --- ОДИНОЧНАЯ ПЕРЕМЕННАЯ: = Expr ; ---
    if (lx == "=") {
        lexer.next();
        SyntaxerNode* expr = Expr();
        root->AddChildren(expr);

        if (lexer.currentToken().lexeme != ";") {
            throw BuildError({";"}, lexer.currentToken());
        }
        lexer.next(); // ';'
        return root;
    }

    // --- МАССИВ: "[" Expr "]" { "[" Expr "]" } ";" ---
    if (lx == "[") {
        while (lexer.currentToken().lexeme == "[") {
            lexer.next();
            SyntaxerNode* dimExpr = Expr();
            if (lexer.currentToken().lexeme != "]") {
                throw BuildError({"]"}, lexer.currentToken());
            }
            lexer.next();
            root->AddChildren(dimExpr);
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
    SyntaxerNode* v = Variable();
    list->AddChildren(makeParam(t, v));

    while (lexer.currentToken().lexeme == ",") {
        lexer.next(); // ','
        SyntaxerNode* t2 = Type();
        SyntaxerNode* v2 = Variable();
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
        }
    }

    if (lexer.currentToken().lexeme != ";") {
        throw BuildError({";"}, lexer.currentToken());
    }
    lexer.next(); // ';'

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

    SyntaxerNode* body = ProgramNoCreateFunction();

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
