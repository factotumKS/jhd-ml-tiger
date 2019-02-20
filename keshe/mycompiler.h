#ifndef _MYCOMPILER_H_
#define _MYCOMPILER_H_

#include <stdio.h>
#include <string.h>
//储存单词表的数据结构
typedef struct { //表示每个单词token
    int name;
    char* text;  //所有的指针类型都只占一个字节
} token;

extern void lexer();
extern FILE* fp;
#endif