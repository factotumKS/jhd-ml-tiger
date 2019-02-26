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
#define IDLEN 32
#define w_is_operand w==ID||w==INT_CONST||w==FLOAT_CONST||w==CHAR_CONST
#define w_is_operator w>=ADD&&w<=ASSIGN

enum token_kind { //词法分析返回的token种类
    //词法分析得到的token
    INT, FLOAT, CHAR, IF, ELSE, DO, WHILE, FOR, CONTINUE, BREAK, //10关键字
    ERROR_TOKEN, ID, INT_CONST, FLOAT_CONST, 
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE, EQ, NEQ, ASSIGN, AND, OR,
    LP, RP, LC, RC, COMMA, SEMI, RETURN,
    STRING,
    //语法分析中需要使用的内部结点标识
    PROGRAM,            //程序
    EXT_DEF_LIST,       //外部定义序列
    EXT_VAR_DEF,        //外部变量定义
    EXT_VAR_LIST,       //外部变量序列
    FUNC_DEF,           //函数定义
    FORMAL_PARA_LIST,   //形参列表
    STATEMENT_BLOCK,    //复合语句
    STATEMENT_LIST,     //语句序列
    LOC_VAR_DEF,        //局部变量定义
    LOC_VAR_LIST,       //局部变量序列
    IF_ELSE, DO_WHILE,  //特殊结点
    //栈结构中需要的起止符
    SS
};
char* TYPE[] = {
    "int", "float", "char", "if", "else", "do", "while", "for", "continue", "break",
    "error_token", "id", "int_const", "float_const",
    "+", "-", "*", "/", "%", ">", "<", ">=", "<=", "==", "!=", "=", "&&", "||",
    "(", ")", "{", "}", ",", ";", "return",
};
typedef struct node{
    int name = 0;   //表示名字
    void* text = NULL;  //表示内容
    struct node* brother = NULL;      //弟结点
    struct node* child = NULL;        //子结点
} ASTnode;
typedef struct chain{
    ASTnode* content;
    struct chain* pre;
    struct chain* next;
} stack;
char precede[][] ={
    //每行表示当前栈顶的运算符，每列表示当前读到的运算符，
    //<会导致当前读到的运算符压栈，
    //>会导致执行栈顶的运算符号，
    //=用于处理掉多余的右括号
    //+    -    *    /    (    )    =  >或< 等不等 ||    &&   ,   起止符
    {'>', '>', '<', '<', '<', '>', ' ', '>', '>', '>', '>', '>', '>', ' '}, //0 : +
    {'>', '>', '<', '<', '<', '>', ' ', '>', '>', '>', '>', '>', '>', ' '}, //1 : -
    {'>', '>', '>', '>', '<', '>', ' ', '>', '>', '>', '>', '>', '>', ' '}, //2 : *
    {'>', '>', '>', '>', '<', '>', ' ', '>', '>', '>', '>', '>', '>', ' '}, //3 : /
    {'<', '<', '<', '<', '<', '=', ' ', '>', '>', '>', '>', '<', '>', ' '}, //4 : ( 
    {'>', '>', '>', '>', '>', ' ', ' ', '>', '>', '>', '>', '>', '>', ' '}, //5 : )
    {'<', '<', '<', '<', '<', ' ', '<', '<', '<', '<', '<', '<', '>', ' '}, //6 : =
    {'<', '<', '<', '<', '<', '>', ' ', '>', '>', '>', '>', '>', '>', ' '}, //7 : >或<还有它们的>=或<=
    {'<', '<', '<', '<', '<', '>', ' ', '<', '>', '>', '>', '>', '>', ' '}, //8 : 等不等
    {'<', '<', '<', '<', '<', '>', ' ', '<', '<', '>', '>', '>', '>', ' '}, //9 : ||
    {'<', '<', '<', '<', '<', '>', ' ', '<', '<', '>', '>', '>', '>', ' '}, //10: &&
    {'<', '<', '<', '<', '<', '>', ' ', '<', '<', '<', '<', '>', '>', ' '}, //11: , 只用于实参列表
    {'<', '<', '<', '<', '<', ' ', '<', '<', '<', '<', '<', '<', '=', ' '}, //12: 起止符
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, //13: 最后一行/列留给错误的符号，防止以溢出的形式报错
};
int w2precede(int tk) { //将操作符转化为行列号
    int num;
    switch (tk) {
        case ADD : return 0;
        case SUB : return 1;
        case MUL : return 2;
        case DIV : return 3;
        case LP  : return 4;
        case RP  : return 5;
        case ASSIGN: return 6;
        case GT  : return 7;
        case LT  : return 7;
        case GE  : return 7;
        case LE  : return 7;
        case EQ  : return 8;
        case NEQ : return 8;
        case OR  : return 9;
        case AND : return 10;
        case COMMA:return 11;
        case SS  : return 12;
        default  : return 13;
    }
}

//初始化一堆用于保存字符串和数字常量的空间
FILE* fp; //源文件指针
int w; //表示刚返回得到的类型
int row, col; //用于报错的行列号
//临时保存用，生成ASTnode时会动态分配空间并将内容转移
char token_text[IDLEN]; //gettoken之后，上一个标识符的text储存在这里
char token_text0[IDLEN]; //处理定义语句时需要保留某些标识符的text，所以保存在这里
int token_name; //处理定义语句时需要保存某些非标识符token的值，表达式结尾时也需要保存结束符号到底是谁？！
int token_int; //gettoken之后，上一个（正）整数常量的大小储存在这里
float token_float; //gettoken之后，上一个浮点数常量的大小储存在这里
char token_char; //gettoken之后哦，上一个字符常量的大小储存在这里
int error; //记录错误的发生次数

//预处理器-------------------------------------------------------
void prepare() {
    //去除注释，尚未实现
    return;
}

//语法分析，每次调用得到一个token，返回名字并将附带text保存在token_text、token_int等全局变量中-
int gettoken() {
    char c;
    for(int i = 0; i < IDLEN; i++) token_text[i] = 0;
    
    //处理全部空白符
    while ((c = fgetc(fp)) && (c == ' '|| c == '\t'|| c == '\n')){
        if (c == '\n') {row += 1; col = 0;}
        else col += 1;
    }

    //处理注释

    //处理标识符 关键字
    if ((c>='a' && c<= 'z') || (c>='A' && c<= 'Z')) {
        int i = 0;
        do {token_text[i] = c; i++; col += 1;}
        while ((c = fgetc(fp)) && ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c <='9')))
        ungetc(c, fp);
        for (int i = 0; i < 10; i++) {  //判断是否为关键字
            if (!strcmp(token_text, TYPE[i])) return i;
        }
        return IDENT; //返回标识符
    }

    //处理整数int，浮点数float
    if (c >= '0' && c <= '9') {
        int val = token - '0';
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
                return FLOAT_CONST;
            }
            token_int = val; return INT_CONST;
        } 
        else if ((c = fgetc(fp)) && (c == 'x' || *src == 'X')) { //十六进制数
            while ((c = fgetc(fp)) && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                val = val * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
            }
            token_int = val; return INT_CONST;
        }
        else { //八进制数，或者0
            while ((c = fgetc(fp)) && c >= '0' && c <= '7') {
                val = val * 8 + c - '0';
            }
            token_int = val; return INT_CONST;
        }
    }
 
    //处理字符char
    if (c = '\'') { //字符常量
        token_char = fgetc(c);
        c = fgetc(fp); //消化掉后面的引号
        return CHAR_CONST;
    }

    //处理各种符号
    switch (c) {
        case '=' : c = fgetc(fp); if (c == '=') {col += 2; return EQ; } else {ungetc(c, fp); col += 1; return ASSIGN;}
        case '+' : c = fgetc(fp); if (c == '+') {col += 2; return INC;} else {ungetc(c, fp); col += 1; return ADD;} 
        case '-' : c = fgetc(fp); if (c == '-') {col += 2; return DEC;} else {ungetc(c, fp); col += 1; return SUB;} 
        case '!' : c = fgetc(fp); if (c == '=') {col += 2; return NEQ;} else {ungetc(c, fp); col += 1; return ERROR_TOKEN;} 
        case '<' : c = fgetc(fp); if (c == '=') {col += 2; return LE; } else {ungetc(c, fp); col += 1; return LT;} 
        case '>' : c = fgetc(fp); if (c == '=') {col += 2; return GE; } else {ungetc(c, fp); col += 1; return GT;}   
        case '|' : c = fgetc(fp); if (c == '|') {col += 2; return OR; } else {ungetc(c, fp); col += 1; return ERROR_TOKEN;}
        case '&' : c = fgetc(fp); if (c == '&') {col += 2; return AND;} else {ungetc(c, fp); col += 1; return ERROR_TOKEN;} 
        case '%' : col += 1; return MOD;
        case '(' : col += 1; return LP;
        case ')' : col += 1; return RP;
        case '{' : col += 1; return LC;
        case '}' : col += 1; return RC;
        case '*' : col += 1; return MUL;
        case '/' : col += 1; return DIV;
        case ',' : col += 1; return COMMA;
        case ';' : col += 1; return SEMI;
        default : if (feof(fp)) return EOF;
                else return ERROR_TOKEN;
    }
}

//数据结构：AST-----------------------------------------------------
//制作新结点    形参分别是结点类型，结点名，结点附带text位置，结点附带text类型
ASTnode* AST_mknode(int gname, void* gtext, int gdata) {
    ASTnode* root = malloc(sizeof(ASTnode));
    root->name = gname;
    root->text = AST_mktext(gtext, gdata);
    return root;
}

//制作新text位置，动态分配空间并将临时得到的空间转移过来
void* AST_mktext(void* gtext, int gdata) {
    if (gdata == 0) return NULL;
    else if (gdata == STRING) { //复制字符串 ID
        char* new = malloc(sizeof(strlen(gtext)) + 1);
        strcpy(new, gtext);
        return new;
    }
    else if (gdata == INT) { //复制整数
        int* new = malloc(sizeof(int));
        *new = *gtext;
        return new
    }
    else if (gdata == FLOAT) { //复制浮点数
        float* new = malloc(sizeof(float));
        *new = *gtext;
        return new
    }
    else { //复制字符串
        char* new = malloc(sizeof(char));
        *new = *gtext;
        return new
    }
}

//增加已有结点作为孩子
void AST_add_child(ASTnode* father, ASTnode* new_child) {
    ASTnode* node = father->child;
    while(node->brother) node = node->brother;
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
stack* Astack_init() { //用链表表示，第一个结点作为底部不保存
    return NULL;
}

//将一个给定元素入给定栈
void push(stack** s, ASTnode* i) {
    stack* newc = malloc(sizeof(chain));
    ASTnode* newn = malloc(sizeof(ASTnode));
    newc->content = newn;
    newc->next = NULL;
    newc->pre = *s;
    *s = newc//修改栈顶
}

//从一个给定栈返回栈顶元素
int pop(stack** s, ASTnode** p) {
    if (*s == NULL) return 0; //表示到达栈下溢
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

//递归性质的全部释放
void stack_clear(stack* t) {
    if (t == NULL) return;
    while(t->pre) Stack_clear(t->pre);
    while(t->next)Stack_clear(t->next);
    free(t); //释放掉当前所在结点
}

//语法分析------------------------------------------------------
//所有程序在开始的时候都默认已经读入了开始的变量并要开始解析，所有的程序在返回前，也都要读取一个变量为之后的程序做准备
//语法单位<程序>的子程序
ASTnode* program() {
    ASTnode* root = AST_mknode(PROGRAM, NULL, 0);
    w = gettoken();
    if (root->child = ExtDefList())  return root;//语法正确返回语法树根节点
    else //语法错误
}

//！！！语法单位<外部定义序列>的子程序
ASTnode* ExtDefList() {
    if (w == EOF) return NULL;
    root = AST_mknode(EXT_DEF_LIST, NULL, 0); //生成外部定义序列结点root
    AST_add_child(root, ExtDef());   //处理一个外部定义得到子树，作为root
    AST_add_child(root, ExtDefList()); //作为root的第二颗子树
    return root;
}

//！语法单位<外部定义>子程序————不需要提前读取token
ASTnode* ExtDef() { //处理外部定义序列，正确时返回子树根节点指针，否则返回NULL
    if (w != INT && w != FLOAT && w != CHAR) {printerror(row, col, "外部定义-类型错误\n"); return NULL;}
    token_name = w; //保存类型说明符
    w = gettoken();
    if (w != IDENT) {printerror(row, col, "外部定义-标识符错误\n"); return NULL;}
    strcpy(token_text0, token_text); //保存第一个变量名或函数名到token_text0
    w = gettoken();
    if (w != LP) p = ExtVarDef(); //调用外部变量定义子程序
    else p = FuncDef(); //调用函数定义子程序
    //如果返回值p非空，表示成功完成一个外部定义的处理
}

//！!!语法单位<外部变量定义>子程序————已经读了一个不是LP的token，这肯定是int a ;的格式，所以不是
ASTnode* ExtVarDef() {
    ASTnode* r = AST_mknode(EXT_VAR_DEF, NULL, 0); //生成外部变量定义结点
    AST_add_child(r, AST_mknode(token_name, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子
    ASTnode* p = AST_mknode(EXT_VAR, NULL, 0); //生成外部变量结点,
    AST_add_child(r, p); //p作为root第二个孩子
    AST_add_child(p, AST_mknode(IDENT, token_text0, STRING)); //由保存在token_text0的第一个变量名生成第一个变量名结点
    while (1) {
        if (w != ',' || w != ';') {printerror(row, col, "外部变量定义-格式错误\n"); AST_clear(root); return NULL}//报错，释放root为根的全部结点，空指针返回
        if (w == ';') {
            w = gettoken();
            return r; //返回根节点
        }
        w = gettoken();
        if (w != IDENT) {printerror(row, col, "外部变量定义-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        ASTnode* q = AST_add_child(p, AST_mknode(EXT_VAR, NULL, 0)); //生成外部变量结点，根指针为q,作为p的第二个孩子
        p = q;
        AST_add_child(p, AST_mknode(IDENT, token_text0, STRING)); //根据token_text的变量名生成一个变量结点，作为p的第一个孩子
        w = gettoken(); //期待得到一个逗号/分号
    }
}

//！！！语法单位<函数定义>子程序————不需要提前读取token
ASTnode* funcDef() {
    ASTnode* root = AST_mknode(FUNC_DEF, token_text0, STRING); //在函数定义中保存有函数名
    AST_add_child(root, AST_mknode(token_name, NULL, 0); //生成返回值类型结点，作为root第一个孩子
    AST_add_child(root, formalPara()); //调用形参子程序，生成第二棵子树
    if (w == ';') {AST_add_child(root, NULL); return root} //为函数声明，返回当前的根结点即可
    else if (w == '{') {AST_add_child(root, statementOrstaementBlock()); return root;} //得到函数体子树，如果是函数声明（以分号结尾）就为空，生成第三棵子树
    else {error(row, col, "函数定义-格式错误"); free(root); return NULL}
}

//！！！语法单位<形参>子程序————已经提前读取了LP，后面应该就是形参————最终还额外读取了一个token，接下来不用再读取了
ASTnode* formalPara() {
    ASTnode* root = AST_mknode(FORMAL_PARA_LIST, NULL, 0); //生成形参定义结点
    ASTnode* p = root;
    w = gettoken(); //读取一个类型名
    if (w == ')') {w = gettoken(); retrun root;} //判断没有形参
    while (1) {
        if (w != INT && w!= FLOAT && w != CHAR) {printerror(row, col, "形参-类型错误\n"); return NULL}//报错，释放root为根的全部结点，空指针返回
        AST_add_child(p, AST_mknode(w, NULL, 0));
        w = gettoken();
        if (w != IDENT) {printerror(row, col, "形参-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        AST_add_child(p, AST_mknode(IDENT, token_text, STRING));
        ASTnode* q = AST_add_child(p, AST_mknode(FORMAL_PARA_LIST, NULL, 0));//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        p = q;
        w = gettoken();
        if (w != ',' && w != ')') {printerror(row, col, "形参-格式错误\n"; AST_clear(root); return NULL;)}
        if (w == ')') { //形参结束了
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken(); //如果为逗号就再读取下一个
    }
}

//语法单位<局部变量定义>子程序————不需要提前读取token
ASTnode* LocVarDef() {
    ASTnode* r = AST_mknode(LOC_VAR_DEF, NULL, 0); //生成局部变量定义结点
    AST_add_child(r, AST_mknode(w, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子。局部变量定义在语句中调用，token还留在w中
    ASTnode* p = AST_mknode(LOC_VAR_LIST, NULL, 0); //生成局部变量序列结点,
    AST_add_child(r, p); //p作为root第二个孩子
    AST_add_child(p, AST_mknode(IDENT, token_text0, STRING)); //由保存在token_text0的第一个变量名生成第一个变量名结点
    w = gettoken();
    while(1) {
        ASTnode* q = AST_mknode(LOC_VAR_LIST, NULL, 0); //创建下一个结点
        AST_add_child(p, q); //作为局部变量结点p的第一棵子树
        if (w != IDENT) { return NULL;} //记得报错
        AST_add_child(p, AST_mknode(IDENT, token_text, STRING); //标识符作为局部变量结点v的第二棵子树
        w = gettoken();
        if (w == EQ) { //需要制作
            AST_add_child(p, expression(SEMI, COMMA)); //有两个终止符号，返回等于的表达式子树
            if (token_name == SEMI) {
                w = gettoken();
                retrun r;
            }
        }
        p = q; //更新
        w = gettoken();//下一个token应该是标识符，否则在上面报错
    }
}

//语法单位<复合语句>子程序————不需要提前读取
//已经读取了一个{，在得到}之后就结束程序
ASTnode* statementBlock() {
    ASTnode* root = AST_mknode(STATEMENT_BLOCK, NULL, 0);
    AST_add_child(root, statementList()); //调用处理语句序列子程序，返回子树根结点指针，作为root的第一个孩子
    //上面结束了这里应该是一个反大括号，可能需要gettoken()
    if (w != '}') {
        //返回空指针，报错并释放结点
        printerror(row, col, "复合语句-不正常结束\n");
        return NULL;
    }
    w = gettoken();
    return root; //返回复合语句子树根指针
}

//语法单位<语句序列>子程序————不需要提前读取token
//读取到不是语句的开头内容的时候可能是上面那样的}终止，直接返回已有的全部东西即可
ASTnode* statementList() {
    ASTnode* root = AST_mknode(STATEMENT_LIST, NULL, 0);
    ASTnode* r1 = statement(); //调用处理一条语句的子程序，返回其子树根指针r1，每次调用完之后都会得到
    if (r1 == NULL) //erros>0的时候还需要处理错误，防止因为语句序列结束了而报错
        return NULL;
    else {
        AST_add_child(root, r1);
        AST_add_child(root, statementList()); //递归地调用语句序列子程序后返回值
        return root;
    }
}

//语法单位<语句><复合语句>，处理很多结构中既可以是语句又可以是语句块的情况
ASTnode* statement_staementBlock() {
    if (w == RC) return statementBlock(); //读到大括号说明是复合语句
    return r = statement();
}

//语法单位<语句>子程序————需要提前读取token
//注意消除最后一个分号
//statementList、statement函数会调用这条程序，如果遇到了终止的符号，读取到应该返回NULL
ASTnode* statement() {
    //调用此子程序时，第一个单词已经读取，根据第一个单词决定如何处理
    switch (w) {
        case IF : //分析条件语句
            w = gettoken();
            if (w != '(') {printerror(); return NULL}
            w = gettoken();
            ASTnode* r1 = expression(RP, 0); //调用处理表达式的子程序（结束符号为反小括号）正确时得到条件表达式子树指针
            ASTnode* r2 = statement_staementBlock(); //调用处理一条语句的子程序，得到IF子句的子树指针
            if (w == ELSE) {
                ASTnode* r3 = statement_staementBlock(); //调用处理一条语句的子程序，得到IF子句的子树根指针
                ASTnode* r = AST_mknode(IF_ELSE, NULL, 0); //生成if-else结点
                AST_add_child(r, r1); //下挂条件结点
                AST_add_child(r, r2); //if子句结点
                AST_add_child(r, r3); //else子句结点
                w = gettoken(); return r;
            }
            else { 
                ASTnode* r = AST_mknode(IF, NULL, 0); //生成if-else结点
                AST_add_child(r, r1); //下挂条件结点
                AST_add_child(r, r2); //if子句结点
                w = gettoken(); return r;
            }
        case LC : //{:调用复合语句子程序，返回得到的子树指针
            AST* r = statementBlock(); //最后已经读了一个新的token，不需要再get
            return r;
        case IDENT : //表达式语句，这个ID是其一部分
            return expression(SEMI, 0); //这个ID读入作为第一个token
        case DO : //分析do-while语句（实验不要求）
            ASTnode* r1 = statement_staementBlock(); //用于保存do子句的指针，可能是语句或者语句块
            w = gettoken();
            if (w != WHILE) {printerror(); return NULL;} //如果不符合do-while的规范就要报错
            w = gettoken();
            if (w != LP) {printerror(); return NULL}
            ASTnode* r2 = expression(RP, 0);
            //读完表达式需不需要继续判断？？？
            w = gettoken();
            if (w != RP) {/*报错*/}
            ASTnode* r = AST_mknode(DO_WHILE, NULL, 0);
            AST_add_child(r, r1); //添加do子句
            AST_add_child(r, r2); //添加循环条件
            w = gettoken(); return r;
        case WHILE : //分析while循环语句，
            w = gettoken();
            if (w != LP) {printerror(); return NULL}
            ASTnode* r1 = expression(RP, 0); //调用处理表达式的子程序（结束符号为反小括号）正确时得到条件表达式子树指针
            ASTnode* r2 = statement_staementBlock(); //用于保留循环体的子树指针
            w = gettoken();
            if (w != RP) {/*报错返回空结点*/}
            ASTnode* r = AST_mknode(WHILE, NULL, 0);
            AST_add_child(r, r1); //添加循环条件
            AST_add_child(r, r2); //添加while循环体
            w = gettoken(); return r;
        case FOR : //分析for语句
            w = gettoken();
            if (w != LP) {printerror(); return NULL}
            ASTnode* r1 = statement(); //调用外部变量定义语句子程序得到初始化条件子树指针
            ASTnode* r2 = expression(SEMI, 0); //调用条件子程序得到循环条件子树指针
            ASTnode* r3 = expression(RP, 0); //调用语句子程度得到循环结束的变化部分
            ASTnode* r4 = statement_staementBlock();
            ASTnode* r = AST_mknode(FOR, NULL, 0);
            AST_add_child(r, r1); //添加初始部分子树语句  
            AST_add_child(r, r2); //添加循环条件子树语句              
            AST_add_child(r, r3); //添加补充条件子树语句
            AST_add_child(r, r4); //添加循环部分子树语句
            w = gettoken(); return r;
        case BREAK : //break关键字单独成一条语句
            ASTnode* r = AST_mknode(BREAK, NULL, 0);
            w = gettoken();
            if (w != SEMI) {printerror(); return NULL} //break语句结束之后需要检查后面一个token是否为分号
            w = gettoken(); return r;
        case CONTINUE : //continue关键字单独成一条语句
            ASTnode* r = AST_mknode(CONTINUE, NULL, 0);
            w = gettoken();
            if (w != SEMI) {printerror(); return NULL} //continue语句结束之后需要检查后面一个token是否为分号
            w = gettoken(); return r;
        case RETURN : //return
            ASTnode* r = AST_mknode(RETURN, NULL, 0);
            AST_add_child(r, expression(SEMI, 0)); //将表达式语句作为return结点的第一棵也是唯一一棵子树
        //尚未解决
        case INT : //int局部变量定义
        case CHAR : //char局部变量定义
        case FLOAT : //float局部变量定义
            return LocVarDef(); //处理局部变量定义说明
        case RC : //}:语句序列结束符号，如果语言支持switch语句，结束符号还有case和deafault
            w = gettoken();
            return NULL;
        default errors += 1; printerror(row, col, ""); return NULL; //报错并返回NULL
    }
}

//语法单位<表达式>子程序————需要提前读取token，需要将导致终止的符号保存在token_name当中
//顺便把终止符号也读进去了，不需要判断终止符号
ASTnode* expression(int end1, int end2) { //传入结束符号，可以是反小括号或者分号
    //已经读入了一个单词在w中
    stack* op = Stack_init(); //定义运算符栈op并初始化，
    push(op, AST_mknode(SS, NULL, 0)); //将启止符#入栈
    stack* opn = Stack_init(); //定义操作数栈opn，元素是结点的指针
    int error = 0; //错误标记error设置为0
    ASTnode* t, t1, t2; //准备在下面用于拼接三个ASTnode
    while ((w != SS || (gettop(op))->name != SS) && !error) { //运算符栈顶不是起止符号，并没有错误时
        if(w == IDENT) { //如果是标识符
            ASTnode* id = AST_mknode(IDENT, token_text, STRING)); //根据w生成一个结点，结点指针进栈opn
            w = gettoken();
            if (w == LP) push(&op); //函数调用，推入运算符栈
            else push(&op); //推入操作数栈
        }
        else if (w == INT_CONST) { //如果是整数常量
            push(&opn, AST_mknode(INT_CONST, token_int, INT)); 
            w = gettoken();
        }
        else if (w == FLOAT_CONST) { //如果是浮点数常量
            push(&opn, AST_mknode(FLOAT_CONST, token_float, FLOAT));
            w = gettoken();
        }
        else if (w == CHAR_CONST) { //如果是字符常量
            push(&opn, AST_mknode(CHAR_CONST, token_char, CHAR));
            w = gettoken();
        }
        else if (w_is_operator) //如果是运算符operator之一
            if(t->name == IDENT) { //函数调用视作一个单元运算符
                if(!pop(&opn, &t2)); error++;
                AST_add_child(t, t1);
                push(&opn, t); break;
            } //否则是二元运算符，使用下面的表格进行判断两个操作数
            switch (precede[w2precede(gettop(op))][w2precede(w)]) {
                    case '<' : push(&op, AST_mknode(w, NULL, 0)); w = gettoken(); break;
                    case '=' : if((gettop(op))==LP||w==RP) {
                                    if(!pop(&op, &t)); error++; w = gettoken(); break; //弹出无用的左括号
                                }
                               if((gettop(op))==LP||w==
                    case '>' : if(pop(&op, &t)); error++; //读出栈顶的一个运算符
                               if(!pop(&opn, &t2)); error++;
                               if(!pop(&opn, &t1)); error++;
                               if(!pop(&op, &t)); error++;
                               AST_add_child(t, t1); //根据运算符栈退栈得到的运算符t和操作数的结点指针t1,t2
                               AST_add_child(t, t2); //完成建立一个运算符的结点
                               push(&opn, t); break; //新的结点指针进入操作数栈，这个w下一个面对的将是栈下面的括号
                    default : if(w == end1 || w == end2) {token_name = w; w = SS;} //遇到结束标记分号,w被替换成为#
                              else error = 1;
            }
        else if (w == end1 || w == end2) {token_name = w; w = SS;} //遇到结束标记分号，w被替换成为#，此外将导致终止的token写入token_name
        else error = 1;
    }
    if (opn->pre = NULL/*表示操作数栈只有一个元素*/ && gettop(op) == SS && !error) {
        w = gettoken();
        ASTnode* r = AST_add_child(AST_mknode(EXPRESSION, NULL, 0), opn->content);
        stack_clear(op);stack_clear(opn);
        return r; /*返回操作数栈唯一的结点指针*/ //成功返回表达式语法树的根结点指针
    }
    else return NULL; //表达式分析出现了错误
}

//根据行列报错
void printerror(int row, int col, char* errortype) {
    printf("row:%d col: %d type:%s\n", row, col, errortype);
}

//语法树显示---------------------------------------------------
//r表示需要打印的结点，t表示开头的tab数量，有格式需要，打印方式按照结点的语法规则
void AST_show(ASTnode* r， int* t) { //递归输出解析出的语法树
    switch (r->name) {
        case 
    }
    AST_show()
}

//美化格式-----------------------------------------------------
void AST_modify(ASTnode* r) { //调整解析出的语法树
    return;
}

void main() {
    FILE* fpo = fopen("test.c", "r");
    fp = fopen(".c", "r");
    prepare(); //预处理
    ASTnode* r = program(); //词法分析并进行语法分析得到AST
    AST_show(r, 0); //展示语法树，最初的tab为0
    AST_modify(r); //美化格式，输出到特定文件
}