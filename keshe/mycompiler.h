#ifndef _MYCOMPILER_H_
#define _MYCOMPILER_H_

#include <stdio.h>
#include <string.h>
typedef enum {AUTO=0, SHORT, INT, LONG, FLOAT, DOUBLE, CHAR, STRUCT, UNION, ENUM, TYPEDEF, CONST, UNSIGNED, SIGNED, EXTERN, REGISTER, STATIC, VOLATILE, VOID, ELSE, SWITCH, CASE, FOR, DO, WHILE, GOTO, CONTINUE, BREAK, DEFAULT, SIZEOF, RETURN, IF, IDENT, INT_CONST, FLOAT_CONST, CHAR_CONST, STRING, EQ, ASSIGN, INC, ADD, DEC, SUB, NE, LE, LT, GE, GT, LOR, LAN, NO, MOD, MUL, LP, RP, LK, RK, LC, RC, SEM, DIV};
//储存单词表的数据结构
typedef struct { //表示每个单词token
    int name;
    char* text;  //所有的指针类型都只占一个字节
} token;
//词法分析
typedef struct {
    token tk;
    astnode* father; //父结点
    astnode* brother;//兄弟结点
    astnode* child;  //子结点
} astnode;

extern void lexer();
extern FILE* fp;
#endif