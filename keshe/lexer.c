#include "mycompiler.h"
typedef enum {AUTO=0, SHORT, INT, LONG, FLOAT, DOUBLE, CHAR, STRUCT, UNION, ENUM, TYPEDEF, CONST, UNSIGNED, SIGNED, EXTERN, REGISTER, STATIC, VOLATILE, VOID, ELSE, SWITCH, CASE, FOR, DO, WHILE, GOTO, CONTINUE, BREAK, DEFAULT, SIZEOF, RETURN, IF, IDENT, INT_CONST, FLOAT_CONST, CHAR_CONST, STRING, EQ, ASSIGN, INC, ADD, DEC, SUB, NE, LE, LT, GE, GT, LOR, LAN, MOD, MUL, LP, RP, LK, RK, LC, RC, SEM, DIV, RCOMMENT, BCOMMENT, ERROR_TOKEN} token_type;
char* name[] = {"auto", "short", "int", "long", "float", "double", "char", "struct", "union", "enum", "typedef", "const", "unsigned", "signed", "extern", "register", "static", "volatile", "void", "else", "switch", "case", "for", "do", "while", "goto", "continue", "break", "default", "sizeof", "return", "if", "ident", "int const", "float_const", "char_const", "string", "==", "=", "++", "+", "--", "-", "!=", "<=", "<", ">=", ">", "||", "&&", "%", "*", "(", ")", "[", "]", "{", "}", ";", "/", "//", "/*", "error"};

char* texts[128];

void lexer (FILE*);
token_type gettoken (FILE*, int*, int*);

void lexer (FILE* fp) {
    char token_text[128];
    int trow = 1;
    int tcol = 0;
    int length = 0;
    int state = 0;
    char c;
    while (feof(fp)) {
        switch (state) {
            case 0 : //正文模式
                token_type token;
                token_type token = gettoken(fp, &trow, &tcol);
                if (token == ERROR_TOKEN)   printf("row%dcol%d : LEXER ERROR!\n", trow, tcol);
                else if (token == RCOMMENT) state = 1;
                else if (token == BCOMMENT) state = 2;
                else printf("row%dcol%d : %s\n", trow, tcol, name[token]);
                break;
            case 1 : //行注释内
                while (c = fgetc(fp) && c != '\n');
                trow += 1;
                state = 0;
                break; 
            case 2 : //列注释内
                while (c = fgetc(fp) && c != '*') {
                    if (c == '\n') trow += 1;
                }
                if (c < 0) printf("LEXER ERROR! EOF before block comments.\n");
                if (c = fgetc(fp) && c == '/') state = 0;
                break;
        }
    }
}

token_type gettoken (FILE* fp, int* row, int* col) {
    char token_text[128];
    char c;
    //过滤空白符号
    while (c = fgetc(fp) && (c == ' '|| c == '\t'|| c == '\n')){
        if (c == '\n') *row += 1;
        else *col += 1;
    }
    
    //处理标识符&关键字
    if ((c > 'a' && c < 'z') || (c > 'A' && c < 'Z')) {
        int i = 0;
        do{token_text[i++] = c; *col += 1;}
        while((c = fgetc(fp)) && ((c > 'a' && c < 'z') || (c > 'A' && c < 'Z') || (c > '0' && c < '9')));
        token_text[i] = 0;  //拼接标识符或者关键字
        ungetc(c, fp);
        if (strcmp(token_text, "auto")) return AUTO;
        else if (strcmp(token_text, "extern")) return EXTERN;
        else if (strcmp(token_text, "register")) return REGISTER;
        else if (strcmp(token_text, "static")) return STATIC;
        else if (strcmp(token_text, "void")) return VOID;
        else if (strcmp(token_text, "short")) return SHORT;
        else if (strcmp(token_text, "int")) return INT;
        else if (strcmp(token_text, "float")) return FLOAT;
        else if (strcmp(token_text, "long")) return LONG;
        else if (strcmp(token_text, "double")) return DOUBLE;
        else if (strcmp(token_text, "char")) return CHAR;
        else if (strcmp(token_text, "const")) return CONST;
        else if (strcmp(token_text, "unsigned")) return UNSIGNED;
        else if (strcmp(token_text, "signed")) return SIGNED;
        else if (strcmp(token_text, "if")) return IF;
        else if (strcmp(token_text, "else")) return ELSE;
        else if (strcmp(token_text, "for")) return FOR;
        else if (strcmp(token_text, "do")) return DO;
        else if (strcmp(token_text, "while")) return WHILE;
        else if (strcmp(token_text, "switch")) return SWITCH;
        else if (strcmp(token_text, "case")) return CASE;
        else if (strcmp(token_text, "default")) return DEFAULT;
        else if (strcmp(token_text, "return")) return RETURN;
        else if (strcmp(token_text, "break")) return BREAK;
        else if (strcmp(token_text, "continue")) return CONTINUE;
        else if (strcmp(token_text, "enum")) return ENUM;
        else if (strcmp(token_text, "typedef")) return TYPEDEF;
        else if (strcmp(token_text, "goto")) return INT;
        else if (strcmp(token_text, "sizeof")) return INT;
        else if (strcmp(token_text, "struct")) return INT;
        else if (strcmp(token_text, "union")) return INT;
        else if (strcmp(token_text, "volatile")) return VOLATILE;
        else strcpy(texts[0], token_text);
        return IDENT;
    }
    
    /*
    //处理数字常量
    if (c > '0' && c < '9') {
        do{token_text[i++] = c;}
        while((c=fgetc(fp)) && (c > '0' && c < '9'));
        ungetc(c, fp);
        return 
    }
    */

    //处理双目算术运算符、关系运算符、逻辑符号、和赋值符号
    switch (c) {
        case '=' : c = fgetc(fp); if (c == '=') *col += 2; return EQ; ungetc(c, fp); *col += 1; return ASSIGN;
        case '+' : c = fgetc(fp); if (c == '+') *col += 2; return INC; ungetc(c, fp); *col += 1; return ADD;
        case '-' : c = fgetc(fp); if (c == '-') *col += 2; return DEC; ungetc(c, fp); *col += 1; return SUB;
        case '!' : c = fgetc(fp); if (c == '=') *col += 2; return NE; ungetc(c, fp); *col += 1; return ERROR_TOKEN;
        case '<' : c = fgetc(fp); if (c == '=') *col += 2; return LE; ungetc(c, fp); *col += 1; return LT;
        case '>' : c = fgetc(fp); if (c == '=') *col += 2; return GE; ungetc(c, fp); *col += 1; return GT;
        case '|' : c = fgetc(fp); if (c == '|') *col += 2; return LOR; ungetc(c, fp); *col += 1;return ERROR_TOKEN;
        case '&' : c = fgetc(fp); if (c == '&') *col += 2; return LAN; ungetc(c, fp); *col += 1; return ERROR_TOKEN;
        case '%' : *col += 1; return MOD;
        case '*' : *col += 1; return MUL;
        case '(' : *col += 1; return LP;
        case ')' : *col += 1; return RP;
        case '[' : *col += 1; return LK;
        case ']' : *col += 1; return RK;
        case '{' : *col += 1; return LC;
        case '}' : *col += 1; return RC;
        case ';' : *col += 1; return SEM;
        case '/' : c = fgetc(fp); if (c == '/') return RCOMMENT; else if (c == '*') return BCOMMENT; *col += 1; return DIV;
        default :
            if (feof(fp)) return EOF;
            else return ERROR_TOKEN;    //报错
    }
}