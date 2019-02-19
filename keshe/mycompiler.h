#ifndef _MYCOMPILER_H_
#define _MYCOMPILER_H_

#include <stdio.h>
#include <string.h>
//储存单词表的数据结构
typedef union { //保存指向附加信息text的指针
    char*   pchar;  //同时表示ident的text和char常量
    int*    pint;
    float*  pfloat;
} point;
typedef struct { //表示每个单词token
    int name;
    point text;
} token;

extern void lexer();
extern FILE* fp;
#endif