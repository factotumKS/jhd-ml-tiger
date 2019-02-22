#include<stdio.h>
#define IDLEN 32

enum token_kind { //词法分析返回的token种类
    INT, FLOAT, CHAR, IF, ELSE, DO, WHILE, FOR, CONTINUE, BREAK, //10关键字
    ERROR_TOKEN, ID, INT_CONST, FLOAT_CONST, 
    ADD, SUB, MUL, DIV, MOD, EQ, NEQ, ASSIGN, AND, OR,
    LP, RP, LC, RC, COMMA, SEMI, RETURN;
};
enum node_kind{ //语法分析返回的node种类
    PROGRAM, EXT_VAR_DEF, EXT_VAR, EXT_FUNC
};
char* TYPE[] = {
    "int", "float", "char", "if", "else", "do", "while", "for", "continue", "break",
    "error_token", "id", "int_const", "float_const",
    "+", "-", "*", "/", "%", "==", "!=", "=", "&&", "||",
    "(", ")", "{", "}", ",", ";", "return",
};
typedef struct node{
    int type = 0;   //表示词法分析或语法分析结点
    int name = 0;   //表示名字
    void* t = NULL;  //表示内容
    struct node* brother = NULL;      //弟结点
    struct node* child = NULL;        //子结点
} ASTnode;

//初始化一堆用于保存字符串和数字常量的空间

FILE* fp; //源文件指针
int w; //表示刚返回得到的类型
int row, col; //用于报错的行列号
char token_text[IDLEN]; //储存文本信息的text
char token_text0[IDLEN]; //变量名
int token_type; //变量类型
float token_float; 

//预处理器-------------------------------------------------------
prepare() {
    //去除注释

    //返回文件指针
}

//语法分析------------------------------------------------------
gettoken() {
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


//数据结构------------------------------------------------------
//制作新结点    形参分别是结点类型，结点名，结点附带text位置，结点附带text类型
ASTnode* AST_mknode(int gtype, int gname, void* gtext, int gdata) {
    ASTnode* root = malloc(sizeof(ASTnode));
    root->name = gname;
    root->type = gtype;
    root->text = AST_mktext(gtext, gdata);
    return root;
}

//制作新text位置
void* AST_mktext(void* gtext, int gdata) {
    if (gdata == 0) return NULL
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

//语法分析------------------------------------------------------
//语法单位<程序>的子程序
ASTnode* program() {
    ASTnode = root;
    w = gettoken();
    if (root = ExtDefList())  return root;//语法正确返回语法树根节点
    else //语法错误
}

//！！！语法单位<外部定义序列>的子程序
ASTnode* ExtDefList() {
    if (w == EOF) return NULL;
    root = AST_mknode(); //生成外部定义序列结点root
    AST_add_child(root, ExtDef());   //处理一个外部定义得到子树，作为root
    AST_add_child(root, ExtDefList()); //作为root的第二颗子树
    return root;
}

//！语法单位<外部定义>子程序
ExtDef() { //处理外部定义序列，正确时返回子树根节点指针，否则返回NULL
    if (w != INT && w != FLOAT && w != ) {error(row, col, "外部定义-类型错误\n"); return NULL;}
    token_type = w; //保存类型说明符
    w = gettoken();
    if (w != ID) {error(row, col, "外部定义-标识符错误\n"); return NULL;}
    strcpy(token_text0, token_text); //保存第一个变量名或函数名到token_text0
    w = gettoken();
    if (w != LP) p = ExtVar(); //调用外部变量定义子程序
    else p = FuncDef(); //调用函数定义子程序
    //如果返回值p非空，表示成功完成一个外部定义的处理
}

//！!!语法单位<外部变量定义>子程序id
ExtVarDef() {
    root = AST_init(); //生成外部变量定义结点
    AST_add_child(root, AST_mknode(1, var_func_type, NULL, 0)); //根据读入外部变量的类型生成结点,作为root第一个孩子
    ASTnode* p = AST_mknode(0, EXTVAR, NULL, 0); //生成外部变量序列结点,
    AST_add_child(root, p); //p作为root第二个孩子
    AST_add_child(p, AST_mknode(1, ID, token_text0, 1)); //由保存在token_text0的第一个变量名生成第一个变量名结点
    w = gettoken();
    while (1) {
        if (w != ',' || w != ';') {error(row, col, "外部变量定义-序列错误\n"); AST_clear(root); return NULL}//报错，释放root为根的全部结点，空指针返回
        if (w == ';') {
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken();
        if (w != ID) {error(row, col, "外部变量定义-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        ASTnode* q = AST_new_child(p, AST_mknode(0, EXTVAR, NULL, 0));//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        p = q;
        AST_new_child(p, AST_make_node(1, ID, token_text0, 1)); //根据token_text的变量名生成一个变量结点，作为p的第一个孩子
        w = gettoken(); //期待得到一个逗号/分号
    }
}

//语法单位<函数定义>子程序
funDef() {
    ASTnode* root = AST_init();
    AST_new_child(root, /*类型说明符*/,); //生成返回值类型结点，作为root第一个孩子
    //调用形参子程序，生成第二棵子树
    //得到函数体子树，如果是函数声明（以分号结尾）就为空，生成第三棵子树
    retrun root;
}

//语法单位<形参>子程序
formalPara() {
    ASTnode* root = AST_init(); //生成形参定义结点
    ASTnode* p = root;
    w = gettoken(); //读取一个类型名
    while (1) {
        if (w != ',' || w != ')') {error(row, col, "形参-序列错误\n"); return NULL}//报错，释放root为根的全部结点，空指针返回
        if (w == ')') {
            w = gettoken();
            return root; //返回根节点
        }
        w = gettoken();
        if (w != ID) {error(row, col, "形参-非标识符\n"); AST_clear(root); return NULL;}//报错，释放root为根的全部结点
        ASTnode* q = AST_new_child(p, /**/, /**/);//生成外部变量序列结点，根指针为q,作为p的第二个孩子
        p = q;
        AST_new_child(p, /**/, /**/)//根据token_text的变量名生成一个变量结点，作为p的第一个孩子
        w = gettoken(); //期待得到一个逗号/分号
    }
}

}
//语法单位<复合语句>子程序

//语法单位<语句序列>子程序

//语法单位<语句>子程序

//语法单位<表达式>子程序

//根据行列报错
error(int row, int col, char* errortype) {
    printf("row:%d col: %d type:%s\n", row, col, errortype);
}

//语法树显示---------------------------------------------------

//美化格式-----------------------------------------------------