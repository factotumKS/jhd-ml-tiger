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
    //语法分析中需要使用的内部结点标识
    PROGRAM, EXT_DEF_LIST, EXT_VAR_DEF, EXT_VAR, FUNC_DEF,
    STATEMENT_BLOCK, STATEMENT_LIST, 
    EXPRESSION
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
    int type = 0;   //表示词法分析或语法分析结点
    int name = 0;   //表示名字
    void* t = NULL;  //表示内容
    struct node* brother = NULL;      //弟结点
    struct node* child = NULL;        //子结点
} ASTnode;
typedef struct chain{
    ASTnode* content;
    struct chain* pre;
    struct chain* next;
} stack;
char precede[][] ={
    //+    -    *    /    (    )    =  >或< 等不等 起止符
    {'>', '>', '<', '<', '<', '>', ' ', '>', '>', '>'}, // +
    {'>', '>', '<', '<', '<', '>', ' ', '>', '>', '>'}, // -
    {'>', '>', '>', '>', '<', '>', ' ', '>', '>', '>'}, // *
    {'>', '>', '>', '>', '<', '>', ' ', '>', '>', '>'}, // /
    {'<', '<', '<', '<', '<', '=', ' ', '>', '>', '>'}, // (
    {'>', '>', '>', '>', '>', ' ', ' ', '>', '>', '>'}, // )
    {'<', '<', '<', '<', '<', ' ', '<', '<', '<', '>'}, // =
    {'<', '<', '<', '<', '<', '>', ' ', '>', '>', '>'}, // >或<
    {'<', '<', '<', '<', '<', '>', ' ', '<', '>', '>'}, // 等不等
    {'<', '<', '<', '<', '<', ' ', '<', '<', '<', '='}, // 起止符
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
        case GT  : return 7
        case LT  : return 7
        case EQ  : return 8;
        case NEQ : return 8;
        case SS  : return 9;
        default : 
            printf("表达式错误\n");
            return -1;
    }
}

//初始化一堆用于保存字符串和数字常量的空间
FILE* fp; //源文件指针
int w; //表示刚返回得到的类型
int row, col; //用于报错的行列号
//临时保存用，生成ASTnode时会动态分配空间并将内容转移
char token_text[IDLEN]; //gettoken之后，上一个标识符的text储存在这里
char token_text0[IDLEN]; //处理定义语句时需要保留某些标识符的text，所以保存在这里
int token_name; //处理定义语句时需要保存某些非标识符token的值，所以用int表示
int token_int; //gettoken之后，上一个整数常量的大小储存在这里
float token_float; //gettoken之后，上一个浮点数的大小储存在这里

//预处理器-------------------------------------------------------
prepare() {
    //去除注释

    //返回文件指针
}

//语法分析------------------------------------------------------
int gettoken() {
    char c;
    for(int i = 0; i < IDLEN; i++) token_text[i] = 0;
    
    //处理全部空白符
    while ((c = fgetc(fp)) && (c == ' '|| c == '\t'|| c == '\n')){
        if (c == '\n') {row += 1; col = 0;}
        else col += 1;
    }

    //处理标识符 关键字
    if ((c>='a' && c<= 'z') || (c>='A' && c<= 'Z')) {
        int i = 0;
        do {token_text[i] = c; i++; col += 1;}
        while ((c = fgetc(fp)) && ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c <='9')))
        ungetc(c, fp);
        for (int i = 0; i < 10; i++) {  //判断是否为关键字
            if (!strcmp(token_text, TYPE[i])) return i;
        }
        return ID; //返回标识符
    }

    //处理整数
    if (c == '-' || (c>='0' && c<='9')) {
        do {token_text[i] = c; i++; col += 1;}
        while ((c = fgetc(fp)) && (c>='0' && c <='9'))
        ungetc(c, fp);
        return INT_CONST;
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
ASTnode* AST_mknode(int gtype, int gname, void* gtext, int gdata) {
    ASTnode* root = malloc(sizeof(ASTnode));
    root->name = gname;
    root->type = gtype;
    root->text = AST_mktext(gtext, gdata);
    return root;
}

//制作新text位置，动态分配空间并将临时得到的空间转移过来
void* AST_mktext(void* gtext, int gdata) {
    if (gdata == 0) return NULL;
    else if (gdata == 1) { //复制字符串
        char* new = malloc(sizeof(strlen(gtext)) + 1);
        strcpy(new, gtext);
        return new;
    }
    else if (gdata == 2) { //复制整数
        int* new = malloc(sizeof(int));
        *new = *gtext;
        return new;
    }
    else { //复制浮点数
        float* new = malloc(sizeof(float));
        *new = *gtext;
        return new;
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
void Stack_clear(layer* t) {
    if (t == NULL) return;
    while(t->pre) Stack_clear(t->pre);
    while(t->next)Stack_clear(t->next);
    free(t); //释放掉当前所在结点
}

//语法分析------------------------------------------------------
//语法单位<程序>的子程序
ASTnode* program() {
    ASTnode* root = AST_mknode(0, PROGRAM, NULL, 0);
    w = gettoken();
    if (root->child = ExtDefList())  return root;//语法正确返回语法树根节点
    else //语法错误
}

//！！！语法单位<外部定义序列>的子程序
ASTnode* ExtDefList() {
    if (w == EOF) return NULL;
    root = AST_mknode(0, EXT_DEF_LIST, NULL, 0); //生成外部定义序列结点root
    AST_add_child(root, ExtDef());   //处理一个外部定义得到子树，作为root
    AST_add_child(root, ExtDefList()); //作为root的第二颗子树
    return root;
}

//！语法单位<外部定义>子程序
ASTnode* ExtDef() { //处理外部定义序列，正确时返回子树根节点指针，否则返回NULL
    if (w != INT && w != FLOAT && w != ) {printerror(row, col, "外部定义-类型错误\n"); return NULL;}
    token_name = w; //保存类型说明符
    w = gettoken();
    if (w != ID) {printerror(row, col, "外部定义-标识符错误\n"); return NULL;}
    strcpy(token_text0, token_text); //保存第一个变量名或函数名到token_text0
    w = gettoken();
    if (w != LP) p = ExtVar(); //调用外部变量定义子程序
    else p = FuncDef(); //调用函数定义子程序
    //如果返回值p非空，表示成功完成一个外部定义的处理
}

//！!!语法单位<外部变量定义>子程序id
ASTnode* ExtVarDef() {
    root = AST_mknode(0, EXT_VAR_DEF, NULL, 0); //生成外部变量定义结点
    AST_add_child(root, AST_mknode(1, token_name, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子
    ASTnode* p = AST_mknode(0, EXTVAR, NULL, 0); //生成外部变量序列结点,
    AST_add_child(root, p); //p作为root第二个孩子
    AST_add_child(p, AST_mknode(1, ID, token_text0, 1)); //由保存在token_text0的第一个变量名生成第一个变量名结点
    w = gettoken();
    while (1) {
        if (w != ',' || w != ';') {printerror(row, col, "外部变量定义-格式错误\n"); AST_clear(root); return NULL}//报错，释放root为根的全部结点，空指针返回
        if (w == ';') {
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken();
        if (w != ID) {printerror(row, col, "外部变量定义-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        ASTnode* q = AST_add_child(p, AST_mknode(0, EXTVAR, NULL, 0));//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        p = q;
        AST_add_child(p, AST_mknode(1, ID, token_text0, 1)); //根据token_text的变量名生成一个变量结点，作为p的第一个孩子
        w = gettoken(); //期待得到一个逗号/分号
    }
}

//！！！语法单位<函数定义>子程序
ASTnode* funcDef() {
    ASTnode* root = AST_mknode(0, FUNC_DEF, token_text0, 1);
    AST_add_child(root, AST_mknode(1, token_name, NULL, 0); //生成返回值类型结点，作为root第一个孩子
    AST_add_child(root, formalPara()); //调用形参子程序，生成第二棵子树
    if (w == ';') {AST_add_child(root, NULL); return root}
    else if (w == '{') {AST_add_child(root, funcBody()); return root;} //得到函数体子树，如果是函数声明（以分号结尾）就为空，生成第三棵子树
    else {error(row, col, "函数定义-格式错误"); free(root); return NULL}
}

//！！！语法单位<形参>子程序
ASTnode* formalPara() {
    ASTnode* root = AST_mknode(0, FORMAL_PARA, NULL, 0); //生成形参定义结点
    ASTnode* p = root;
    w = gettoken(); //读取一个类型名
    if (w == ')') {w = gettoken(); retrun root;} //没有形参
    while (1) {
        if (w != INT && w!= FLOAT && w != CHAR) {printerror(row, col, "形参-类型错误\n"); return NULL}//报错，释放root为根的全部结点，空指针返回
        AST_add_child(p, AST_mknode(1, w, NULL, 0));
        w = gettoken();
        if (w != ID) {printerror(row, col, "形参-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        AST_add_child(p, AST_mknode(1, ID, token_text, 1));
        w = gettoken();
        ASTnode* q = AST_add_child(p, AST_mknode(0, FORMAL_PARA, NULL, 0));//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        p = q;
        if (w != ',' && w != ')') {printerror(row, col, "形参-格式错误\n"; AST_clear(root); return NULL;)}
        if (w == ')') { //形参结束了
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken(); //如果为逗号就再读取下一个
    }
}

//语法单位<复合语句>子程序
ASTnode* statementBlock() {
    ASTnode* root = AST_mknode(0, STATEMENT_BLOCK, NULL, 0);
    w = gettoken();
    if (w == INT || w == FLOAT || w == CHAR) {
        token_name = w; //准备调用下面的程序
        AST_add_child(root, ExtVarDef()); //调用局部变量说明序列子程序，得到返回的子树根结点作为root的第一个孩子
    }
    else {
        AST_add_child(root, NULL); //无局部变量说明，root的第一个孩子设置为空指针
    }
    AST_add_child(root, statementList()); //调用处理语句序列子程序，返回子树根结点指针，作为root的第二个孩子
    //上面结束了这里应该是一个反大括号，可能需要gettoken()
    if (w != '}'/*w不是反大括号*/) {
        //返回空指针，报错并释放结点
        printerror(row, col, "复合语句-不正常结束\n");
        return NULL;
    }
    w = gettoken();
    return root; //返回复合语句子树根指针
}

//语法单位<语句序列>子程序
ASTnode* statementList() {
    ASTnode* root = AST_mknode(0, STATEMENT_LIST, NULL, 0);
    w = gettoken(); //读入一个字符
    ASTnode* r1 = statement(); //调用处理一条语句的子程序，返回其子树根指针r1;
    if (r1 == NULL) //erros>0的时候还需要处理错误
        return NULL;
    else {
        root->child = r1;
        root->child->brother = statementList(); //递归地调用语句序列子程序后返回值
        return root;
    }
}

//语法单位<语句>子程序
ASTnode* statement() {
    //调用此子程序时，第一个单词已经读取，根据第一个单词决定如何处理
    switch (w) {
        case IF : //分析条件语句
            w = gettoken();
            if (w != '(') {printerror(); return NULL}
            //调用处理表达式的子程序（结束符号为反小括号）正确时得到条件表达式子树指针
            //调用处理一条语句的子程序，得到IF子句的子树指针
            if (w == ELSE) {
                //调用处理一条语句的子程序，得到IF子句的子树根指针
                //生成if-else结点，下挂条件，IF子句，ELSE子句，三棵子树
            }
            else //生成IF结点，下挂条件，IF子句2棵子树
        case LC : //调用复合语句子程序，返回得到的子树指针
            ASTnode* root = statementBlock(); //不需要提前传入符号
            return root;
        case WHILE : //……
        case LP : //各种表达式语句，含有赋值，形式为表达式，以分号结束
        case ID : //……
        case INT_CONST :
            //调用表达式处理子程序（结束符号为分号）
            //正确时
                w = gettoken();
                //返回表达式语句子树指针
        case RC : //语句序列结束符号，如果语言支持switch语句，结束符号还有case和deafault
            w = gettoken();
            return NULL;
        default errors += 1; printerror(row, col, ""); return NULL; //报错并返回NULL
    }
}

//语法单位<表达式>子程序
ASTnode* expression(int endsym) { //传入结束符号，可以是反小括号或者分号
    //已经读入了一个单词在w中
    stack* op = Stack_init(); //定义运算符栈op并初始化，
    push(op, AST_mknode(1, SS, NULL, 0)); //将启止符#入栈
    stack* opn = Stack_init(); //定义操作数栈opn，元素是结点的指针
    int error = 0; //错误标记error设置为0
    ASTnode* t, t1, t2; //准备在下面用于拼接三个ASTnode
    while ((w != SS || (gettop(op))->name != SS) && !error) { //运算符栈顶不是起止符号，并没有错误时
        if(w == ID) { //如果是标识符
            push(&opn, AST_mknode(1, ID, token_text, 1)); //根据w生成一个结点，结点指针进栈opn
            w = gettoken();
        }
        else if (w == INT_CONST) { //如果是整数常量
            push(&opn, AST_mknode(1, INT_CONST, token_int, 2)); 
            w = gettoken();
        }
        else if (w_is_operator) //如果w是运算符
            switch (precede[w2precede(gettop(op))][w2precede(w)]) {
                    case '<' : push(&op, AST_mknode(1, w, NULL, 0)); w = gettoken(); break;
                    case '=' : if(!pop(&op, &t)); error++; w = gettoken(); break; //去括号
                    case '>' : if(!pop(&opn, &t2)); error++;
                               if(!pop(&opn, &t1)); error++;
                               if(!pop(&opn, &t)); error++;
                               AST_add_child(t, t1); //根据运算符栈退栈得到的运算符t和操作数的结点指针t1,t2
                               AST_add_child(t, t2); //完成建立一个运算符的结点，结点指针进栈opn
                               push(&opn, t); break;
                    default : if(w == endsym) w = SS; //遇到结束标记分号,w被替换成为#
                            else error = 1;
            }
        else if (w == endsym) w = SS; //遇到结束标记分号，w被替换成为#
        else error = 1;
    }
    if (opn->pre = NULL/*表示操作数栈只有一个元素*/ && gettop(op) == SS && !error)
        return opn->content; /*返回操作数栈唯一的结点指针*/ //成功返回表达式语法树的根结点指针
    else return NULL; //表达式分析出现了错误
}

//根据行列报错
printerror(int row, int col, char* errortype) {
    printf("row:%d col: %d type:%s\n", row, col, errortype);
}

//语法树显示---------------------------------------------------

//美化格式-----------------------------------------------------