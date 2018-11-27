type pos = int
type lexresult = Tokens.token

val lineNum = ErrorMsg.lineNum
val linePos = ErrorMsg.linePos
(* "ref" means they are all pointer. *)
val commentLen = ref 0
val stringPos = ref 0
val stringCon = ref ""
fun err(p1,p2) = ErrorMsg.error p1

(* if comment doesn't end there is a EOF error*)
fun eof() = 
let val pos = hd(!linePos) 
in if !commentLen = 0 then ()
else (ErrorMsg.error pos "nuclosed comment"); Tokens.EOF(pos,pos) end

%%
newline = [\n\r];
white = [\t\ ];
quote = \";
%s COMMENT STRING; 

%%
<INITIAL,COMMENT> {newline} => (lineNum := !lineNum+1; linePos := yypos :: !linePos; continue());
<INITIAL,COMMENT> {white}+  => (continue());

<INITIAL> "var"   => (Tokens.VAR(yypos,yypos+3));
<INITIAL> "function"  => (Tokens.FUNCTION(yypos,yypos+8));
<INITIAL> "typ"   => (Tokens.TYPE(yypos,yypos+8));
<INITIAL> "break" => (Tokens.BREAK(yypos,yypos+5));
<INITIAL> "for"   => (Tokens.FOR(yypos,yypos+3));
<INITIAL> "while" => (Tokens.WHILE(yypos,yypos+5));
<INITIAL> "do"    => (Tokens.DO(yypos,yypos+2));
<INITIAL> "of"    => (Tokens.OF(yypos,yypos+2));
<INITIAL> "to"    => (Tokens.TO(yypos,yypos+2));
<INITIAL> "let"   => (Tokens.LET(yypos,yypos+3));
<INITIAL> "in"    => (Tokens.IN(yypos,yypos+2));
<INITIAL> "end"   => (Tokens.END(yypos,yypos+3));
<INITIAL> "nil"   => (Tokens.NIL(yypos,yypos+3));
<INITIAL> "if"    => (Tokens.IF(yypos,yypos+2));
<INITIAL> "then"  => (Tokens.THEN(yypos,yypos+4));
<INITIAL> "else"  => (Tokens.ELSE(yypos,yypos+4));
<INITIAL> "array" => (Tokens.ARRAY(yypos,yypos+5));
<INITIAL> ":="    => (Tokens.ASSIGN(yypos,yypos+2));
<INITIAL> "|"     => (Tokens.OR(yypos,yypos+1));
<INITIAL> "&"     => (Tokens.AND(yypos,yypos+1));
<INITIAL> ">="    => (Tokens.GE(yypos,yypos+2));
<INITIAL> "<="    => (Tokens.LE(yypos,yypos+2));
<INITIAL> ">"     => (Tokens.GT(yypos,yypos+1));
<INITIAL> "<"     => (Tokens.LT(yypos,yypos+1));
<INITIAL> "="     => (Tokens.EQ(yypos,yypos+1));
<INITIAL> "<>"    => (Tokens.NEQ(yypos,yypos+2));
<INITIAL> "+"     => (Tokens.PLUS(yypos,yypos+1));
<INITIAL> "-"     => (Tokens.MINUS(yypos,yypos+1));
<INITIAL> "*"     => (Tokens.TIMES(yypos,yypos+1));
<INITIAL> "/"     => (Tokens.DIVIDE(yypos,yypos+1));
<INITIAL> "."     => (Tokens.DOT(yypos,yypos+1));
<INITIAL> "{"     => (Tokens.LBRACE(yypos,yypos+1));
<INITIAL> "}"     => (Tokens.RBRACE(yypos,yypos+1));
<INITIAL> "["     => (Tokens.LBRACK(yypos,yypos+1));
<INITIAL> "]"     => (Tokens.RBRACK(yypos,yypos+1));
<INITIAL> "("     => (Tokens.LPAREN(yypos,yypos+1));
<INITIAL> ")"     => (Tokens.RPAREN(yypos,yypos+1));
<INITIAL> ":"     => (Tokens.COLON(yypos,yypos+1));
<INITIAL> ";"     => (Tokens.SEMICOLON(yypos,yypos+1));
<INITIAL> ","	    => (Tokens.COMMA(yypos,yypos+1));
<INITIAL> "/*"    => (YYBEGIN COMMENT; commentLen := 1; continue());
<INITIAL> {quote} => (YYBEGIN STRING; stringCon := ""; stringPos := yypos; continue());
<INITIAL> [a-zA-Z][a-zA-Z0-9]*  => (Tokens.ID(yytext,yypos,yypos+size yytext));
<INITIAL> [0-9]+  => (Tokens.INT(valOf(Int.fromString yytext),yypos,yypos+size yytext));
<INITIAL> .       => (ErrorMsg.error yypos ("illegal character " ^ yytext); continue());

<COMMENT> "*/"  => (YYBEGIN INITIAL; commentLen := 0; continue());
<COMMENT> .     => (continue());

<STRING> {quote}  => (YYBEGIN INITIAL; Tokens.STRING(!stringCon,!stringPos,yypos));
<STRING> .        => (stringCon := !stringCon ^ yytext; continue());
