#include "mycompiler.h"

extern void lexer(FILE*);

//枚举标记类型
void main () {
    FILE* fp = fopen("/home/factotum/test.c", "r");
    lexer(fp);
    fclose(fp);
}