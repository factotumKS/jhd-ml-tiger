#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define IMAX 32
#define SMAX 100
#define KWNUM 11

enum token_kind {
    INT=257, FLOAT, CHAR, IF, ELSE, DO, WHILE, FOR, CONTINUE, BREAK, RETURN,//10关键字
    ERROR_TOKEN, IDENT, INT_CONST, FLOAT_CONST, CHAR_CONST,
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE, EQ, NEQ, ASSIGN, AND, OR,
    LP, RP, L2, R2, L3, R3, COMMA, SEMI, SS, STR,
    PROGRAM,            //程序
    EXT_DEF_LIST,       //外部定义序列
    EXT_VAR_DEF,        //外部变量定义
    EXT_ARR_DEF,        //外部数组定义
    EXT_VAR_LIST,       //外部变量序列
    FUN_DEF,            //函数定义
    FORMAL_PARA,        //形参
    FORMAL_PARA_LIST,   //形参列表
    FUN_CALL,           //函数调用
    ARRAY_CALL,         //数组调用
    ACTUAL_PARA_LIST,   //实参列表
    STATEMENT_BLOCK,    //复合语句
    STATEMENT_LIST,     //语句序列
    LOC_VAR_DEF,        //局部变量定义
    LOC_VAR_LIST,       //局部变量序列
    LOC_ARR_DEF,        //局部数组定义
    EXPRESSION,         //表达式
    IF_ELSE,
};

char* keepwords[] = {
    "int", "float", "char", "if", "else", "do", "while", "for", "continue", "break", "return",
    "error_token", "id", "int_const", "float_const", "char_cosnt",
    "+", "-", "*", "/", "%", ">", "<", ">=", "<=", "==", "!=", "=", "&&", "||",
    "(", ")", "[", "]", "{", "}", ",", ";", "#",
};

typedef union point_ {
    int* pi;  //指向整数常量
    float* pf;//指向浮点数常量
    char* pc; //指向字符常量
} point;

typedef struct node_ {
    int name;         //token名字
    point text;       //token值的指针  
    struct node_* bro;//兄弟结点
    struct node_* chi;//孩子结点
} ASTnode;

typedef struct stack_ {
    ASTnode* A[SMAX];
    ASTnode** top;
} stack;

//全局变量------------------------------------------------------------------
FILE* fp;   //指向被解析的源文件
int w;      //当前读取的tokenname
int row;    //当前行号
int pe;     //存在解析错误
int layer;  //在<语句>的多重递归中计数
int look;   //在<语句>的多重递归中记录是否已经向后面看了一个非自身token
char ident_text[IMAX];
char ident_text0[IMAX];
int token_name;
int int_value;
float float_value;
char char_value;

//函数声明------------------------------------------------------------------
//1、AST数据结构部分
ASTnode* AST_mknode(int n, void* t, int l);     //创建新结点
int      AST_getname(ASTnode* r);               //得到结点名字
char*    AST_gettext(ASTnode* r);               //得到IDENT结点的文本
int*     AST_getint(ASTnode* r);                //得到INT_CONST的值
float*   AST_getfloat(ASTnode* r);              //得到FLOAT_CONST的值
char*    AST_getchar(ASTnode* r);               //得到CHAR_CONST的值
ASTnode* AST_getchi(ASTnode* r, int n);         //得到第n个孩子结点
int      AST_getchiname(ASTnode* r, int n);     //得到第n个孩子的结点token名字
ASTnode* AST_getbro(ASTnode* r, int n);         //得到第n个兄弟结点
int      AST_getbroname(ASTnode* r, int n);     //得到第n个兄弟的结点token名字
void     AST_addchild(ASTnode* r, ASTnode* c);  //为r添加一个孩子
//2、Stack数据结构部分
stack*   stack_init();               //
void     push(stack* s, ASTnode* n); //
int      pop(stack* s, ASTnode** n);  //
ASTnode* gettop(stack* s);           //
int      gettopname(stack* s);       //
//3、词法分析部分
int      gettoken();                 //
int      gettoken0();                //
int      match(int c);               //检测当前token是否为给出
int      getmatch(int c);            //检测下一个token是否为给出
int      match_error(int c, const char* s);
int      getmatch_error(int c, const char* s);
int      matchType_error(const char* s);
int      matchOp(int c);             //检测是否为运算符
void     layerUp();                  //递归增加一层
void     layerDown();                //递归减少一层，并在完全结束之后负责额外读取
void     looked();
int      haslooked();
int      rank(int t);
//4、语法分析部分，递归下降
ASTnode* program();         //<程序>入口
ASTnode* ExtDefList();      //<外部定义序列>循环
ASTnode* ExtDef();          //<外部变量定义>、<外部数组定义>、<函数定义>、选择
ASTnode* ExtVarDef();       //<外部变量定义>生成，<外部变量序列>循环
ASTnode* ExtArrDef();
ASTnode* funDef();          //<函数定义>生成
ASTnode* formalPara();      //<形参序列>循环
ASTnode* body();            //<语句>或<复合语句>判断
ASTnode* statementBlock();  //<复合语句><复合语句序列>循环
ASTnode* statement();       //<语句>生成
ASTnode* expr();            //<表达式>生成
ASTnode* funCall();         //<函数调用>生成
ASTnode* arrayCall();       //<数组调用>生成
ASTnode* LocArrDef();       //<数组>生成，用于处理表达式中的数组
ASTnode* LocVarDef();       //<局部变量定义>，<局部变量序列>循环
//5、打印排版部分
void     AST_show(ASTnode* r, int n);
void     pt(int t);
void     AST_output();

//函数定义-------------------------------------------------------------------
//1、AST数据结构部分
ASTnode* AST_mknode(int n, void* t, int l) {
    ASTnode* r = malloc(sizeof(ASTnode));
    if(!n) {
        match_error(0, "创建结点失败");
        return NULL;
    }
    r->name = n; r->bro = NULL; r->chi = NULL;
    if(l == INT) {
        int* n = malloc(sizeof(int));
        if(!n) {
            match_error(0, "创建INT_CONST结点失败");
            return NULL;
        }
        int* tmp = t;
        *n = *tmp; //将指针转换为相应指针并转移值
        (r->text).pi = n;
        return r;
    }
    else if(l == FLOAT) {
        float* n = malloc(sizeof(float));
        if(!n) {
            match_error(0, "创建FLOAT_CONST结点失败");
            return NULL;
        }
        float* tmp = t;
        *n = *tmp;
        (r->text).pf = n;
        return r;
    }
    else if(l == CHAR) {
        char* n = malloc(sizeof(char));
        if(!n) {
            match_error(0, "创建CHAR_CONST结点失败");
            return NULL;
        }
        char* tmp = t;
        *n = *tmp; 
        (r->text).pc = n;
        return r;
    }
    else if(l == STR) {
        char* tmp = t; //先转化为字符指针更安全
        char* n = malloc(strlen(tmp) + 1);
        if(!n) {
            match_error(0, "创建IDENT结点失败");
            return NULL;
        }
        strcpy(n, tmp); //字符串拷贝
        (r->text).pc = n;
        return r;
    }
    else return r;
}
int      AST_getname(ASTnode* r) {
    return r->name;
}
char*    AST_gettext(ASTnode* r) {
    return (r->text).pc;
}
int*     AST_getint(ASTnode* r) {
    return (r->text).pi;
}
float*   AST_getfloat(ASTnode* r) {
    return (r->text).pf;
}
char*    AST_getchar(ASTnode* r) {
    return (r->text).pc;
}
ASTnode* AST_getchi(ASTnode* r, int n) {
    if (n <= 0) return NULL;
    if (!r->chi) return NULL;
    ASTnode* nr = r->chi;
    int i = 1; //已经得到第一个孩子
    while(i < n) {
        if(nr->bro) nr = nr->bro;
        else return NULL; //如果数目过大没有这个孩子
        i++;
    }
    return nr;
}
int      AST_getchiname(ASTnode* r, int n) {
    ASTnode* nr = AST_getchi(r, n);
    if (nr) return nr->name;
    else return 0; //处理空情况
}
ASTnode* AST_getbro(ASTnode* r, int n) {
    if(n <= 0) return NULL;
    int i = 0;
    ASTnode* nr = r;
    while(i < n) {
        if(nr->bro) nr = nr->bro;
        else return NULL;
        i++;
    }
    return nr;
}
int      AST_getbroname(ASTnode* r, int n) {
    ASTnode* nr = AST_getbro(r, n);
    if (nr) return nr->name;
    else return 0; //处理空情况
}
void     AST_addchild(ASTnode* r, ASTnode* c) {
    if(!r) {
        match_error(0, "被添加结点的树是空的\n"); 
        return;
    }
    if(!r->chi) {
        r->chi = c;
    }
    else {
        ASTnode* nr = r->chi;
        while(nr->bro) nr = nr->bro;
        nr->bro = c;
    }
}

//2、stack数据结构部分
stack*   stack_init() {
    stack* s = malloc(sizeof(stack));
    s->top = NULL; //top指针指向数组第一个位置
    return s;
}
void     push(stack* s, ASTnode* n) {
    ASTnode** u = s->A + SMAX;
    if (s->top == u) {
        printf("栈上溢\n");
        return;
    }
    if (!s->top) { //第一次push
        s->top = s->A;
    }
    (s->top)++; *(s->top) = n;
}
int      pop(stack* s, ASTnode** n) {
    ASTnode** b = s->A;
    if (s->top == NULL) {
        printf("栈下溢\n");
        return 0;
    }
    if (s->top == b) { //最后一次pop
        *n = *(s->top);
        s->top = NULL;
        return 1;
    }
    *n = *(s->top); (s->top)--; return 1;
}
ASTnode* gettop(stack* s) {
    if(!s->top) return NULL;
    return *(s->top);
}
int      gettopname(stack* s) {
    ASTnode* nr = gettop(s);
    if(!nr) return 0;
    return nr->name;
}

//3、词法分析部分
int      gettoken() {
    static int num = 0;
    printf("第%3d个token: ", ++num);
    int w = gettoken0();
    if (w == EOF) printf("文件尾\n");
    else if (w == IDENT) printf("IDENT %s\n", ident_text);
    else if (w == INT_CONST) printf("整数常量 %d\n", int_value);
    else if (w == FLOAT_CONST) printf("浮点数常量 %f\n", float_value);
    else if (w == CHAR_CONST) printf("字符常量 %c\n", char_value);
    else printf("%s\n", keepwords[w-INT]);
    return w;
}
int      gettoken0() {
    if (row == 0) row++; //初始化行号
    if (feof(fp)) return EOF; //保险措施
    char c;
    
    //处理全部空白符
    while ((c = fgetc(fp)) && (c == ' '|| c == '\t'|| c == '\n')){
        if (c == '\n') row += 1;
    }

    //处理标识符 关键字
    if ((c>='a' && c<= 'z') || (c>='A' && c<= 'Z')) {
        int i = 0;
        do {ident_text[i] = c; i++;}
        while ((c = fgetc(fp)) && ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c <='9')));
        ident_text[i] = 0; //封！
        ungetc(c, fp);
        for (int i = 0; i < KWNUM; i++) {  //判断是否为关键字
            if (!(strcmp(ident_text, keepwords[i]))) return i+INT;
        }
        return IDENT; //返回标识符
    }

    //处理整数int，浮点数float
    if (c >= '0' && c <= '9') {
        int val = c - '0';
        if (val > 0) { //十进制数
            while ((c = fgetc(fp)) &&  c >= '0' && c <= '9') {
                val = val * 10 + c - '0';
            }
            if (c == '.') { //浮点数，不考虑负数，不考虑e和E的科学技术法格式
                float val0 = 0;
                while ((c = fgetc(fp)) && c >= '0' && c <= '9') {
                    val = val / 10 + c - '0';
                }
                float_value = val + val0;
                ungetc(c, fp); return FLOAT_CONST;
            }
            int_value = val; ungetc(c, fp); return INT_CONST;
        } 
        else if ((c = fgetc(fp)) && (c == 'x' || c == 'X')) { //十六进制数
            while ((c = fgetc(fp)) && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                val = val * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
            }
            int_value = val; ungetc(c, fp); return INT_CONST;
        }
        else { //八进制数，或者0，到这里已经多读了一个不是x或X的字符，需要复原
            ungetc(c, fp);
            while ((c = fgetc(fp)) && c >= '0' && c <= '7') { //最终仍然多读取一个
                val = val * 8 + c - '0';
            }
            int_value = val; ungetc(c, fp); return INT_CONST;
        }
    }
 
    //处理字符char
    if (c == '\'') { //字符常量
        char_value = fgetc(fp);
        c = fgetc(fp); //消化掉后面的引号
        return CHAR_CONST;
    }

    //处理各种符号
    switch (c) {
        case '=' : c = fgetc(fp); if (c == '=') return EQ;  else {ungetc(c, fp); return ASSIGN;}
        case '!' : c = fgetc(fp); if (c == '=') return NEQ; else {ungetc(c, fp); return ERROR_TOKEN;} 
        case '<' : c = fgetc(fp); if (c == '=') return LE;  else {ungetc(c, fp); return LT;} 
        case '>' : c = fgetc(fp); if (c == '=') return GE;  else {ungetc(c, fp); return GT;}   
        case '|' : c = fgetc(fp); if (c == '|') return OR;  else {ungetc(c, fp); return ERROR_TOKEN;}
        case '&' : c = fgetc(fp); if (c == '&') return AND; else {ungetc(c, fp); return ERROR_TOKEN;} 
        case '+' : return ADD;
        case '-' : return SUB;
        case '%' : return MOD;
        case '[' : return L2;
        case ']' : return R2;
        case '{' : return L3;
        case '}' : return R3;
        case '(' : return LP;
        case ')' : return RP;
        case '*' : return MUL;
        case '/' : return DIV;
        case ',' : return COMMA;
        case ';' : return SEMI;
        default : if (feof(fp)) return EOF;
                else return ERROR_TOKEN;
    }
}
int      match(int c) {
    if(w == c) return 1;
    return 0;
}
int      getmatch(int c) {
    w = gettoken();
    return (match(c));
}
int      match_error(int c, const char* s) {
    if(!match(c)) {
        printf("！！！第%d行：%s\n", row, s);
        pe = 1; return 1; //表示检测到错误
    }
    return 0;
}
int      getmatch_error(int c, const char* s) {
    if(!getmatch(c)) {
        printf("\t！！！%s\n", s);
        pe = 1; return 1; //表示检测到错误
    }
    return 0;
}
int      matchType_error(const char* s) {
    if(w==INT||w==FLOAT||w==CHAR) return 0;
    match_error(0, s);
}
int      matchOp(int c) {
    if(w==LP||w==RP||w==MUL||w==DIV||w==MOD||w==ADD||w==SUB||
       w==GT||w==LT||w==GE||w==LE||w==NEQ||w==EQ||
       w==OR||w==AND||w==ASSIGN||w==COMMA||w==SS)
       return 1;
    return 0;
}
void     layerUp() {
    if(!layer) look = 0;
    layer++;
}
void     layerDown() {
    layer--;
    if(!layer && !look) w = gettoken();
}
void     looked() {
    look = 1;
}
int      haslooked() {
    return look == 1;
}
int      rank(int t) {
    switch (t) {
        case LP : //需特殊处理//左括号在下，任何输入的符号都可以堆积在上面，除了右括号和终止符
        case RP : return 1; //右括号在上，任何已输入的符号都会被清楚，遇到左括号会相互抵消
        case MUL:
        case DIV:
        case MOD: return 3;
        case ADD:
        case SUB: return 4;
        case GT :
        case LT :
        case GE :
        case LE : return 5;
        case NEQ: 
        case EQ : return 6;
        case AND: return 11;
        case OR : return 12;
        case ASSIGN: return 14;
        case COMMA: return 15;
        case SS : return 1000; //输入终止符，前面的运算符号都要执行，优先级最低
        default : return 0;
    }
}

//4、语法分析部分，递归下降
//默认协议：期待之前读取过，为后续操作读取token
//自主协议：不期待，不为后续操作读取token
//懒惰协议：期待之前读取过，不为后续操作读取token
ASTnode* program() {
    ASTnode* r = AST_mknode(PROGRAM, NULL, 0);
    w = gettoken(); //程序的”第一动力“,这个token将会在外部定义中第一次被利用到
    AST_addchild(r, ExtDefList());
    if(!AST_getchi(r, 1)) {
        printf("<程序>检测到错误\n");
        return NULL;
    }
    else printf("<程序>正常返回\n");
    return r;
}
ASTnode* ExtDefList() {
    ASTnode* r = AST_mknode(EXT_DEF_LIST, NULL, 0);
    if(w == EOF) return NULL; //确保到文件尾，返回<外部定义序列结点序列>
    ASTnode* p = ExtDef();
    if(!p) return r;
    AST_addchild(r, p);
    AST_addchild(r, ExtDefList());
    return r;
}
ASTnode* ExtDef() {
    if(pe) return NULL;
    if(matchType_error("外部定义开头应该为类型")) return NULL;
    token_name = w; //保存类型说明符
    if(getmatch_error(IDENT, "外部定义需要标识符")) return NULL;
    strcpy(ident_text0, ident_text); //保存第一个变量名或函数名到ident_text0
    if(getmatch(LP)) {
        w = gettoken();
        return funDef();
    }
    if(match(L2)) {
        w = gettoken();
        return ExtArrDef();
    }
    return ExtVarDef();
}
ASTnode* ExtArrDef() {
    if(pe) return NULL;
    printf("检测到外部数组定义\n");
    ASTnode* r = AST_mknode(EXT_ARR_DEF, NULL, 0);
    AST_addchild(r, AST_mknode(token_name, NULL, 0));
    AST_addchild(r, AST_mknode(IDENT, ident_text0, STR));
    if(match_error(INT_CONST, "外部数组必须为整数大小")) return NULL;
    AST_addchild(r, AST_mknode(INT_CONST, &int_value, INT));
    if(getmatch_error(R2, "外部数组需要反方括号")) return NULL;
    if(getmatch_error(SEMI, "外部数组声明需要分号结尾")) return NULL;
    w = gettoken(); return r;
}
ASTnode* ExtVarDef() {
    if(pe) return NULL;
    printf("检测到外部变量定义\n");
    ASTnode* r = AST_mknode(EXT_VAR_DEF, NULL, 0);
    AST_addchild(r, AST_mknode(token_name, NULL, 0));
    ASTnode* p = AST_mknode(EXT_VAR_LIST, NULL, 0);
    AST_addchild(r, p);

    AST_addchild(p, AST_mknode(IDENT, ident_text0, STR));
    ASTnode* q = AST_mknode(EXT_VAR_LIST, NULL, 0);
    AST_addchild(p, q);
    p = q;
    q = AST_mknode(EXT_VAR_LIST, NULL, 0);
    while(1) {
        if(match(SEMI)) {
            w = gettoken();
            return r;
        }
        if(match(COMMA)) {
            if(getmatch_error(IDENT, "外部变量定义需要标识符")) return NULL;
            AST_addchild(p, AST_mknode(IDENT, ident_text, STR));
            AST_addchild(p, q); //q,作为p的第二个孩子
            p = q;
            q = AST_mknode(EXT_VAR_LIST, NULL, 0);
            w = gettoken();
            continue;   
        }
        match_error(SEMI, "外部变量定义缺少逗号分号而终止"); //不满足上面两个条件必然是出错了
        return NULL;
    }
} 
ASTnode* funDef() {
    if(pe) return NULL;
    ASTnode* r = AST_mknode(FUN_DEF, ident_text0, STR); //函数名保存
    AST_addchild(r, AST_mknode(token_name, NULL, 0));
    AST_addchild(r, formalPara());
    if(match(SEMI)) { 
        printf("检测到函数声明\n");
        w = gettoken();
        return r;
    }
    if(match(L3)) {
        printf("检测到函数定义\n");
        w = gettoken();
        AST_addchild(r, statementBlock());
        return r;
    }
    match_error(SEMI, "不是函数声明也不是函数定义"); 
    return NULL;
}  
ASTnode* formalPara(){
    if(pe) return NULL;
    ASTnode* r = AST_mknode(FORMAL_PARA, ident_text0, STR);
    ASTnode* p = AST_mknode(FORMAL_PARA_LIST, NULL, 0);
    AST_addchild(r, p);
    ASTnode* q = AST_mknode(FORMAL_PARA_LIST, NULL, 0);
    if(match(RP)) { //空参数
        w = gettoken();
        return r;
    }
    while(1) {
        if(matchType_error("形参需要类型符号")) return NULL;
        AST_addchild(p, AST_mknode(w, NULL, 0));
        if(getmatch_error(IDENT, "形参需要标识符")) return NULL;
        AST_addchild(p, AST_mknode(IDENT, ident_text, STR));
        AST_addchild(p, q);
        w = gettoken();
        if(match(RP)) {
            w = gettoken();
            return r;
        }
        if(match(COMMA)) {
            p = q;
            q = AST_mknode(FORMAL_PARA_LIST, NULL, 0);
            w = gettoken();
            continue;
        }
        match_error(RP, "形参缺少分号和逗号导致终止");
        return NULL;
    }
}
ASTnode* statementBlock() {
    if(pe) return NULL;
    printf("检测到复合语句\n");
    ASTnode* r = AST_mknode(STATEMENT_BLOCK, NULL, 0);
    ASTnode* p = AST_mknode(STATEMENT_LIST, NULL, 0);
    AST_addchild(r, p);
    ASTnode* q = AST_mknode(STATEMENT_LIST, NULL, 0);
    while(1) {
        ASTnode* n = statement();
        if(!n) return r;
        AST_addchild(p, n);
        AST_addchild(p, q);
        p = q;
        q = AST_mknode(STATEMENT_LIST, NULL, 0);
    }
}
ASTnode* body() {
    if(pe) return NULL;
    if(match(L3)) {
        w = gettoken();
        return statementBlock();
    }
    return statement();
}
ASTnode* statement() {
    if(pe) return NULL;
    if(w == EOF) return NULL;
    ASTnode *r, *r1, *r2, *r3, *r4;
    switch(w) {
        case IF: //IF语句
            layerUp();
            if(getmatch_error(LP, "IF语句条件错误")) return NULL;
            w = gettoken(); r1 = expr();
            if(match_error(RP, "IF条件未闭合")) return NULL;
            w = gettoken(); r2 = body();
            if(haslooked()) { //如果已经看过下一个，即确认终止（处理嵌套if）
                printf("检测到嵌套IF语句\n");
                r = AST_mknode(IF, NULL, 0);
                AST_addchild(r, r1); //下挂条件结点
                AST_addchild(r, r2); //if子句结点
            }
            else if(match(ELSE)) {
                printf("检测到IF_ELSE语句\n");
                w = gettoken(); r3 = body();
                r = AST_mknode(IF_ELSE, NULL, 0);
                AST_addchild(r, r1); //下挂条件结点
                AST_addchild(r, r2); //if子句结点
                AST_addchild(r, r3); //else子句结点
                looked();
            }
            else {
                printf("检测到IF语句\n");
                r = AST_mknode(IF, NULL, 0);
                AST_addchild(r, r1); //下挂条件结点
                AST_addchild(r, r2); //if子句结点
                looked(); //标记看过下一个而且不是else
            }
            layerDown(); return r;
        case WHILE:
            layerUp();
            printf("检测到WHILE语句\n");
            if(getmatch_error(LP, "while条件缺少左括号")) return NULL;
            w = gettoken(); r1 = expr();
            if(match_error(RP, "while条件缺少右括号")) return NULL;
            w = gettoken(); r2 = body(); looked();
            r = AST_mknode(WHILE, NULL, 0);
            AST_addchild(r, r1); //添加循环条件
            AST_addchild(r, r2); //添加while循环体
            layerDown(); return r;
        case FOR:
            layerUp();
            printf("检测到for语句\n");
            if(getmatch_error(LP, "for循环缺少左括号")) return NULL;
            w = gettoken(); r1 = statement();
            r2 = expr(); //上面有额外读取
            if(match_error(SEMI, "for循环初始化")) return NULL;
            w = gettoken(); r3 = expr();
            if(match_error(RP, "for循环结尾语句错误")) return NULL;
            w = gettoken(); r4 = body(); looked();
            r = AST_mknode(FOR, NULL, 0);
            AST_addchild(r, r1); //添加初始部分子树语句  
            AST_addchild(r, r2); //添加循环条件子树语句              
            AST_addchild(r, r3); //添加补充条件子树语句
            AST_addchild(r, r4); //添加循环部分子树语句
            layerDown(); return r;
        case IDENT: //（最常见的）表达式语句
            printf("检测到表达式语句\n");
            w = gettoken(); r = expr();
            if(match_error(SEMI, "表达式语句结束错误")) return NULL;
            w = gettoken(); return r;
        case CONTINUE:
            printf("检测到continue语句\n");
            r = AST_mknode(CONTINUE, NULL, 0);
            w = gettoken(); return r;
        case BREAK:
            printf("检测到break语句\n");
            r = AST_mknode(BREAK, NULL, 0);
            w = gettoken(); return r;
        case RETURN: //返回语句
            printf("检测到return语句\n");
            r = AST_mknode(RETURN, NULL, 0);
            w = gettoken(); r1 = expr();
            if(match_error(SEMI, "表达式语句结束错误")) return NULL;
            w = gettoken(); return r;
        case INT: //局部变量声明
        case CHAR: //局部变量声明
        case FLOAT: //局部变量声明
            token_name = w;
            w = gettoken();
            strcpy(ident_text0, ident_text);
            if(getmatch(L2)) { //数组声明
                w = gettoken();
                r = LocArrDef();
                return r;
            }
            else {
                r = LocVarDef();
                return r;
            }
        case R3: //复合语句结束的标志
            printf("检测到复合语句结束\n");
            w = gettoken();
            return NULL;
        default : //无法识别的东西
            match_error(0, "违反语句语法的句首token");
            return NULL;
    }
}
ASTnode* expr() {
    if(pe) return NULL;
    int lp = 0, rp = 0;
    stack* op = stack_init(); //运算符栈
    stack* opn = stack_init();//操作数栈
    push(op, AST_mknode(SS, NULL, 0));
    int error = 0;
    while((!match(SS) || gettopname(op) != SS) && !error) {
        if(match(IDENT)) { //函数调用、数组调用、或标识符
            strcpy(ident_text0, ident_text);
            if(getmatch(LP)) { //确认为函数调用
                w = gettoken();
                push(opn, funCall());
                continue;
            }
            if(match(L2)) { //确认为数组调用
                w = gettoken();
                push(opn, arrayCall());
            }
            else {
                push(opn, AST_mknode(IDENT, ident_text0, STR));
                continue;
            }
        }
        else if(match(INT_CONST)) { //操作数栈：整数常量
            push(opn, AST_mknode(INT_CONST, &int_value, INT));
            w = gettoken(); continue;
        }
        else if(match(FLOAT_CONST)) { //操作数栈：浮点数常量
            push(opn, AST_mknode(INT_CONST, &float_value, INT));
            w = gettoken(); continue;
        }
        else if(match(CHAR_CONST)) { //操作数栈：字符常量
            push(opn, AST_mknode(CHAR_CONST, &char_value, INT));
            w = gettoken(); continue;
        }
        else if(lp <= rp && w == RP) { //括号数不匹配
            token_name = w; w = SS; continue;
        }
        else if(matchOp(w)) {
            //括号未正常闭合
            if ((gettopname(op)==LP && match(SS))||(gettopname(op)==SS && match(RP))) {
                error = 1;
                break;
            }
            //右括号消栈遇到左括号
            else if (gettopname(op)==LP && match(RP)) { //当右括号消栈遇到左括号
                ASTnode *t;
                if(!(pop(op, &t))) {error = 1; break;}
                w = gettoken(); continue;
            }
            //压栈
            else if ((rank(gettopname(op)) > rank(w)) || (gettopname(op)==LP && !match(RP))) {
                if(gettopname(op)==LP && !match(RP)) lp++; //对左括号数量进行计数
                push(op, AST_mknode(w, NULL, 0));
                w = gettoken(); continue;
            }
            //执行运算符
            else if ((rank(gettopname(op)) <= rank(w)) || (gettopname(op)!=LP && match(RP))) {
                if(gettopname(op)!=LP && match(RP)) rp++; //对右括号数量进行计数
                ASTnode *t = NULL, *t1 = NULL, *t2 = NULL;
                if(!(pop(op, &t)))   {error = 1; break;}
                if(!(pop(opn, &t2))) {error = 1; break;}
                if(!(pop(opn, &t1))) {error = 1; break;}
                AST_addchild(t, t1); AST_addchild(t, t2);
                push(opn, t); //不需要再读入任何token，可能就这样一口气下降到底
                continue;
            }
            else; //该情况不可能存在
        }
        else { //不在运算符中的字符
            token_name = w; w = SS;
        }
    }
    if(!error) { //没有中途发生语法错误
        ASTnode* r = AST_mknode(EXPRESSION, NULL, 0);
        ASTnode* p = NULL; pop(opn, &p); //弹出运算符栈中的唯一那个
        AST_addchild(r, p);
        w = token_name; //将结尾起止符改回原来的token名称用于下面的分析
        return r;
    }
    else {
        match_error(0, "检测到表达式中出现语法错误");
        return NULL;
    }
}
ASTnode* funCall() {
    if(pe) return NULL;
    printf("检测到表达式中函数调用\n");
    ASTnode* r = AST_mknode(FUN_CALL, NULL, 0);
    AST_addchild(r, AST_mknode(IDENT, ident_text0, STR));
    ASTnode* p = AST_mknode(ACTUAL_PARA_LIST, NULL, 0);
    ASTnode* q = AST_mknode(ACTUAL_PARA_LIST, NULL, 0);
    ASTnode* e;
    AST_addchild(r, p);
    while(1) {
        e = expr();
        AST_addchild(p, e);
        AST_addchild(p, q);
        if(match(RP)) {
            w = gettoken();
            return r;
        }
        if(match(COMMA)) {
            p = q;
            q = AST_mknode(ACTUAL_PARA_LIST, NULL, 0);
            w = gettoken();
            continue;
        }
        match_error(0, "函数调用中缺少逗号和分号导致终止");
        return NULL;
    }
}
ASTnode* arrayCall() {
    if(pe) return NULL;
    printf("检测到表达式中数组调用\n");
    ASTnode* r = AST_mknode(ARRAY_CALL, NULL, 0);
    AST_addchild(r, AST_mknode(IDENT, ident_text0, STR));
    if(match_error(INT_CONST, "数组下标必须为整形常量")) return NULL;
    AST_addchild(r, AST_mknode(INT_CONST, &int_value, INT));
    if(getmatch_error(R2, "数组下标必须用方括号框起来")) return NULL;
    return r;
}
ASTnode* LocVarDef() {
    if(pe) return NULL;
    printf("检测到局部变量定义\n");
    ASTnode* r = AST_mknode(LOC_VAR_DEF, NULL, 0);
    AST_addchild(r, AST_mknode(token_name, NULL, 0));
    ASTnode* p = AST_mknode(LOC_VAR_LIST, NULL, 0);
    AST_addchild(r, p);

    AST_addchild(p, AST_mknode(IDENT, ident_text0, STR));
    ASTnode* e;
    if(match(ASSIGN)) {
        w = gettoken();
        e = expr();
    }
    else e = AST_mknode(EXPRESSION, NULL, 0);
    AST_addchild(p, e);
    ASTnode* q = AST_mknode(LOC_VAR_LIST, NULL, 0);
    AST_addchild(p, q);
    p = q;
    q = AST_mknode(LOC_VAR_LIST, NULL, 0);
    while(1) {
        if(match(SEMI)) {
            w = gettoken();
            return r;
        }
        if(match(COMMA)) {
            if(getmatch_error(IDENT, "局部变量定义需要标识符")) return NULL;
            AST_addchild(p, AST_mknode(IDENT, ident_text, STR));
            if(getmatch(ASSIGN)) {
                w = gettoken(); //可能为逗号分号
                e = expr();
            }
            else e = AST_mknode(EXPRESSION, NULL, 0);
            AST_addchild(p, e);
            AST_addchild(p, q);
            
            p = q;
            q = AST_mknode(LOC_VAR_LIST, NULL, 0);
            continue;   
        }
        match_error(SEMI, "局部变量定义缺少逗号分号而终止"); //不满足上面两个条件必然是出错了
        return NULL;
    }
}
ASTnode* LocArrDef() {
    if(pe) return NULL;
    printf("检测到局部数组定义\n");
    ASTnode* r = AST_mknode(LOC_ARR_DEF, NULL, 0);
    AST_addchild(r, AST_mknode(token_name, NULL, 0)); //类型符
    AST_addchild(r, AST_mknode(IDENT, ident_text0, STR)); //标识符
    if(match_error(INT_CONST, "数组定义长度必须为整数")) return NULL;
    if(int_value <= 0) {
        match_error(0, "数组长度必须为正整数");
        return NULL;
    }
    if(getmatch_error(R2, "数组定义中放括号未闭合")) return NULL;
    /*
    if(getmatch(ASSIGN)) {
        if(getmatch_error(L3, "数组初始化必须以花括号开始")) return NULL;

    }
    else {
        return r;
    }*/
    if(getmatch_error(SEMI, "数组定义语句非正常结尾")) return NULL;
    return r;
}

//5、打印排版部分
void     AST_show(ASTnode* r, int n) {
    if(!r) return;
    ASTnode *ch1 = NULL, *ch2 = NULL, *ch3 = NULL;
    if(AST_getname(r) == PROGRAM) {
        printf("\n");
        for(int i = 0; i < 60; i++) printf("*");
        printf("\n下面开始打印AST\n");
        for(int i = 0; i < 60; i++) printf("*");
        printf("\n\n");
    }
    switch(AST_getname(r)) {
        case PROGRAM:       //<程序>
            AST_show(AST_getchi(r, 1), 0);
            return;
        case EXT_DEF_LIST:  //<外部定义序列>
            ch1 = AST_getchi(r, 1);
            ch2 = AST_getchi(r, 2);
            if(!ch1) return;
            AST_show(ch1, 0);
            printf("\n");
            AST_show(ch2, 0);
            return;
        case EXT_VAR_DEF:   //<外部变量定义>
            ch1 = AST_getchi(r, 1); //变量类型
            ch2 = AST_getchi(r, 2); //变量序列
            printf("外部变量定义：\n");
            pt(1); printf("类型：%s\n", keepwords[AST_getname(ch1)-INT]);
            pt(1); printf("变量：\n");
            AST_show(ch2, 0);
            return;
        case EXT_VAR_LIST:  //<外部变量序列>
            if(!AST_getchi(r, 1)) return;
            ch1 = AST_getchi(r, 1);
            ch2 = AST_getchi(r, 2);
            pt(2); printf("IDENT：%s\n", AST_gettext(ch1));
            AST_show(ch2, 0);
            return;
        case EXT_ARR_DEF:   //<外部数组定义>
        case FUN_DEF:       //<函数定义>
            ch1 = AST_getchi(r, 1); //返回值类型
            ch2 = AST_getchi(r, 2); //函数名
            ch3 = AST_getchi(r, 3); //（可能存在的）函数体
            if(ch3) {
                printf("函数定义：\n");
                pt(1); printf("返回值类型：%s\n", keepwords[AST_getname(ch1)-INT]);
                pt(1); printf("函数名：%s\n", AST_gettext(ch2));
                AST_show(ch3, 0); //打印复合语句了
            }
            else {
                printf("函数声明：\n");
                pt(1); printf("返回值类型：%s\n", keepwords[AST_getname(ch1)-INT]);
                pt(1); printf("函数名：%s\n", AST_gettext(ch2));
            }
            return;
        case :
        default :
            return;
    }
}
void     pt(int t) {
    for(int i = 0; i < t; i++) printf("\t");
}
void     AST_output() {
}

void main() {
    fp = fopen("test.c", "r");
    ASTnode* r = program();
    if(pe) return;
    AST_show(r, 0);
    //AST_output();
}