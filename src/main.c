#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "ad.h"
#include "at.h"

#define NMAX 30000
#define SAFEALLOC(var, Type) if((var = (Type*)malloc(sizeof(Type))) == NULL) err("notenough memory");

typedef struct _Token {
    int code; // codul (numele)
    union{
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct _Token *next;
} Token;

bool unit();
bool structDef();
bool varDef();
bool typeBase(Type *t);
bool arrayDecl(Type *t);
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain);
bool expr(Ret *r);
bool exprAssign(Ret *r);
bool exprOr(Ret *r);
bool exprAnd(Ret *r);
bool exprEq(Ret *r);
bool exprRel(Ret *r);
bool exprAdd(Ret *r);
bool exprMul(Ret *r);
bool exprCast(Ret *r);
bool exprUnary(Ret *r);
bool exprPostfix(Ret *r);
bool exprPrimary(Ret *r);

enum {
    ID,
    BREAK,
    CHAR,
    DOUBLE,
    ELSE,
    FOR,
    IF,
    INT,
    RETURN, 
    STRUCT,
    VOID,
    WHILE,
    CT_INT,
    CT_REAL,
    CT_CHAR,
    CT_STRING,
    COMMA,
    SEMICOLON,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    LACC,
    RACC,
    END,
    ADD,
    SUB,
    MUL,
    DIV,
    DOT,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQUAL,
    NOTEQ,
    LESS, 
    LESSEQ,
    GREATER,
    GREATEREQ
};

Token* tokens;
Token* token;
Token* lastToken;

int line = 1;
char inbuff[NMAX + 1];
char* pch; // points to the current character

Token* iterToken;
Token* consumedToken; // last atom that's been consumed

Symbol* owner;


void tkerr(const Token *tk,const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

/* _________ LEX _________ */

Token *addToken(int code) {
    Token *tk;
    SAFEALLOC(tk, Token)
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if(lastToken){
        lastToken->next = tk;
    }else{
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

char* extract(const char* start, const char* end) {
    unsigned int length = end - start + 1;
    char* result = (char*) malloc(length * sizeof(char));
    snprintf(result, length, "%s", start);
    return result;
}

char* getTokenCode(int code) {
    switch(code) {
        case ID: return "ID";
        case BREAK: return "BREAK";
        case CHAR: return "CHAR";
        case DOUBLE: return "DOUBLE";
        case ELSE: return "ELSE";
        case FOR: return "FOR";
        case IF: return "IF";
        case INT: return "INT";
        case RETURN: return "RETURN";
        case STRUCT: return "STRUCT";
        case VOID: return "VOID";
        case WHILE: return "WHILE";
        case CT_INT: return "CT_INT";
        case CT_REAL: return "CT_REAL";
        case CT_CHAR: return "CT_CHAR";
        case CT_STRING: return "CT_STRING";
        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";
        case LPAR: return "LPAR";
        case RPAR: return "RPAR";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case LACC: return "LACC";
        case RACC: return "RACC";
        case END: return "END";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case DOT: return "DOT";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case ASSIGN: return "ASSIGN";
        case EQUAL: return "EQUAL";
        case NOTEQ: return "NOTEQ";
        case LESS: return "LESS";
        case LESSEQ: return "LESSEQ";
        case GREATER: return "GREATER";
        case GREATEREQ: return "GREATEREQ";
        default: return "Code not found.\n"; break;
    }
}

int getNextToken() {
    int currentState = 0;
    const char* start;
    for (;;) {
        switch (currentState) {
            case 0:
            if (isalpha(*pch) || *pch == '_') {
                currentState = 1, start = pch++;
            } else if (*pch == ' ' || *pch == '\t' || *pch == '\n' || *pch == '\r') {
                if (*pch == '\n') line++;
                pch++;
            } else if (isdigit(*pch)) {
                currentState = 3, start = pch++;
            } else if (*pch == '\'') {
                currentState = 11, start = pch++;
            } else if (*pch == '"') {
                currentState = 14, start = pch++;
            } else if (*pch == ',') {
                currentState = 16, start = pch++;
            } else if (*pch == ';') {
                currentState = 17, pch++;
            } else if (*pch == '(') {
                currentState = 18, pch++;
            } else if (*pch == ')') {
                currentState = 19, pch++;
            } else if (*pch == '[') {
                currentState = 20, pch++;
            } else if (*pch == ']') {
                currentState = 21, pch++;
            } else if (*pch == '{') {
                currentState = 22, pch++;
            } else if (*pch == '}') {
                currentState = 23, pch++;
            } else if (*pch == '\0') {
                currentState = 24, pch++;
            } else if (*pch == '+') {
                currentState = 25, pch++;
            } else if (*pch == '-') {
                currentState = 26, pch++;
            } else if (*pch == '*') {
                currentState = 27, pch++;
            } else if (*pch == '/') {
                currentState = 28, pch++;
            } else if (*pch == '.') {
                currentState = 29, pch++;
            } else if (*pch == '&') {
                currentState = 30, pch++;
            } else if (*pch == '|') {
                currentState = 32, pch++;
            } else if (*pch == '!') {
                currentState = 34, pch++;
            } else if (*pch == '=') {
                currentState = 37, pch++;
            } else if (*pch == '<') {
                currentState = 40, pch++;
            } else if (*pch == '>') {
                currentState = 43, pch++;
            }
            break; 
            case 1:
            if (isalnum(*pch) || *pch == '_') {
                pch++;
            } else {
                currentState = 2;
            }
            break;
            case 2:
            token = addToken(ID);
            token->text = extract(start, pch);
            if (!strcmp(token->text, "break")) token->code = BREAK;
            if (!strcmp(token->text, "char")) token->code = CHAR;
            if (!strcmp(token->text, "double")) token->code = DOUBLE;
            if (!strcmp(token->text, "else")) token->code = ELSE;
            if (!strcmp(token->text, "for")) token->code = FOR;
            if (!strcmp(token->text, "if")) token->code = IF;
            if (!strcmp(token->text, "int")) token->code = INT;
            if (!strcmp(token->text, "return")) token->code = RETURN;
            if (!strcmp(token->text, "struct")) token->code = STRUCT;
            if (!strcmp(token->text, "void")) token->code = VOID;
            if (!strcmp(token->text, "while")) token->code = WHILE;
            return token->code;
            case 3:
            if (isdigit(*pch)) {
                pch++;
            } else if (*pch == '.') {
                currentState = 5, pch++;
            } else if (*pch == 'e' || *pch == 'E') {
                currentState = 7, pch++;
            } else {
                currentState = 4;
            }
            break;
            case 4:
            token = addToken(CT_INT);
            token->i = atoi(extract(start, pch));
            return CT_INT;
            case 5:
            if (isdigit(*pch)) {
                currentState = 6, pch++;
            }
            break;
            case 6:
            if (isdigit(*pch)) {
                pch++;
            } else if (*pch == 'e' || *pch == 'E') {
                currentState = 7, pch++;
            } else {
                currentState = 9;
            }
            break;
            case 7: 
            if (*pch == '+' || *pch == '-') {
                currentState = 8, pch++;
            } else {
                currentState = 8;
            }
            break;
            case 8:
            if (isdigit(*pch)) {
                currentState = 9, pch++;
            }
            break;
            case 9:
            if (isdigit(*pch)) {
                pch++;
            } else {
                currentState = 10;
            }
            break;
            case 10:
            token = addToken(CT_REAL);
            token->r = atof(extract(start, pch));
            return CT_REAL;
            case 11:
            if (*pch != '\'') {
                currentState = 12, pch++;
            }
            break;
            case 12:
            if (*pch == '\'') {
                currentState = 13, pch++;
            }
            break;
            case 13:
            token = addToken(CT_CHAR);
            token->i = start[1];
            return CT_CHAR;
            case 14:
            if (*pch != '"') {
                pch++;
            } else if (*pch == '"') {
                currentState = 15, pch++;
            }
            break;
            case 15:
            token = addToken(CT_STRING);
            token->text = extract(start + 1, pch - 1);
            return CT_STRING;
            break;
            case 16:
            addToken(COMMA);
            return COMMA;
            case 17:
            addToken(SEMICOLON);
            return SEMICOLON;
            case 18: 
            addToken(LPAR);
            return LPAR;
            case 19:
            addToken(RPAR);
            return RPAR;
            case 20:
            addToken(LBRACKET);
            return LBRACKET;
            case 21:
            addToken(RBRACKET);
            return RBRACKET;
            case 22:
            addToken(LACC);
            return LACC;
            case 23:
            addToken(RACC);
            return RACC;
            case 24:
            addToken(END);
            return END;
            case 25:
            addToken(ADD);
            return ADD;
            case 26:
            addToken(SUB);
            return SUB;
            case 27:
            addToken(MUL);
            return MUL;
            case 28:
            if (*pch == '/') {
                currentState = 47, pch++;
            } else {
                currentState = 46;
            }
            break;
            case 29:
            addToken(DOT);
            return DOT;
            case 30:
            if (*pch == '&') {
                currentState = 31, pch++;
            }
            break;
            case 31:
            addToken(AND);
            return AND;
            case 32:
            if (*pch == '|') {
                currentState = 33, pch++;
            }
            break;
            case 33:
            addToken(OR);
            return OR;
            case 34:
            if (*pch == '=') {
                currentState = 36, pch++;
            } else {
                currentState = 35;
            }
            break;
            case 35:
            addToken(NOT);
            return NOT;
            case 36:
            addToken(NOTEQ);
            return NOTEQ;
            case 37:
            if (*pch == '=') {
                currentState = 39, pch++;
            } else {
                currentState = 38;
            }
            break;
            case 38:
            addToken(ASSIGN);
            return ASSIGN;
            case 39:
            addToken(EQUAL);
            return EQUAL;
            case 40:
            if (*pch == '=') {
                currentState = 42, pch++;
            } else {
                currentState = 41;
            }
            break;
            case 41: 
            addToken(LESS);
            return LESS;
            case 42:
            addToken(LESSEQ);
            return LESSEQ;
            case 43:
            if (*pch == '=') {
                currentState = 45, pch++;
            } else {
                currentState = 44;
            }
            break;
            case 44:
            addToken(GREATER);
            return GREATER;
            case 45:
            addToken(GREATEREQ);
            return GREATEREQ;
            case 46:
            addToken(DIV);
            return DIV;
            case 47:
            if (*pch != '\r' && *pch != '\t' && *pch != '\n') {
                pch++;
            } else {
                currentState = 0;
            }
            break;
            default: printf("Eroare: Stare inexistenta: %d (ch = %c).\n", currentState, *pch); break;
        }
    }
}

void printToken() {
    Token* currentToken = tokens;
    while(currentToken != NULL) {
        printf("%d\t%s", currentToken->line, getTokenCode(currentToken->code));
        switch(currentToken->code) {
            case ID: printf(":%s", currentToken->text); break;
            case CT_INT: printf(":%d", currentToken->i); break;
            case CT_REAL: printf(":%g", currentToken->r); break;
            case CT_CHAR: printf(":%c", currentToken->i); break;
            case CT_STRING: printf(":%s", currentToken->text); break;
        }
        currentToken = currentToken->next;
        printf("\n");
    }
}

/* _________ SINTAX _________ */

bool consume(int code) {
    if (iterToken->code == code) {
        consumedToken = iterToken;
        iterToken = iterToken->next;
        return true;
    }
    return false;
}

bool unit() {
    for (;;) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (consume(END)) {
        return true;
    } else {
        tkerr(iterToken, "Missing End of File");
    }
}

bool structDef() {
    Token *start = iterToken;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            Token *name = consumedToken;
            if (consume(LACC)) {
                {
                    Symbol *s = findSymbolInDomain(symTable, name->text);
                    if(s)tkerr(iterToken,"symbol redefinition: %s", name->text);
                    s = addSymbolToDomain(symTable, newSymbol(name->text, SK_STRUCT));
                    s->type.tb = TB_STRUCT;
                    s->type.s = s;
                    s->type.n = -1;
                    pushDomain();
                    owner = s;
                }
                while (varDef()) {}
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        {
                            owner=NULL;
                            dropDomain();
                        }
                        return true;
                    } else tkerr(iterToken, "Missing ; after struct definition");
                } else tkerr(iterToken, "Missing } after the struct variables");
            }
        } else tkerr(iterToken, "Missing struct name");
    }
    iterToken = start;
    return false;
}

bool varDef() {
    Token *start = iterToken;
    Type t;
    if (typeBase(&t)) {
        if (consume(ID)) {
            Token* name = consumedToken;
            if (arrayDecl(&t)) {
                if (t.n == 0)
                    tkerr(iterToken, "a vector variable must have a specified dimension");
            }
            if (consume(SEMICOLON)) {
                {
                    Symbol *var = findSymbolInDomain(symTable,name->text);
                    if (var) tkerr(iterToken, "symbol redefinition: %s", name->text);
                    var = newSymbol(name->text, SK_VAR);
                    var->type = t;
                    var->owner = owner;
                    addSymbolToDomain(symTable, var);
                    if (owner) {
                        switch(owner->kind){
                            case SK_FN:
                                var->varIdx = symbolsLen(owner->fn.locals);
                                addSymbolToList(&owner->fn.locals, dupSymbol(var));
                                break;
                            case SK_STRUCT:
                                var->varIdx = typeSize(&owner->type);
                                addSymbolToList(&owner->structMembers,dupSymbol(var));
                                break;
                            case SK_VAR:
                                break;
                            case SK_PARAM:
                                break;
                        }
                    } else {
                        var->varIdx = allocInGlobalMemory(typeSize(&t));
                    }
                }
                return true;
            } else tkerr(iterToken, "Missing ; after variable name");
        } else tkerr(iterToken, "Missing variable name");
    }
    iterToken = start;
    return false;
}

bool typeBase(Type *t) {
    t->n = -1;
    Token *start = iterToken;
    if (consume(INT)) {
        t->tb = TB_INT;
        return true;
    }
    if (consume(DOUBLE)) {
        t->tb = TB_DOUBLE;
        return true;
    }
    if (consume(CHAR)) {
        t->tb = TB_CHAR;
        return true;
    }
    if (consume(STRUCT)) {
        if (consume(ID)) {
            Token* name = consumedToken;
            t->tb = TB_STRUCT;
            t->s = findSymbol(name->text);
            if (!t->s) tkerr(iterToken, "Undefined struct: %s", name->text);
            return true;
        } else tkerr(iterToken, "Missing struct name");
    }
    iterToken = start;
    return false;
}

bool arrayDecl(Type *t) {
    Token *start = iterToken;
    if (consume(LBRACKET)) {
        if (consume(CT_INT)) {
            Token *size = consumedToken;
            t->n = size->i;
        } else {
            t->n=0; 
        }
        if (consume(RBRACKET)) {
            return true;
        } else tkerr(iterToken, "Missing ] in array declaration");
    }
    iterToken = start;
    return false;
}

bool fnDef() {
    Token *start = iterToken;
    Type t;
    int id = -1;
    if (typeBase(&t)) { id = 100; }
    if (consume(VOID)) { t.tb=TB_VOID, id = VOID; }
    if (id >= 0) {
        if (consume(ID)) {
            Token* name = consumedToken;
            if (consume(LPAR)) {
                {
                    Symbol *fn = findSymbolInDomain(symTable, name->text);
                    if (fn) tkerr(iterToken, "symbol redefinition: %s", name->text);
                    fn = newSymbol(name->text, SK_FN);
                    fn->type = t;
                    addSymbolToDomain(symTable, fn);
                    owner = fn;
                    pushDomain();
                }
                if (fnParam()) {
                    while (consume(COMMA)) {
                        if (fnParam()) {}
                        else tkerr(iterToken, "Missing parameter after ,");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        {
                            dropDomain();
                            owner=NULL;
                        }
                        return true;
                    } else tkerr(iterToken, "Missing function body");
                } else tkerr(iterToken, "Missing ) after function parameters");
            } 
        } else tkerr(iterToken, "Missing function name");
    }
    iterToken = start;
    return false;
}

bool fnParam() {
    Token *start = iterToken;
    Type t;
    if (typeBase(&t)) {
        if (consume(ID)) {
            Token* name = consumedToken;
            if (arrayDecl(&t)) { t.n = 0; }
            {
                Symbol *param=findSymbolInDomain(symTable, name->text);
                if (param) tkerr(iterToken,"symbol redefinition: %s", name->text);
                param = newSymbol(name->text,SK_PARAM);
                param->type = t;
                param->paramIdx = symbolsLen(owner->fn.params);
                addSymbolToDomain(symTable, param);
                addSymbolToList(&owner->fn.params, dupSymbol(param));
            }
            return true;
        } else tkerr(iterToken, "Missing parameter name");
    }
    iterToken = start;
    return false;
}

bool stm() {
    Token *start = iterToken;
    Ret rInit, rCond, rStep, rExpr;
    if (stmCompound(true)) return true;
    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&rCond)) {
                {if (!canBeScalar(&rCond)) tkerr(iterToken, "the if condition must be a scalar value");}
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (stm()) {}
                            else tkerr(iterToken, "Missing else branch body");
                        }
                        return true;
                    } else tkerr(iterToken, "Missing if branch body");
                } else tkerr(iterToken, "Missing ) after if condition");
            } else tkerr(iterToken, "Missing if statement condition");
        } else tkerr(iterToken, "Missing ( after if");
    } 
    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr(&rCond)) {
                {if (!canBeScalar(&rCond)) tkerr(iterToken, "the while condition must be a scalar value");}
                if (consume(RPAR)) {
                    if (stm()) {
                        return true;
                    } else tkerr(iterToken, "Missing while body");
                } else tkerr(iterToken, "Missing ) after while condition");
            } else tkerr(iterToken, "Missing while statement condition");
        } else tkerr(iterToken, "Missing ( after while");
    } 
    if (consume(FOR)) {
        if (consume(LPAR)) {
            if (expr(&rInit)) {}
            if (consume(SEMICOLON)) {
                if (expr(&rCond)) {
                    {if (!canBeScalar(&rCond)) tkerr(iterToken, "the for condition must be a scalar value");}
                }
                    if (consume(SEMICOLON)) {
                        if (expr(&rStep)) {}
                        if (consume(RPAR)) {
                            if (stm()) {
                                return true;
                            } else tkerr(iterToken, "Missing the for body");
                        } else tkerr(iterToken, "Missing ) after the for condition");
                    } else tkerr(iterToken, "Missing the second ; in the for statement");
            } else tkerr(iterToken, "Missing the first ; in the for statement");
        } else tkerr(iterToken, "Missing ( after for");
    } 
    if (consume(BREAK)) {
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr(iterToken, "Missing ; after break");
    } 
    if (consume(RETURN)) {
        if (expr(&rExpr)) {
            {
            if (owner->type.tb == TB_VOID) tkerr(iterToken, "a void function cannot return a value");
            if (!canBeScalar(&rExpr)) tkerr(iterToken, "the return value must be a scalar value");
            if (!convTo(&rExpr.type, &owner->type)) tkerr(iterToken, "cannot convert the return expression type to the function return type");
            }
        } else {
            {if (owner->type.tb != TB_VOID) tkerr(iterToken, "a non-void function must return a value");}
        }
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr(iterToken, "Missing ; after return");
    }
    int ok = 0;
    if (expr(&rExpr)) { ok = 1; }
    if (ok) {
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr(iterToken, "Missing ; after expression");
    }
    iterToken = start;
    return false;
}

bool stmCompound(bool newDomain) {
    Token *start = iterToken;
    if (consume(LACC)) {
        if (newDomain) pushDomain();
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            if (newDomain) dropDomain();
            return true;
        } else tkerr(iterToken, "Missing } after body");
    } 
    iterToken = start;
    return false;
}

bool expr(Ret *r) {
    if (exprAssign(r)) {
        return true;
    }
    return false;
}

bool exprAssign(Ret *r) {
    Token *start = iterToken;
    Ret rDst;
    if (exprUnary(&rDst)) {
        if (consume(ASSIGN)) {
            if (exprAssign(r)) {
                if (!rDst.lval) tkerr(iterToken, "the assign destination must be a left-value");
                if (rDst.ct) tkerr(iterToken, "the assign destination cannot be constant");
                if (!canBeScalar(&rDst)) tkerr(iterToken, "the assign destination must be scalar");
                if (!canBeScalar(r)) tkerr(iterToken, "the assign source must be scalar");
                if (!convTo(&r->type,&rDst.type)) tkerr(iterToken, "the assign source cannot be converted to destination");
                r->lval = false;
                r->ct = true;
                return true;
            } else tkerr(iterToken, "Invalid expression after =");
        }
    } 
    iterToken = start;
    if (exprOr(r)) return true;
    iterToken = start;
    return false;
}

bool exprOrPrim(Ret *r) {
    Token *start = iterToken;
    if (consume(OR)) {
        Ret right;
        if (exprAnd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for ||");
            *r = (Ret) {{TB_INT, NULL, -1}, false, true};
            if (exprOrPrim(r)) {
                return true;
            }
        } else tkerr(iterToken, "Invalid expression after ||");
    }
    iterToken = start;
    return true;
}

// exprOr

bool exprOr(Ret *r) {
    if (exprAnd(r)) {
        if (exprOrPrim(r)) {
            return true;
        }
    } 
    return false;
}

bool exprAndPrim(Ret *r) {
    Token *start = iterToken;
    if (consume(AND)) {
        Ret right;
        if (exprEq(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for &&");
            *r = (Ret) {{TB_INT,NULL, -1}, false, true};
            if (exprAndPrim(r)) {
                return true;
            }
        } else tkerr(iterToken, "Invalid expression after &&");
    }
    iterToken = start;
    return true;
}

bool exprAnd(Ret *r) {
    if (exprEq(r)) {
        if (exprAndPrim(r)) {
            return true;
        }
    }
    return false;
}

bool exprEqPrim(Ret *r) {
    Token *start = iterToken;
    int id = -1;
    if (consume(EQUAL)) { id = EQUAL; }
    else if(consume(NOTEQ)) { id = NOTEQ; }

    if (id >= 0) {
        Ret right;
        if (exprRel(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for == or !=");
            *r = (Ret) {{TB_INT, NULL, -1}, false, true};
            if (exprEqPrim(r)) return true;
        } else if (id == EQUAL) tkerr(iterToken, "Invalid expression after ==");
        else if (id == NOTEQ) tkerr(iterToken, "Invalid expression after !=");
    }
    iterToken = start;
    return true;
}

bool exprEq(Ret *r) {
    if (exprRel(r)) {
        if (exprEqPrim(r)) {
            return true;
        }
    }
    return false;
}

bool exprRelPrim(Ret *r) {
    Token *start = iterToken;
    int id = -1;
    if (consume(LESS)) { id = LESS; }
    else if (consume(LESSEQ)) { id = LESSEQ; }
    else if (consume(GREATER)) { id = GREATER; }
    else if (consume(GREATEREQ)) { id = GREATEREQ; }

    if (id >= 0 ) {
        Ret right;
        if (exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for <, <=, >, >=");
            *r = (Ret) {{TB_INT, NULL, -1}, false, true};
            if (exprRelPrim(r)) {
                return true;
            }
        } else if (id == LESS) tkerr(iterToken, "Invalid expression after <");
        else if (id == LESSEQ) tkerr(iterToken, "Invalid expression after <=");
        else if (id == GREATER) tkerr(iterToken, "Invalid expression after >");
        else if (id == GREATEREQ) tkerr(iterToken, "Invalid expression after >=");
    }
    iterToken = start;
    return true;
}

bool exprRel(Ret *r) {
    if (exprAdd(r)) {
        if (exprRelPrim(r)) {
            return true;
        }
    }
    return false;
}

bool exprAddPrim(Ret *r) {
    Token *start = iterToken;
    int id = -1;
    if (consume(ADD)) { id = ADD; }
    else if (consume(SUB)) { id = SUB; }
    if (id >= 0) {
        Ret right;
        if (exprMul(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for + or -");
            *r = (Ret) {{TB_INT, NULL, -1}, false, true};
            if (exprAddPrim(r)) {
                return true;
            }
        } else if (id == ADD) tkerr(iterToken, "Invalid expression after +");
        else if (id == SUB) tkerr(iterToken, "Invalid expression after -");
    }
    iterToken = start;
    return true;
}

bool exprAdd(Ret *r) {
    if (exprMul(r)) {
        if (exprAddPrim(r)) {
            return true;
        }
    }
    return false;
}

bool exprMulPrim(Ret *r) {
    Token *start = iterToken;
    int id = -1;
    if (consume(MUL)) { id = MUL; }
    else if (consume(DIV)) { id = DIV; }
    if (id >= 0) {
        Ret right;
        if (exprCast(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iterToken, "invalid operand type for * or /");
            *r = (Ret) {{TB_INT, NULL,-1} ,false, true};
            if (exprMulPrim(r)) {
                return true;
            }
        } else if (id == MUL) tkerr(iterToken, "Invalid expression after *");
        else if (id == DIV) tkerr(iterToken, "Invalid expression after /");
    }
    iterToken = start;
    return true;
}

bool exprMul(Ret *r) {
    if (exprCast(r)) {
        if (exprMulPrim(r)) {
            return true;
        }
    }
    return false;
}

bool exprCast(Ret *r) {
    Token *start = iterToken;
    Type t;
    Ret op;
    if (consume(LPAR)) {
        if (typeBase(&t)) {
            if (arrayDecl(&t)) {}
            if (consume(RPAR)) {
                if (exprCast(&op)) {
                    if (t.tb == TB_STRUCT) tkerr(iterToken, "cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT) tkerr(iterToken, "cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0) tkerr(iterToken, "an array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0) tkerr(iterToken, "a scalar can be converted only to another scalar");
                    *r = (Ret) {t, false, true};
                    return true;
                } else tkerr(iterToken, "Invalid expression after ) in cast statement");
            } else tkerr(iterToken, "Missing ) in cast statement");
        }
    } 
    iterToken = start;
    if (exprUnary(r)) return true;
    iterToken = start;
    return false;
}

bool exprUnary(Ret *r) {
    Token *start = iterToken;
    int id = -1;
    if (consume(SUB)) {
        id = SUB;
    }
    if (consume(NOT)) {
        id = NOT;
    }
    
    if (id >= 0) {
        if (exprUnary(r)) {
            if (!canBeScalar(r)) tkerr(iterToken, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        } else if (id == SUB) tkerr(iterToken, "Invalid or invalid expression after -");
        else if (id == NOT) tkerr(iterToken, "Invalid expression after !");
    }

    if (exprPostfix(r)) {
        return true;
    }
    iterToken = start;
    return false;
}

bool exprPostfixPrim(Ret *r) {
    Token *start = iterToken;
    if (consume(LBRACKET)) {
        Ret idx;
        if (expr(&idx)) {
            if (consume(RBRACKET)) {
                if (r->type.n < 0) tkerr(iterToken, "only an array can be indexed");
                Type tInt = {TB_INT, NULL, -1};
                if (!convTo(&idx.type, &tInt)) tkerr(iterToken, "the index is not convertible to int");
                r->type.n = -1;
                r->lval = true;
                r->ct = false;
                if (exprPostfixPrim(r)) {
                    return true;
                } else tkerr(iterToken, "Invalid expression after ]");
            } else tkerr(iterToken, "Missing ] after expression");
        } else tkerr(iterToken, "Invalid expression after [");
    } 
    if (consume(DOT)) {
        if (consume(ID)) {
            Token* name = consumedToken;
            if (r->type.tb != TB_STRUCT) tkerr(iterToken, "a field can only be selected from a struct");
            Symbol *s = findSymbolInList(r->type.s->structMembers, name->text);
            if (!s) tkerr(iterToken, "the structure %s does not have a field %s", r->type.s->name, name->text);
            *r = (Ret) {s->type, true, s->type.n>=0};
            if (exprPostfixPrim(r)) {
                return true;
            } else tkerr(iterToken, "Invalid expression after name");
        } else tkerr(iterToken, "Invalid expression after .");
    } 
    iterToken = start;
    return true;
}

bool exprPostfix(Ret *r) {
    if (exprPrimary(r)) {
        if (exprPostfixPrim(r)) {
            return true;
        }
    } 
    return false;
}

bool exprPrimary(Ret *r) {
    Token *start = iterToken;
    if (consume(ID)) {
        Token* name = consumedToken;
        Symbol *s = findSymbol(name->text);
        if (!s) tkerr(iterToken, "undefined id: %s",name->text);
        if (consume(LPAR)) {
            if (s->kind != SK_FN) tkerr(iterToken, "only a function can be called");
            Ret rArg;
            Symbol *param=s->fn.params;
            if (expr(&rArg)) {
                if (!param) tkerr(iterToken, "too many arguments in function call");
                if (!convTo(&rArg.type, &param->type)) tkerr(iterToken,"in call, cannot convert the argument type to the parameter type");
                param = param->next;
                for (;;) {
                    if (consume(COMMA)) {
                        if (expr(&rArg)) {
                            if (!param) tkerr(iterToken, "too many arguments in function call");
                            if (!convTo(&rArg.type, &param->type)) tkerr(iterToken, "in call, cannot convert the argument type to the parameter type");
                            param = param->next;
                        }
                        else tkerr(iterToken, "Missing expression after ,");
                    } else break;
                }
            }
            if (consume(RPAR)) {
                if (param) tkerr(iterToken, "too few arguments in function call");
                *r = (Ret) {s->type, false, true};
            } else {
                tkerr(iterToken, "Missing ) after expression");
            }
        } else {
            if (s->kind == SK_FN) tkerr(iterToken, "a function can only be called");
            *r=(Ret) {s->type, true, s->type.n >= 0};
        }
        return true;
    }
    iterToken = start;
    if (consume(CT_INT)) {
        *r = (Ret) {{TB_INT, NULL, -1}, false, true}; 
        return true;
    }
    iterToken = start;
    if (consume(CT_REAL)) {
        *r = (Ret) {{TB_DOUBLE, NULL, -1}, false, true}; 
        return true;
    }
    iterToken = start;
    if (consume(CT_CHAR)) {
        *r = (Ret) {{TB_CHAR, NULL, -1}, false, true}; 
        return true;
    }
    iterToken = start;
    if (consume(CT_STRING)) {
        *r = (Ret) {{TB_CHAR, NULL, 0}, false, true}; 
        return true;
    }
    iterToken = start;
    if (consume(LPAR)) {
        if (expr(r)) {
            if (consume(RPAR)) {
                return true;
            } else tkerr(iterToken, "Missing ) after expression");
        } 
    }
    iterToken = start;
    return false;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <testFile>", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char* filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Nu s-a putut deschide fisierul.\n");
        return 1;
    }

    size_t size = fread(inbuff, sizeof(char), NMAX, file);
    inbuff[size] = '\0';

    fclose(file);

    pch = inbuff;

    /* show atoms
     pushDomain();
     unit()
     showDomain(symTable, "Global")
     dropDomain();
*/
    while (getNextToken() != END);
    printToken();
    pushDomain();
    iterToken = tokens;
    if (unit()) printf("Program compiled succesfully.\n");
    showDomain(symTable, "Global");
    dropDomain();

    return 0;
}
