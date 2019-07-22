/*
*   BNF，作为下面代码的参考
*   <程序> ::= <外部定义序列>
*   <外部定义序列> ::= <外部定义> <外部定义序列>
*                |   <外部定义>
*   <外部定义> ::= <外部变量定义>
*             |   <函数定义>
*   <外部变量定义> ::= <类型说明符> <外部变量序列> ;
*   <类型说明符> ::= int | float | char
*   <外部变量序列> ::= ident , <外部变量序列>
*                |   ident
*   <函数定义> ::= <类型说明符> <函数名> (<形式参数序列>) <复合语句> 
*             |   <类型说明符> <函数名> (<形式参数序列>) ;
*   <形式参数序列> ::= <形式参数> , <形式参数序列>
*             |   <空>
*   <形式参数> ::= <类型说明符> 标识符
*   <语句或复合语句> ::= <语句> ;
*                  |   <复合语句>
*   <复合语句> ::= { <局部变量定义序列> <语句序列> }
*   <语句序列> ::= <语句> <语句序列>
*             |   <空> 
*   <局部变量定义序列> ::= <局部变量定义> <局部变量定义序列>
*                   |   <空>
*   <局部变量定义> ::= <类型说明符> <变量序列> ;
*   <语句> ::= <表达式> ; 
*         |   if (<表达式>) <语句> else <语句>
*         |   if (<表达式>) <语句> else <复合语句>
*         |   if (<表达式>) <复合语句> else <语句>
*         |   if (<表达式>) <复合语句> else <复合语句>
*         |   if (<表达式>) <语句>
*         |   if (<表达式>) <复合语句>
*         |   do <语句> ; while ( <表达式> )
*         |   do <复合语句> while ( <表达式> )
*         |   while (表达式) <语句> ;
*         |   while (表达式) <复合语句>
*         |   for ( <语句> ; <表达式> ; <语句> ) <语句> ;
*         |   for ( <语句> ; <表达式> ; <语句> ) <复合语句>
*         |   return <表达式> ; 
*         |   <表达式> ; 
*         |   continue ;
*         |   break ;
*   <表达式> ::= <表达式> + <表达式>
*           |   <表达式> - <表达式>
*           |   <表达式> * <表达式>
*           |   INT_CONST
*           |   IDENT
*           |   IDENT ( <实参序列> )
*           |   <表达式> == <表达式>
*           |   <表达式> != <表达式>
*           |   <表达式> > <表达式>
*           |   <表达式> >= <表达式>
*           |   <表达式> < <表达式>
*           |   <表达式> <= <表达式>
*           |   <表达式> = <表达式>
*           |   <表达式> > <表达式>
*   <实参序列> ::= <表达式> <实参序列>
*             |   <空>
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define IDLEN 32
#define KWNUM 11

enum token_kind { //词法分析返回的token种类
    //词法分析得到的token
    INT=0, FLOAT, CHAR, IF, ELSE, DO, WHILE, FOR, CONTINUE, BREAK, RETURN,//10关键字
    ERROR_TOKEN, IDENT, INT_CONST, FLOAT_CONST, CHAR_CONST,
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE, EQ, NEQ, ASSIGN, AND, OR,
    LP, RP, LC, RC, COMMA, SEMI, SS,/*栈中的起止符*/
    STRING,
    //语法分析中需要使用的内部结点标识
    PROGRAM,            //程序
    EXT_DEF_LIST,       //外部定义序列
    EXT_VAR_DEF,        //外部变量定义
    EXT_VAR_LIST,       //外部变量序列
    FUNC_DEF,           //函数定义
    FORMAL_PARA_LIST,   //形参列表
    ACTUAL_PARA_LIST,   //实参列表
    STATEMENT_BLOCK,    //复合语句
    STATEMENT_LIST,     //语句序列
    LOC_VAR_DEF,        //局部变量定义
    LOC_VAR_LIST,       //局部变量序列
    EXPRESSION,         //表达式
    IF_ELSE, DO_WHILE,  //特殊结点
};
char* TYPE[] = {
    "int", "float", "char", "if", "else", "do", "while", "for", "continue", "break", "return",
    "error_token", "id", "int_const", "float_const", "char_cosnt",
    "+", "-", "*", "/", "%", ">", "<", ">=", "<=", "==", "!=", "=", "&&", "||",
    "(", ")", "{", "}", ",", ";", "#",
};
typedef struct node{
    int name;   //表示名字
    void* text;  //表示内容
    struct node* brother;      //弟结点
    struct node* child;        //子结点
} ASTnode;
typedef struct chain{
    ASTnode* content;
    struct chain* pre;
    struct chain* next;
} stack;

int getRank(int t) {
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

//初始化一堆用于保存字符串和数字常量的空间
FILE* fp; //源文件指针
int w; //表示刚返回得到的类型
int row, col; //用于报错的行列号
//临时保存用，生成ASTnode时会动态分配空间并将内容转移
char token_text [IDLEN]; //gettoken之后，上一个标识符的text储存在这里
char token_text0[IDLEN]; //处理定义语句时需要保留某些标识符的text，所以保存在这里
int token_name; //处理定义语句时需要保存某些非标识符token的值，表达式结尾时也需要保存结束符号到底是谁？！
int token_int; //gettoken之后，上一个（正）整数常量的大小储存在这里
float token_float; //gettoken之后，上一个浮点数常量的大小储存在这里
char token_char; //gettoken之后哦，上一个字符常量的大小储存在这里
int pe; //记录错误的发生次数

void prepare();
int gettoken();
int gettoken0();
ASTnode* AST_mknode(int gname, void* gtext, int gdata);
void* AST_mktext(void* gtext, int gdata);
void AST_add_child(ASTnode* father, ASTnode* new_child);
void AST_clear(ASTnode* root);
stack* stack_init(); //用链表表示，第一个结点作为底部不保存
void push(stack** s, ASTnode* i);
int pop(stack** s, ASTnode** p);
ASTnode* gettop(stack* s);
int gettopname(stack* s);
void stack_clear(stack* t);
void stack_show(stack* t);
ASTnode* program();
ASTnode* ExtDefList();
ASTnode* ExtDef(); //处理外部定义序列，正确时返回子树根节点指针，否则返回NULL
ASTnode* ExtVarDef();
ASTnode* funcDef();
ASTnode* formalPara();
ASTnode* LocVarDef();
ASTnode* statementBlock();
ASTnode* statementList();
ASTnode* statement_statementBlock();
ASTnode* statement();
ASTnode* expression(); //传入结束符号，可以是反小括号或者分号
ASTnode* funcCall();
void AST_show(ASTnode* r, int t); //递归输出解析出的语法树
void AST_expression_show(ASTnode* r); //打印表达式
void AST_modify(ASTnode* r); //调整解析出的语法树

//预处理器-------------------------------------------------------
void prepare() {
    //去除注释，尚未实现
    return;
}
//
int gettoken() {
    static int num = 0;
    printf("第%d个token:", ++num);
    int w = gettoken0();
    if (w == EOF) printf("文件尾\n");
    else if (w == IDENT) printf("IDENT %s\n", token_text);
    else if (w == INT_CONST) printf("整数常量 %d\n", token_int);
    else if (w == FLOAT_CONST) printf("浮点数常量 %f\n", token_float);
    else if (w == CHAR_CONST) printf("字符常量 %c\n", token_char);
    else printf("%s\n", TYPE[w]);
    return w;
}

//语法分析，每次调用得到一个token，返回名字并将附带text保存在token_text、token_int等全局变量中-
int gettoken0() {
    if (row == 0) row++; //初始化行号
    if (feof(fp)) return EOF; //保险措施
    char c;
    for(int i = 0; i < IDLEN; i++) token_text[i] = 0;
    
    //处理全部空白符
    while ((c = fgetc(fp)) && (c == ' '|| c == '\t'|| c == '\n')){
        if (c == '\n') row += 1;
    }

    //处理标识符 关键字
    if ((c>='a' && c<= 'z') || (c>='A' && c<= 'Z')) {
        int i = 0;
        do {token_text[i] = c; i++;}
        while ((c = fgetc(fp)) && ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c <='9')));
        ungetc(c, fp);
        for (int i = 0; i < KWNUM; i++) {  //判断是否为关键字
            if (!(strcmp(token_text, TYPE[i]))) return i;
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
                token_float = val + val0;
                ungetc(c, fp); return FLOAT_CONST;
            }
            token_int = val; ungetc(c, fp); return INT_CONST;
        } 
        else if ((c = fgetc(fp)) && (c == 'x' || c == 'X')) { //十六进制数
            while ((c = fgetc(fp)) && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                val = val * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
            }
            token_int = val; ungetc(c, fp); return INT_CONST;
        }
        else { //八进制数，或者0，到这里已经多读了一个不是x或X的字符，需要复原
            ungetc(c, fp);
            while ((c = fgetc(fp)) && c >= '0' && c <= '7') { //最终仍然多读取一个
                val = val * 8 + c - '0';
            }
            token_int = val; ungetc(c, fp); return INT_CONST;
        }
    }
 
    //处理字符char
    if (c == '\'') { //字符常量
        token_char = fgetc(fp);
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
        case '(' : return LP;
        case ')' : return RP;
        case '{' : return LC;
        case '}' : return RC;
        case '*' : return MUL;
        case '/' : return DIV;
        case ',' : return COMMA;
        case ';' : return SEMI;
        default : if (feof(fp)) return EOF;
                else return ERROR_TOKEN;
    }
}

//数据结构：AST-----------------------------------------------------
//制作新结点    形参分别是结点类型，结点名，结点附带text位置，结点附带text类型
ASTnode* AST_mknode(int gname, void* gtext, int gdata) {
    ASTnode* root = malloc(sizeof(ASTnode));
    root->name = gname;
    root->child = NULL; root->brother = NULL;
    root->text = AST_mktext(gtext, gdata);
    return root;
}

//制作新text位置，动态分配空间并将临时得到的空间转移过来
void* AST_mktext(void* gtext, int gdata) {
    if (gdata == 0) return NULL;
    else if (gdata == STRING) { //复制字符串 ID
        char* new = malloc(sizeof(strlen(gtext)) + 1);
        char* t = gtext; strcpy(new, t);
        return new;
    }
    else if (gdata == INT) { //复制整数
        int* new = malloc(sizeof(int));
        int* t = gtext; *new = *t;
        return new;
    }
    else if (gdata == FLOAT) { //复制浮点数
        float* new = malloc(sizeof(float));
        float* t = gtext; *new = *t;
        return new;
    }
    else { //复制字符串
        char* new = malloc(sizeof(char));
        char* t = gtext; *new = *t;
        return new;
    }
}

//增加已有结点作为孩子
void AST_add_child(ASTnode* father, ASTnode* new_child) {
    ASTnode* node = father->child;
    if (!node) {father->child = new_child; return;}//首个孩子
    while(node->brother) node = node->brother; //非首个孩子
    node->brother = new_child; //收为次子
}

//递归地删除root为根的整颗树
void AST_clear(ASTnode* root) {
    if (root->child) AST_clear(root->child);
    if (root->brother) AST_clear(root->brother);
    if (root->text) free(root->text);
    free(root);
}

//数据结构：结点栈-------------------------------------------------
//初始化一个栈，传入需要的值
stack* stack_init() { //用链表表示，第一个结点作为底部不保存
    return NULL;
}

//将一个给定元素入给定栈
void push(stack** s, ASTnode* newn) {
    stack* newc = malloc(sizeof(stack));
    newc->content = newn;
    newc->next = NULL;
    newc->pre = *s;
    *s = newc; //修改栈顶
}

//从一个给定栈返回栈顶元素
int pop(stack** s, ASTnode** p) {
    if (*s == NULL) return 0; //表示到达栈下溢
    if ((*s)->pre == NULL) { //栈中只有一个元素
        *p = (*s)->content; //将弹出的ASTnode放入容器的地址
        free(*s); //直接清楚，不用搞一堆有的没的
        *s = NULL; //栈直接清空了
        return 1;
    }
    *p = (*s)->content; //将弹出的ASTnode放入容器的地址
    *s = (*s)->pre; //表示栈顶指针往之前退一位
    free((*s)->next); //释放旧的栈顶锁链
    (*s)->next = NULL; //到这里完成新栈顶的设置
    return 1; //表示弹出成功
}

//取得一个栈的栈顶元素，这里返回的是ASTnode
ASTnode* gettop(stack* s) {
    if(s == NULL) return NULL;
    return s->content;
}

//取得一个栈的栈顶元素，但是只返回名字不返回结点
int gettopname(stack* s) {
    ASTnode* r = gettop(s);
    if(r) return r->name;
    return 0;
}

//展示栈
void stack_show(stack* s) {
    if(!s) {printf("栈为空\n"); return;}
    stack* p = s;
    printf("栈元素左进左出:");
    while(p) {
        printf("%s ", TYPE[p->content->name]);
        p = p->pre;
    }
    printf("\n");
}

//递归性质的全部释放
void stack_clear(stack* t) {
    if (t == NULL) return;
    while(t->pre) stack_clear(t->pre);
    while(t->next)stack_clear(t->next);
    free(t); //释放掉当前所在结点
}

//语法分析------------------------------------------------------
//所有程序在开始的时候都默认已经读入了开始的变量并要开始解析，所有的程序在返回前，也都要读取一个变量为之后的程序做准备
//语法单位<程序>的子程序
ASTnode* program() {
    ASTnode* root = AST_mknode(PROGRAM, NULL, 0);
    w = gettoken(); //程序的”第一动力“,这个token将会在外部定义中第一次被利用到
    if (root->child = ExtDefList()) {printf("词法分析正常结束\n"); return root;}//语法正确返回语法树根节点
    printf("词法分析出现错误，提前终止！\n");
    return NULL;//语法错误
}

//！！！语法单位<外部定义序列>的子程序
ASTnode* ExtDefList() {
    if(pe) return NULL;
    if (w == EOF) return NULL; //如果经过了之前的读取已经到达了文件尾
    ASTnode* root = AST_mknode(EXT_DEF_LIST, NULL, 0); //生成外部定义序列结点root
    AST_add_child(root, ExtDef());   //处理一个外部定义得到子树，作为root
    AST_add_child(root, ExtDefList()); //作为root的第二颗子树
    return root;
}

//！语法单位<外部定义>子程序————不需要提前读取token
ASTnode* ExtDef() { //处理外部定义序列，正确时返回子树根节点指针，否则返回NULL
    if(pe) return NULL;
    //printf("检测到外部定义\n");
    if (w != INT && w != FLOAT && w != CHAR) {printf("\t！！！第%d行：外部定义-类型错误\n", row); pe=1;return NULL;}
    token_name = w; //保存类型说明符
    w = gettoken();
    if (w != IDENT) {printf("\t！！！第%d行：外部定义-标识符错误\n", row);pe = 1; return NULL;}
    strcpy(token_text0, token_text); //保存第一个变量名或函数名到token_text0
    w = gettoken();
    ASTnode* p;
    if (w != LP) p = ExtVarDef(); //调用外部变量定义子程序，再读一个，刚好可以在循环开头就判断逗号分号
    else p = funcDef(); //调用函数定义子程序，这是因为读到左括号，读到这里就可以了，后面交给复合语句处理即可
    return p; //如果返回值p非空，表示成功完成一个外部定义的处理
}

//！!!语法单位<外部变量定义>子程序————已经读了一个不是LP的token，这肯定是int a ;的格式，所以不是
ASTnode* ExtVarDef() {
    if(pe) return NULL;
    printf("检测到外部变量定义\n");
    ASTnode* r = AST_mknode(EXT_VAR_DEF, NULL, 0); //生成外部变量定义结点
    AST_add_child(r, AST_mknode(token_name, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子
    ASTnode* p = AST_mknode(EXT_VAR_LIST, NULL, 0); //生成外部变量序列结点,
    AST_add_child(r, p); //p作为root第二个孩子
    AST_add_child(p, AST_mknode(IDENT, token_text0, STRING)); //由保存在token_text0的第一个变量名生成第一个变量名结点
    ASTnode* q = AST_mknode(EXT_VAR_LIST, NULL, 0);
    AST_add_child(p, q); p = q;
    while (1) {
        if (w != COMMA && w != SEMI) {printf("\t！！！第%d行：外部变量定义-格式错误\n", row); pe=1;return NULL;}//报错，释放root为根的全部结点，空指针返回
        if (w == SEMI) {w = gettoken(); return r;} //返回根节点
        w = gettoken(); //读取逗号后面的下一个标识符
        if (w != IDENT) {printf("\t！！！第%d行：外部变量定义-非标识符\n", row);pe=1; return NULL;}//报错，释放root为根的全部结点
        AST_add_child(p, AST_mknode(IDENT, token_text, STRING)); //根据token_text的变量名生成一个变量结点，作为p的第一个孩子
        q = AST_mknode(EXT_VAR_LIST, NULL, 0); //生成外部变量结点，根指针为q
        AST_add_child(p, q); //q,作为p的第二个孩子
        p = q;
        w = gettoken(); //期待得到一个逗号/分号
    }
}

//！！！语法单位<函数定义>子程序————读取了LP之后就没有再读取了
ASTnode* funcDef() {
    if(pe) return NULL;
    printf("检测到函数定义\n");
    ASTnode* root = AST_mknode(FUNC_DEF, token_text0, STRING); //在函数定义中保存有函数名
    AST_add_child(root, AST_mknode(token_name, NULL, 0)); //生成返回值类型结点，作为root第一个孩子
    AST_add_child(root, formalPara()); //调用形参子程序，生成第二棵子树
    if (w == SEMI) {AST_add_child(root, NULL); w = gettoken(); return root;} //为函数声明，返回当前的根结点即可，但是要记得去掉分号
    else if (w == LC) {AST_add_child(root, statementBlock()); return root;} //得到函数体子树，如果是函数声明（以分号结尾）就为空，生成第三棵子树
    else {printf("\t！！！第%d行：函数定义-格式错误", row); pe=1;free(root); return NULL;}
}

//！！！语法单位<形参>子程序————已经提前读取了LP，后面应该就是形参————最终还额外读取了一个token，接下来不用再读取了
ASTnode* formalPara() {
    if(pe) return NULL;
    printf("检测到形参\n");
    ASTnode* root = AST_mknode(FORMAL_PARA_LIST, NULL, 0); //生成形参定义结点
    ASTnode* p = root;
    w = gettoken(); //读取一个类型名
    if (w == RP) {w = gettoken(); return root;} //判断没有形参，会返回一个写着形参的结点
    while (1) {
        if (w != INT && w!= FLOAT && w != CHAR) {printf("\t！！！第%d行形参-类型错误\n", row); pe=1;return NULL;}//报错，释放root为根的全部结点，空指针返回
        AST_add_child(p, AST_mknode(w, NULL, 0));
        w = gettoken();
        if (w != IDENT) {printf("\t！！！第%d行形参-非标识符\n", row); pe=1;AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        AST_add_child(p, AST_mknode(IDENT, token_text, STRING));
        ASTnode* q = AST_mknode(FORMAL_PARA_LIST, NULL, 0);//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        AST_add_child(p, q);
        p = q;
        w = gettoken();
        if (w != COMMA && w != RP) {printf("\t！！！第%d行形参-格式错误\n", row); pe=1;AST_clear(root); return NULL;}
        if (w == RP) { //形参结束了
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken(); //如果为逗号就再读取下一个
    }
}

//语法单位<局部变量定义>子程序————已经提前读取了局部变量定义的变量类型
ASTnode* LocVarDef() {
    if(pe) return NULL;
    printf("检测到局部变量定义\n");
    ASTnode* r = AST_mknode(LOC_VAR_DEF, NULL, 0); //生成局部变量定义结点
    AST_add_child(r, AST_mknode(w, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子。局部变量定义在语句中调用，token还留在w中
    ASTnode* p = AST_mknode(LOC_VAR_LIST, NULL, 0); //生成局部变量序列结点,
    ASTnode* q = AST_mknode(LOC_VAR_LIST, NULL, 0);
    AST_add_child(r, p); //p作为root第二个孩子
    AST_add_child(p, q); //由保存在token_text0的第一个变量名生成第一个变量名结点
    w = gettoken();
    AST_add_child(p, AST_mknode(IDENT, token_text, STRING));
    w = gettoken(); //读取下一个token
    while(1) {
        if (w == ASSIGN) { //需要制作第三子树即表达式
            w = gettoken(); //为表达式读入token
            AST_add_child(p, expression()); //有两个终止符号，返回等于的表达式子树
        }//根据等号选择是否补充表达式语句之后
        if (w == SEMI) { //如果导致结尾的符号为分号，那么局部变量定义就已经结束
            w = gettoken(); //最终读出一个变量
            return r;
        }//要么直接得到逗号，要么
        if (w == COMMA) { //逗号说明序列继续
            p = q;
            q = AST_mknode(LOC_VAR_LIST, NULL, 0); //创建下一个结点
            AST_add_child(p, q);
            w = gettoken(); //期待是一个标识符
            AST_add_child(p, AST_mknode(IDENT, token_text, STRING));
            if (w != IDENT) {printf("\t！！！第%d行局部变量错误", row); pe=1;return NULL;}
            w = gettoken();
            continue;
        }
        printf("\t！！！第%d行局部变量错误\n", row);pe = 1; return NULL;
    }
}

//语法单位<复合语句>子程序————出现场景：已经读取了一个{，在得到}之后就结束程序
ASTnode* statementBlock() {
    if(pe) return NULL;
    printf("检测到复合语句\n");
    w = gettoken(); //层层调用，最终交给statement，这里要读取下一个token作为下面递归调用的“第一动力”
    ASTnode* root = AST_mknode(STATEMENT_BLOCK, NULL, 0);
    AST_add_child(root, statementList()); //调用处理语句序列子程序，返回子树根结点指针，作为root的第一个孩子
    printf("确认复合语句结束\n");
    return root; //返回复合语句子树根指针，不需要gettoken，因为上面语句列表结束之后已经读取了下一个token
}

//语法单位<语句序列>子程序————不需要提前读取token
//读取到不是语句的开头内容的时候可能是上面那样的}终止，直接返回已有的全部东西即可
ASTnode* statementList() {
    if(pe) return NULL;
    printf("检测到语句序列\n");
    ASTnode* root = AST_mknode(STATEMENT_LIST, NULL, 0);
    ASTnode* r1 = statement(); //调用处理一条语句的子程序，返回其子树根指针r1，每次调用完之后都会得到
    if (r1 == NULL) //error>0的时候还需要处理错误，防止因为语句序列结束了而报错
        return NULL;
    else {
        AST_add_child(root, r1);
        AST_add_child(root, statementList()); //递归地调用语句序列子程序后返回值
        return root;
    }
}

//语法单位<语句><复合语句>，处理很多结构中既可以是语句又可以是语句块的情况
ASTnode* statement_statementBlock() {
    if(pe) return NULL;
    ASTnode* r;
    if (w == LC) r = statementBlock(); //读到左大括号说明是复合语句
    else r = statement();
    return r; //经过上面调用都额外读取了一个token
}

//语法单位<语句>子程序————需要提前读取token
//注意消除最后一个分号
//statementList、statement函数会调用这条程序，如果遇到了终止的符号，读取到应该返回NULL
ASTnode* statement() {
    if(pe) return NULL;
    //调用此子程序时，第一个单词已经读取，根据第一个单词决定如何处理
    ASTnode *r, *r1, *r2, *r3, *r4;
    switch (w) {
        case IF : //分析条件语句
            printf("检测到if语句\n");
            w = gettoken();
            if (w != LP) {printf("\t！！！第%d行：IF语句条件错误\n", row);pe = 1; return NULL;}
            w = gettoken(); //为表达式处理做准备
            r1 = expression(); //调用处理表达式的子程序（结束符号为反小括号）正确时得到条件表达式子树指针
            if (w != RP) {printf("\t！！！第%d行：IF条件\n", row);pe = 1; return NULL;}
            w = gettoken(); //为分析语句还是复合语句做准备
            r2 = statement_statementBlock(); //调用处理一条语句的子程序，得到IF子句的子树指针
            if (w == ELSE) {
                printf("确认为IF_THEN_ELSE语句\n");
                w = gettoken(); //需要覆盖掉已经读掉的else
                r3 = statement_statementBlock(); //调用处理一条语句的子程序，得到IF子句的子树根指针
                r = AST_mknode(IF_ELSE, NULL, 0); //生成if-else结点
                AST_add_child(r, r1); //下挂条件结点
                AST_add_child(r, r2); //if子句结点
                AST_add_child(r, r3); //else子句结点
                return r; //上面调用了语句或复合语句，不需要再读
            }
            else { 
                printf("确认为IF_THEN语句\n");
                ASTnode* r = AST_mknode(IF, NULL, 0); //生成if-else结点
                AST_add_child(r, r1); //下挂条件结点
                AST_add_child(r, r2); //if子句结点
                return r; //遥远的上面调用了语句或复合语句，不需要再读
            }
        case LC : //{:调用复合语句子程序，返回得到的子树指针
            r = statementBlock(); //最后已经读了一个新的token，不需要再get
            return r;
        case IDENT : //表达式语句，这个ID是其一部分；表达式语句的结尾是确认为分号的，需要消除
            printf("检测到表达式语句\n");
            r = expression(); //这个ID读入作为第一个token //执行后w是导致终止的token
            if (w != SEMI) {printf("\t！！！第%d行表达式语句结束错误\n", row);pe = 1; return NULL;}
            w = gettoken(); return r; //最终多读出一个token
        case DO : //分析do-while语句（实验不要求）
            printf("检测到do-while语句\n");
            r1 = statement_statementBlock(); //用于保存do子句的指针，可能是语句或者语句块，已经返回了下一个token
            if (w != WHILE) {printf("\t！！！第%d行while错误\n", row);pe = 1; return NULL;} //如果不符合do-while的规范就要报错
            w = gettoken();
            if (w != LP) {printf("\t！！！第%d行while条件错误\n", row);pe = 1; return NULL;}
            r2 = expression(); //读取条件表达式
            if (w != RP) {printf("\t！！！第%d行while条件错误\n", row);pe = 1; return NULL;}
            r = AST_mknode(DO_WHILE, NULL, 0);
            AST_add_child(r, r1); //添加do子句
            AST_add_child(r, r2); //添加循环条件
            w = gettoken(); return r; //最终多读出一个token
        case WHILE : //分析while循环语句，
            printf("检测到while语句\n");
            w = gettoken();
            if (w != LP) {printf("\t！！！第%d行while条件错误\n", row); pe = 1;return NULL;}
            w = gettoken();//不可以读入小括号
            r1 = expression(); //调用处理表达式的子程序（结束符号为反小括号）正确时得到条件表达式子树指针
            if (w != RP) {printf("\t！！！第%d行while条件错误\n", row); pe = 1;return NULL;}
            w = gettoken(); //读入一个可能是{的token
            r2 = statement_statementBlock(); //用于保留循环体的子树指针，已经读取下一个token
            ASTnode* r = AST_mknode(WHILE, NULL, 0);
            AST_add_child(r, r1); //添加循环条件
            AST_add_child(r, r2); //添加while循环体
            w = gettoken(); return r; //最终多读出一个token
        case FOR : //分析for语句
            printf("检测到for语句\n");
            w = gettoken();
            if (w != LP) {printf("\t！！！第%d行：for循环没带括号", row); return NULL;}
            w = gettoken(); //准备下面语句的第一个token
            r1 = statement(); //调用外部变量定义语句子程序得到初始化条件子树指针，由于调用语句所以分号已经被消化掉了
            r2 = expression(); //调用条件子程序得到循环条件子树指针
            if (w != SEMI) {printf("\t！！！第%d行：for循环循环条件错误\n", row);pe = 1 ;return NULL;}
            w = gettoken();
            r3 = expression(); //调用语句子程度得到循环结束的变化部分
            if (w != RP) {printf("\t！！！第%d行：for循环结尾语句错误\n", row);pe = 1 ;return NULL;}
            w = gettoken(); //为下面的判断语句或复合语句做准备
            r4 = statement_statementBlock();
            r = AST_mknode(FOR, NULL, 0);
            AST_add_child(r, r1); //添加初始部分子树语句  
            AST_add_child(r, r2); //添加循环条件子树语句              
            AST_add_child(r, r3); //添加补充条件子树语句
            AST_add_child(r, r4); //添加循环部分子树语句
            w = gettoken(); return r;
        case BREAK : //break关键字单独成一条语句
            r = AST_mknode(BREAK, NULL, 0);
            w = gettoken();
            if (w != SEMI) {printf("\t！！！第%d行：break语句错误\n", row); pe = 1;return NULL;} //break语句结束之后需要检查后面一个token是否为分号
            w = gettoken(); return r;
        case CONTINUE : //continue关键字单独成一条语句
            r = AST_mknode(CONTINUE, NULL, 0);
            w = gettoken();
            if (w != SEMI) {printf("\t！！！第%d行：continue语句错误\n", row);pe = 1; return NULL;} //continue语句结束之后需要检查后面一个token是否为分号
            w = gettoken(); return r;
        case RETURN : //return
            r = AST_mknode(RETURN, NULL, 0);
            w = gettoken(); //为下面的表达式读入一个token
            if (w == SEMI) {w = gettoken(); return r;} //没有要返回的表达式
            AST_add_child(r, expression()); //将表达式语句作为return结点的第一棵也是唯一一棵子树
            if (w != SEMI) {printf("\t！！！第%d行：return语句错误\n", row);pe = 1; return NULL;} //continue语句结束之后需要检查后面一个token是否为分号
            w = gettoken(); return r;
        case INT : //int局部变量定义
        case CHAR : //char局部变量定义
        case FLOAT : //float局部变量定义
            return LocVarDef(); //处理局部变量定义说明
        case RC : //}:语句序列结束符号，如果语言支持switch语句，结束符号还有case和deafault
            printf("检测到复合语句结束\n");
            w = gettoken();
            return NULL;
        default : pe = 1; printf("\t！！！第%d行：语句无法解析\n", row); return NULL; //报错并返回NULL
    }
}

//语法单位<表达式>子程序————需要提前读取token，需要将导致终止的符号保存在token_name当中
//顺便把终止符号也读进去了，不需要判断终止符号
ASTnode* expression() {
    if(pe) return NULL;
    printf("检测到表达式\n");
    int lp = 0, rp = 0;
    stack* op = stack_init(); //定义运算符栈op并初始化，
    push(&op, AST_mknode(SS, NULL, 0)); //将启止符#入栈
    stack* opn = stack_init(); //定义操作数栈opn，元素是结点的指针
    int error = 0; //错误标记error设置为0
    while ((w != SS || gettopname(op) != SS) && !error) { //运算符栈顶不是起止符号，并没有错误时
        stack_show(op); stack_show(opn);
        if(w == IDENT) {
            token_name = w; //保存函数名
            strcpy(token_text0, token_text);
            w = gettoken();
            if(w == LP) { //这是一个函数调用
                push(&opn, funcCall()); //把函数调用看作一个算术表达式的操作数装入栈中
                continue;
            }
            else { //额外读入的w并不是左小括号，当前读入的是一个标识符
                push(&opn, AST_mknode(IDENT, token_text0, STRING)); //放入操作数
                printf("\t表达式标识符%s\n", (gettop(opn))->text);
                continue;
            }
        }
        else if (w == INT_CONST) { //如果是整数常量
            push(&opn, AST_mknode(INT_CONST, &token_int, INT)); 
            w = gettoken(); continue;
        }
        else if (w == FLOAT_CONST) { //如果是浮点数常量
            push(&opn, AST_mknode(FLOAT_CONST, &token_float, FLOAT));
            w = gettoken(); continue;
        }
        else if (w == CHAR_CONST) { //如果是字符常量
            push(&opn, AST_mknode(CHAR_CONST, &token_char, CHAR));
            w = gettoken(); continue;
        }
        else if(lp <= rp && w == RP) { //左右括号数匹配时出现了一个右括号，说明能读取到的表达式已经结束了，是否正确交给别的程序，到此为止
            token_name = w; w = SS; continue;//输入起止符
        }
        else if (getRank(w) > 0 && w != COMMA) { //运算符有一个不等于0的优先级，逗号交给函数调用的程序单独处理，算术表达式中不应该考虑逗号
            if ((gettopname(op)==LP && w==SS) || (gettopname(op)==SS && w==RP)) {error = 1; break;}
            else if (gettopname(op)==LP && w==RP) { //当右括号消栈遇到左括号
                ASTnode *t;
                if(!(pop(&op, &t))) {error = 1; break;}
                w = gettoken(); continue;
            }
            else if ((getRank(gettopname(op)) > getRank(w)) || (gettopname(op)==LP && w!=RP)) { //如'+' > '*'，需压栈；小括号是例外
                if(gettopname(op)==LP && w!=RP) lp++; //对左括号数量进行计数
                push(&op, AST_mknode(w, NULL, 0));
                w = gettoken(); continue;
            }
            else if ((getRank(gettopname(op)) <= getRank(w)) || (gettopname(op)!=LP && w==RP)) { //如'*' < '+'，可以从左到右运行
                if(gettopname(op)!=LP && w==RP) rp++; //对右括号数量进行计数
                ASTnode *t, *t1, *t2;
                if(!(pop(&op, &t)))   {error = 1; break;}
                if(!(pop(&opn, &t2))) {error = 1; break;}
                if(!(pop(&opn, &t1))) {error = 1; break;}
                AST_add_child(t, t1); AST_add_child(t, t2);
                push(&opn, t); //不需要再读入任何token，可能就这样一口气下降到底
                continue;
            }
            else; //该情况不可能存在
        }
        else { //无法识别，说明已经到了终止的时候了，把起止符入栈，而且不用再读入新的字符
            token_name = w; w = SS; //保存名字
        }
    } //循环终止要么是因为报错，要么是因为运算符栈只剩下起止符了
    if(!error) {
        ASTnode* p; pop(&opn, &p); //弹出运算符栈中的唯一那个
        w = token_name; //将结尾起止符改回原来的token名称用于下面的分析
        printf("检测到表达式结束\n");
        return p;
    }
    else {
        /*整体上的报错*/
        return NULL;
    }
}

//语法单位<函数调用>子程序-------已经读取了LP到w，尚未进一步读取
ASTnode* funcCall() { //函数调用
    if(pe) return NULL;
    printf("检测到函数调用\n");
    ASTnode* r = AST_mknode(token_name, NULL, 0); //函数调用结点
    ASTnode* p = AST_mknode(ACTUAL_PARA_LIST, NULL, 0); //函数调用结点
    ASTnode* q;
    while(1) {
        w = gettoken(); //为表达式读入一个token
        if (w == RP) { //实参列表已经结束了
            return p;
        } //否则就还没有结束，继续读表达式
        q = expression(); //得到表达式，结束之后得到结尾的符号
        AST_add_child(p, q);
        AST_add_child(p, AST_mknode(ACTUAL_PARA_LIST, NULL, 0));
        if(w == RP) { //形参结束了!
            w = gettoken(); return p; //需要读下一个token
        }
        if(w == COMMA) { //形参还有，这个循环还会继续
            p = q; continue; //下次循环
        }
        //会跑到这里肯定是w有问题，需要报错
        printf("\t！！！第%d行：函数调用错误\n", row);pe=1; return NULL;
    }
}

//语法树显示---------------------------------------------------
//r表示需要打印的结点，t表示开头的tab数量，有格式需要，打印方式按照结点的语法规则
void AST_show(ASTnode* r, int t) { //递归输出解析出的语法树
    if (!r) return;
    switch (r->name) {
        case PROGRAM : //程序
            AST_show(r->child, t);
            break;
        case EXT_DEF_LIST : //外部定义序列
            AST_show(r->child, t);
            AST_show(r->child->brother, t);
            break;
        case EXT_VAR_DEF : //外部变量定义
            printf("外部变量定义:\n");
            printf("\t类型: %s\n", TYPE[r->child->name]);
            AST_show(r->child->brother, t+1);
            break;
        case EXT_VAR_LIST : //外部变量序列
            if(!r->child) break;
            printf("\t\tIDENT: %s\n", r->child->text);
            AST_show(r->child->brother, t);
            break;
        case FUNC_DEF : //函数定义
            if(!r->child->brother->brother) printf("\n函数声明:\n");
            else printf("\n函数定义:\n");
            printf("\t类型: %s\n", TYPE[r->child->name]);
            printf("\t函数名: %s\n", r->text);
            printf("\t函数形参:\n");
            AST_show(r->child->brother, t); //开始打印形参序列
            AST_show(r->child->brother->brother, t); //打印函数体
            break;
        case FORMAL_PARA_LIST : //形式参数
            if(!r->child) break;
            printf("\t\t类型名: %s, 参数名: %s\n", TYPE[r->child->name], r->child->brother->text);
            AST_show(r->child->brother->brother, t); //打印下一个形参序列
            break;
        case STATEMENT_BLOCK : //符合语句
            printf("\t复合语句：\n");
            AST_show(r->child, t); //打印语句序列函数
            break;
        case STATEMENT_LIST : //语句序列
            if(!r->child) return;
            AST_show(r->child, t); //打印这条语句
            AST_show(r->child->brother, t); //打印下一条语句序列
            break;
        case LOC_VAR_DEF : //局部变量定义语句
            printf("\t\t局部变量定义:\n");
            printf("\t\t\t类型: %s\n", TYPE[r->child->name]);
            AST_show(r->child->brother, t); //开始打印局部变量语句
            break;
        case LOC_VAR_LIST : //局部变量序列语句
            if(!r->child) return;
            printf("\t\t\t\tIDENT: %s", r->child->brother->text);
            if (r->child->brother->brother) {
                printf(" = ");
                //AST_expression_show(r->child->brother->brother);
                printf("\n");
            }
            AST_show(r->child, t); //从第一个结点向下递归
            break;
        case IF : //没有else的if语句
            printf("\t\t条件语句(if_then)\n");
            printf("\t\t\t条件:\n");
            //AST_show(r->child, 4); //打印条件表达式
            printf("\t\t\tif子句:\n");
            AST_show(r->child->brother, t);
        case IF_ELSE : //有else的if语句
            printf("\t\t条件语句(if_then)\n");
            printf("\t\t\t条件:\n");
            //AST_show(r->child, 4); //打印条件表达式
            printf("\t\t\tif子句:\n");
            AST_show(r->child->brother, t);
            printf("\t\t\telse子句:\n");
            AST_show(r->child->brother->brother, t);
            break;
        case WHILE : //while循环语句
            printf("\t\twhile语句:\n");
            printf("\t\t\t条件:\n");
            //AST_show(r->child, 4); //打印条件表达式
            printf("\t\t\t循环体:\n");
            AST_show(r->child->brother, t); //打印循环体
            break;
        case DO_WHILE : //do……while……循环语句
            printf("\t\tdo_while语句:\n");
            printf("\t\t\t循环体:\n");
            //AST_show(r->child, t); //打印循环体
            printf("\t\t\t条件:\n");
            //AST_show(r->child, t); //打印条件表达式
            break;
        case FOR : //for循环语句
            printf("\t\tfor语句:\n");
            printf("\t\t\t初始化子句:\n");
            AST_show(r->child, t); //打印语句
            printf("\t\t\t循环条件:\n");
            AST_show(r->child->brother, t); //打印循环体
            printf("\t\t\t结尾表达式:\n");
            //AST_show(r->child->brother->brother, t); //打印表达式
            printf("\t\t\t循环体:\n");
            AST_show(r->child->brother->brother->brother, t); //打印循环体
        case BREAK : //break子句
            printf("\t\tbreak语句\n");
            break;
        case CONTINUE : //continue子句
            printf("\t\tcontinue语句\n");
            break;
        case RETURN : //return子句
            printf("\t\t返回语句:\n\t\t\t");
            AST_expression_show(r->child);
            printf("\n");
            break;
        case ACTUAL_PARA_LIST : //实际参数序列的打印
            if(!r->child) return;
            AST_expression_show(r->child);
            if(r->child->brother->child) printf(",");
            AST_show(r->child->brother,t);
            break;
        default : //应该没有这种奇怪的东西
            return;
    }
}

//打印表达式---------------------------------------------------
void AST_expression_show(ASTnode* r) { //打印表达式
    if (!r) return;
    if (r->name == IDENT && r->child && r->child->brother) { //如果是函数调用标识符，就不需要优先打印左子树
        printf("%s(", r->text); //打印函数调用和左括号
        AST_show(r->child, 0); //按照函数实参的逗号打印实参序列
        printf(")"); //打印右括号
    }
    if (r->child) { //优先打印括号和左子树
        AST_expression_show(r->child);
    }
    if (getRank(r->name)>0) printf("%s\n", r->text); //打印其他运算符，包括函数调用中的括号
    else if (r->name == IDENT) printf("<标识符%s>", r->text);
    else if (r->name == INT_CONST) printf("<整数常量%d>", r->text);
    else if (r->name == FLOAT_CONST) printf("<浮点数常量%f>", r->text);
    else if (r->name == CHAR_CONST) printf("<字符常量%c>", r->text);
    else;
    if (r->child->brother) { //其次打印括号和右子树
        AST_expression_show(r->child->brother);
    }
}

//美化格式-----------------------------------------------------
void AST_modify(ASTnode* r) { //调整解析出的语法树
    return;
}

void main() {
    fp = fopen("test.c", "r");
    //ASTnode* h = prepare(); //预处理得到头文件引用，并滚出所有注释
    ASTnode* r = program(); //词法分析并进行语法分析得到AST
    if(pe) return;
    printf("\n下面开始打印解析出的语法树：\n");
    AST_show(r, 0); //展示语法树，最初的tab为0
    //AST_modify(r); //美化格式，输出到特定文件
    fclose(fp);
}