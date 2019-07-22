#include "mycompiler.h"

FILE* fp;   //指向源文件的全局指针

//枚举标记类型
void main () {
    fp = fopen("test.c", "r");
    lexer();
    fclose(fp);
}