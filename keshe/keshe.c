//枚举各类单词
enum token_kind {ERROR_TOKEN,IDENT,INT_CONST,FLOAT_CONST,CHAR_CONST,

//词法分析函数
int gettoken (源文件指针：fp) {
    char token_text[128];
    
    //过滤空白符号；需要报错信息（换行符=》行号++）
    while ((c=fgetc(fp))为空白符);
    
    //处理标识符&关键字
    if () {
        do{token_text}
        while((c=fgetc(fp))是字母或数字)    //拼标识符串
        ungetc(c, fp);
        if……//关键字
        return 
    }
    
    //处理数字常量
    if () {
        do{token_text+ctoken_text}
        while((c=fgetc(fp))是数字)  //拼数字串
        ungetc(c, fp);
        return 
    }
    
    switch (c) {
        case '=':
            c = fgetc(fp);
            if (c == '=')   return EQ
            ungetc(c, fp);  return ASSIGN
            break;
        
        case :

        default :
            if (feof(fp)) return EOF;
            else return ERROR_TOKEN;    //报错
    }
}