#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

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
bool typeBase();
bool arrayDecl();
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound();
bool expr();
bool exprAssign();
bool exprOr();
bool exprAnd();
bool exprEq();
bool exprRel();
bool exprAdd();
bool exprMul();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPrimary();

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
Token* consumed; // last atom that's been consumed

void err(const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

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
    printf("consume(%s)", getTokenCode(code));
    if (iterToken->code == code) {
        consumed = iterToken;
        iterToken = iterToken->next;
        printf("=> consumed\n");
        return true;
    }
    printf(" => found %s\n",getTokenCode(iterToken->code));
    return false;
}

bool unit() {
    printf("#unit() -> %s\n", getTokenCode(iterToken->code));
    for (;;) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (consume(END)) {
        return true;
    } else {
        // error
    }
}

bool structDef() {
    printf("#structDef() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            if (consume(LACC)) {
                while (varDef()) {}
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        return true;
                    } else tkerr(iterToken, "Missing ; after }");
                } else tkerr(iterToken, "Missing } after {");
            }
        } else tkerr(iterToken, "Missing variable name after struct");
    }
    iterToken = start;
    return false;
}

bool varDef() {
    printf("#varDef() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (typeBase()) {
        if (consume(ID)) {
            if (arrayDecl()) {}
            if (consume(SEMICOLON))
                return true;
        } else tkerr(iterToken, "Missing variable name");
    }
    iterToken = start;
    return false;
}

bool typeBase() {
    printf("#typeBase() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(INT)) return true;
    if (consume(DOUBLE)) return true;
    if (consume(CHAR)) return true;
    if (consume(STRUCT)) 
        if (consume(ID)) 
            return true;
    iterToken = start;
    return false;
}

bool arrayDecl() {
    printf("#arrayDecl() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(LBRACKET)) {
        if (expr()) {}
        if (consume(RBRACKET)) {
            return true;
        }
    }
    iterToken = start;
    return false;
}

bool fnDef() {
    printf("#fnDef() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (typeBase()) {}
    if (consume(VOID)) {}
    if (consume(ID)) {
        if (consume(LPAR)) {
            if (fnParam()) {
                while (consume(COMMA)) {
                    if (fnParam()) {}
                }
            }
            if (consume(RPAR)) {
                if (stmCompound()) {
                    return true;
                }
            }
        }
    }
    iterToken = start;
    return false;
}

bool fnParam() {
    printf("#fnParam() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (typeBase()) {
        if (consume(ID)) {
            if (arrayDecl()) {}
            return true;
        }
    }
    iterToken = start;
    return false;
}

bool stm() {
    printf("#stm() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (stmCompound()) return true;
    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (stm()) {}
                        }
                        return true;
                    }
                }
            }
        }
    } 
    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        return true;
                    }
                }
            }
        }
    } 
    if (consume(FOR)) {
        if (consume(LPAR)) {
            if (expr()) {}
            if (consume(SEMICOLON)) {
                if (expr()) {}
                    if (consume(SEMICOLON)) {
                        if (expr()) {}
                        if (consume(RPAR)) {
                            if (stm()) {
                                return true;
                            }
                        }
                    }
            }
        }
    } 
    if (consume(BREAK)) {
        if (consume(SEMICOLON)) {
            return true;
        }
    } 
    if (consume(RETURN)) {
        if (expr()) {}
        if (consume(SEMICOLON)) {
            return true;
        }
    }
    if (expr()) {}
    if (consume(SEMICOLON)) {
        return true;
    }
    iterToken = start;
    return false;
}

bool stmCompound() {
    printf("#stmCompound() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(LACC)) {
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            return true;
        }
    }
    iterToken = start;
    return false;
}

bool expr() {
    printf("#expr() -> %s\n", getTokenCode(iterToken->code));
    if (exprAssign()) {
        return true;
    }
    return false;
}

bool exprAssign() {
    printf("#exprAssign() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (exprAssign()) {
                return true;
            }
        }
    } 
    iterToken = start;
    if (exprOr()) return true;
    iterToken = start;
    return false;
}

bool exprOrPrim() {
    Token *start = iterToken;
    if (consume(OR)) {
        if (exprAnd()) {
            if (exprOrPrim()) {
                return true;
            }
        }
    }
    iterToken = start;
    return true;
}

// exprOr

bool exprOr() {
    printf("#exprOr() -> %s\n", getTokenCode(iterToken->code));
    if (exprAnd()) {
        if (exprOrPrim()) {
            return true;
        }
    }
    return false;
}

bool exprAndPrim() {
    Token *start = iterToken;
    if (consume(AND)) {
        if (exprEq()) {
            if (exprAndPrim()) {
                return true;
            }
        }
    }
    iterToken = start;
    return true;
}

bool exprAnd() {
    printf("#exprAnd() -> %s\n", getTokenCode(iterToken->code));
    if (exprEq()) {
        if (exprAndPrim()) {
            return true;
        }
    }
    return false;
}

bool exprEqPrim() {
    Token *start = iterToken;
    int id = -1;
    if (consume(EQUAL)) { id = EQUAL; }
    else if(consume(NOTEQ)) { id = NOTEQ; }

    if (id >= 0) {
        if (exprRel()) {
            if (exprEqPrim()) return true;
        }
    }
    iterToken = start;
    return true;
}

bool exprEq() {
    printf("#exprEq() -> %s\n", getTokenCode(iterToken->code));
    if (exprRel()) {
        if (exprEqPrim()) {
            return true;
        }
    }
    return false;
}

bool exprRelPrim() {
    Token *start = iterToken;
    int id = -1;
    if (consume(LESS)) { id = LESS; }
    else if (consume(LESSEQ)) { id = LESSEQ; }
    else if (consume(GREATER)) { id = GREATER; }
    else if (consume(GREATEREQ)) { id = GREATEREQ; }

    if (id >= 0 ) {
        if (exprAdd()) {
            if (exprRelPrim()) {
                return true;
            }
        }
    }
    iterToken = start;
    return true;
}

bool exprRel() {
    printf("#exprRel() -> %s\n", getTokenCode(iterToken->code));
    if (exprAdd()) {
        if (exprRelPrim()) {
            return true;
        }
    }
    return false;
}

bool exprAddPrim() {
    Token *start = iterToken;
    int id = -1;
    if (consume(ADD)) { id = ADD; }
    else if (consume(SUB)) { id = SUB; }
    if (exprMul()) {
        if (exprAddPrim()) {
            return true;
        }
    }
    iterToken = start;
    return true;
}

bool exprAdd() {
    printf("#exprAdd() -> %s\n", getTokenCode(iterToken->code));
    if (exprMul()) {
        if (exprAddPrim()) {
            return true;
        }
    }
    return false;
}

bool exprMulPrim() {
    Token *start = iterToken;
    int id = -1;
    if (consume(MUL)) { id = MUL; }
    else if (consume(DIV)) { id = DIV; }
    if (id >= 0) {
        if (exprCast()) {
            if (exprMulPrim()) {
                return true;
            }
        }
    }
    iterToken = start;
    return true;
}

bool exprMul() {
    printf("#exprMul() -> %s\n", getTokenCode(iterToken->code));
    if (exprCast()) {
        if (exprMulPrim()) {
            return true;
        }
    }
    return false;
}

bool exprCast() {
    printf("#exprCast() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(LPAR)) {
        if (typeBase()) {
            if (arrayDecl()) {}
            if (consume(RPAR)) {
                if (exprCast()) {
                    return true;
                }
            }
        }
    } 
    iterToken = start;
    if (exprUnary()) return true;
    iterToken = start;
    return false;
}

bool exprUnary() {
    printf("#exprUnary() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    int id = -1;
    if (consume(SUB)) {
        id = SUB;
    }
    iterToken = start;
    if (consume(NOT)) {
        id = NOT;
    }
    iterToken = start;
    
    if (id >= 0) {
        if (exprUnary()) 
            return true;
    }

    if (exprPostfix()) {
        return true;
    }
    iterToken = start;
    return false;
}

bool exprPostfixPrim() {
    Token *start = iterToken;
    if (consume(LBRACKET)) {
        if (expr()) {
            if (consume(RBRACKET)) {
                if (exprPostfixPrim()) {
                    return true;
                }
            }
        }
    } 
    if (consume(DOT)) {
        if (consume(ID)) {
            if (exprPostfixPrim()) {
                return true;
            }
        }
    }
    iterToken = start;
    return true;
}

bool exprPostfix() {
    printf("#exprPostfix() -> %s\n", getTokenCode(iterToken->code));
    if (exprPrimary()) {
        if (exprPostfixPrim()) {
            return true;
        }
    }
    return false;
}

bool exprPrimary() {
    printf("#exprPrimary() -> %s\n", getTokenCode(iterToken->code));
    Token *start = iterToken;
    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                for (;;) {
                    if (consume(COMMA)) {
                        if (expr()) {}
                    } else break;
                }
            }
        }
        return true;
    }
    iterToken = start;
    if (consume(CT_INT)) return true;
    iterToken = start;
    if (consume(CT_REAL)) return true;
    iterToken = start;
    if (consume(CT_CHAR)) return true;
    iterToken = start;
    if (consume(CT_STRING)) return true;
    iterToken = start;
    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            }
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

    while (getNextToken() != END);
    printToken();

    printf("%s\n", getTokenCode(tokens[0].code));

    iterToken = tokens;
    unit();

    return 0;
}