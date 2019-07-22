/*
 *  编译器lexer部分
 *  目标     1、实现ident、int_const、float_const的存储
 *          2、实现char_const的存储
 *          3、实现剩下的几种赋值语句
 *          4、指针应该没有时间和智力实现
 *          5、struct等复杂类型应该也没有时间和智力实现
 *          6、函数调用应该有机会实现
 */
#include "mycompiler.h"
#define IDLENGH 32

const char* TYPE[] = {"auto", "short", "int", "long", "float", "double", "char", "struct", "union", "enum", "typedef", "const", "unsigned", "signed", "extern", "register", "static", "volatile", "void", "else", "switch", "case", "for", "do", "while", "goto", "continue", "break", "default", "sizeof", "return", "if", "ident", "int_const", "float_const", "char_const", "string", "==", "=", "++", "+", "--", "-", "!=", "<=", "<", ">=", ">", "||", "&&", "!", "%", "*", "(", ")", "[", "]", "{", "}", ";", "/"};
token words[2048];
char IDENT_TEXT[128][IDLENGH];
char CHAR_TEXT[128];
int INT_TEXT[128];
float FLOAT_TEXT[128];

void lexer ();
void printlexererror(int, int);

void lexer () {
    int fin = 0;
    int state = 0;
    int row = 1;
    int col = 0;
    token* ptk = words;
    char* pit = IDENT_TEXT[0];
    char* pcc = CHAR_TEXT;
    int* pic = INT_TEXT;
    float* pfc = FLOAT_TEXT;
    char c;
    
    //每次循环获得一个token
    while (!fin) {
        //state2 行注释内部
        if (state == 1) {
            while ((c = fgetc(fp)) && c != '\n');
            ungetc(c, fp);
            row += 1;
            col = 0;
            state = 0;
            continue;
        }

        //state3 块内部注释
        if (state == 2) {
            while ((c = fgetc(fp)) && c != '*') {
                if (c == '\n') row += 1;
            }
            if (c < 0) printf("LEXER ERROR! EOF before block comments.\n");
            if ((c = fgetc(fp)) && c == '/') {state = 0; continue;}
            ungetc(c, fp);
            continue;
        }

        //state0 正文内容
        //过滤空白符号
        while ((c = fgetc(fp)) && (c == ' '|| c == '\t'|| c == '\n')){
            if (c == '\n') {row += 1; col = 0;}
            else col += 1;
        }

        //处理标识符&关键字
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            int i = 0;
            do{*(pit + i) = c; i++; col += 1;}
            while((c = fgetc(fp)) && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')));
            ungetc(c, fp);
            *(pit + i) = 0;  //拼接标识符或者关键字
            i = 0;
            for (; i < 32; i++) { //识别是否为关键字
                if (!strcmp(pit, TYPE[i])) {
                    ptk->name = i; ptk++; break;
                }
            }
            if (i == 32) { //为标识符
                ptk->text = pit; 
                pit += IDLENGH; 
                ptk->name = IDENT; 
                ptk++; 
                continue;
            }
        }

        //检查整数类型
        /*
        int base = 0;
        pit[0] = c; pit[1] = fgetc(fp); pit[2] = fgetc(fp);
        if (((pit[0] > '0' && pit[2] <= '9') && (pit[1] >= '0' && pit[1] <= '9') && (pit[2] >= '0' && pit[2] <= '9'))
           ||(pit[0] == '-'                  && (pit[1] > '0' && pit[1] <= '9')  && (pit[2] >= '0' && pit[2] <= '9'))
           ||(pit[0] == '0'                  && (pit[1] > '0' && pit[1] <= '9')  && (pit[2] >= '0' && pit[2] <= '9'))
           ||(pit[0] == '-'                  && pit[1] == '0'                    && (pit[2] > '0' && pit[2] <= '9'))
           ||(pit[0] == '0'                  && pit[1] == 'x'                    && (pit[2] > '0' && pit[2] <= '9'))
           ||(pit[0] == '-'                  && pit[1] == '0'                    && pit[2] == 'x')) { //负数和三种进制
            int i = 3;
            while ((c = fgetc(fp)) && (c >= '0' && c <= '9')) {
                pit[i] = c;
                i++;
            }
            pit[i] == 0; //收尾构造字符串
            col += i;
        }
        if (c == ' ' || c == '\t' || c == '\n') { //确认接收到数字常量
            ungetc(c, fp); ptk->name = INT_CONST; ptk->text = pit; pit += IDLENGH; ptk++;
        }
        else { //错误读取
            ungetc(pit[2], fp); ungetc(pit[1], fp); c = pit[0]; //恢复原状
            printlexererror(row, col);
        }
        */

        //处理float常量

        //处理char常量

        //处理双目算术运算符、关系运算符、逻辑符号、和赋值符号
        switch (c) {
            case '=' : c = fgetc(fp); if (c == '=') { ptk->name = EQ;     ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = ASSIGN; ptk++; col += 1; break;
            case '+' : c = fgetc(fp); if (c == '+') { ptk->name = INC;    ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = ADD;    ptk++; col += 1; break;
            case '-' : c = fgetc(fp); if (c == '-') { ptk->name = DEC;    ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = SUB;    ptk++; col += 1; break; 
            case '!' : c = fgetc(fp); if (c == '=') { ptk->name = NE;     ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = NO;     ptk++; col += 1; break;
            case '<' : c = fgetc(fp); if (c == '=') { ptk->name = LE;     ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = LT;     ptk++; col += 1; break;
            case '>' : c = fgetc(fp); if (c == '=') { ptk->name = GE;     ptk++; col += 2; break;}
                                      ungetc(c, fp);  ptk->name = GT;     ptk++; col += 1; break;
            case '|' : c = fgetc(fp); if (c == '|') { ptk->name = LOR;    ptk++; col += 2; break;}
                                      ungetc(c, fp); printlexererror(row, col);  col += 1; break;
            case '&' : c = fgetc(fp); if (c == '&') { ptk->name = LAN;    ptk++; col += 2; break;}
                                      ungetc(c, fp); printlexererror(row, col);  col += 1; break;
            case '%' : ptk->name = MOD; ptk++; col += 1; break;
            case '*' : ptk->name = MUL; ptk++; col += 1; break;
            case '(' : ptk->name = LP;  ptk++; col += 1; break;
            case ')' : ptk->name = RP;  ptk++; col += 1; break;
            case '[' : ptk->name = LK;  ptk++; col += 1; break;
            case ']' : ptk->name = RK;  ptk++; col += 1; break;
            case '{' : ptk->name = LC;  ptk++; col += 1; break;
            case '}' : ptk->name = RC;  ptk++; col += 1; break;
            case ';' : ptk->name = SEM; ptk++; col += 1; break;
            case '/' : c = fgetc(fp); if (c == '/') {state = 1; break;}
                                 else if (c == '*') {state = 2; break;}
                                    else {ungetc(c, fp); ptk->name = DIV; ptk++; col +=1; break;}
            default :
                if (feof(fp)) { fin = 1; break;}
                col += 1;
                printlexererror(row, col);
                continue;    //报错
        }
    }

    //单词表完毕之后设置边界，在后一个元素上的name标记为-1
    ptk->name = -1;

    //打印单词表
    for(token* i = words; i->name >=0; i++) {
        if (i->name == IDENT) printf("%s : %s\n", TYPE[i->name], i->text);
        else if (i->name ==INT_CONST) printf("%s : %s\n", TYPE[i->name], i->text);
        else printf("%s\n", TYPE[i->name]);

    }
}

void printlexererror(int row, int col) {
    printf("!!! row%dcol%d : lexer error : wrong token !!!\n", row, col);
}