%{
////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   lex.l
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-17 14:32:25
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "lc.h"
#include "parse.h"

#include <io.h>

// for VC6 only!
#pragma warning(disable:4018)

CHAR g_szSymName[defSYM_NAME_LEN];

CHAR g_szIncludeName[MAX_PATH];
INT g_nIncludeNameLen;

INCLUDE_ITEM g_Incl[defINCLUDE_MAX_LEVEL];
INT g_nInclCnt = 0;

CHAR g_szQString[defQSTRING_LEN];
INT g_nQStringLen = 0;

void get_qstring(const char ch);
int lex_char_esc(char *str);

void get_include_str(const char ch);
void push_include(void);
int pop_include(void);

int check_type();
%}

wspace  [\t\r\x0C ]
letter  [_a-zA-Z]
dec     [0-9]
oct     [0-7]
hex     [0-9a-fA-F]
ident   {letter}+({letter}|{dec})*
char_esc n|t|v|b|r|f|a|\\|\?|\"|\'|[0-7]+|x{hex}+

%x BLOCKCOMMENT
%x QSTRING_X
%x PREPROCESS_INCLUDE

%%

\n                          { /* ++yylineno; */ }
{wspace}                    { /* eat up white space */ }

[1-9]+{dec}*                { yylval = Tree_NewConst(strtol(yytext, NULL, 10)); return CONSTANT; }
0{oct}*                     { yylval = Tree_NewConst(strtol(yytext, NULL,  8)); return CONSTANT; }
0(x|X){hex}+                { yylval = Tree_NewConst(strtol(yytext, NULL, 16)); return CONSTANT; }

\'.\'                       { yylval = Tree_NewConst(yytext[1]); return CONSTANT; }
\'\\{char_esc}\'            { yylval = Tree_NewConst(lex_char_esc(&yytext[1])); return CONSTANT; }

"//".*                      { /* eat up line comment */ }

"/*"                        { BEGIN BLOCKCOMMENT; }
<BLOCKCOMMENT>[^*\n]+       { /* eat up anything that's not a '*' */ }
<BLOCKCOMMENT>"*"           { /* eat up '*'s not followed by '/'s */ }
<BLOCKCOMMENT>\n            { /* ++yylineno; */ }
<BLOCKCOMMENT>"*/"          { BEGIN INITIAL; }

\"                          { g_nQStringLen = 0; BEGIN QSTRING_X; }
<QSTRING_X>[^\"\n]          { get_qstring((char)yytext[0]); }
<QSTRING_X>\\{char_esc}     { get_qstring((char)lex_char_esc(yytext)); }
<QSTRING_X>\"               {
                                yylval = Tree_NewString(g_szQString, g_nQStringLen);
                                BEGIN INITIAL;
                                return QSTRING;
                            }

"#"[ \t]*"include"[ \t]+\"  { g_nIncludeNameLen = 0; BEGIN PREPROCESS_INCLUDE; }
<PREPROCESS_INCLUDE>[^\"\n] { get_include_str(yytext[0]); }
<PREPROCESS_INCLUDE>\"      { push_include(); BEGIN INITIAL; }

bool    { return tBOOL; }
break   { return tBREAK; }
case    { return tCASE; }
char    { return tCHAR; }
continue {return tCONTINUE; }
default { return tDEFAULT; }
do      { return tDO; }
else    { return tELSE; }
for     { return tFOR; }
goto    { return tGOTO; }
if      { return tIF; }
int     { return tINT; }
long    { return tLONG; }
const   { return tCONST; }
return  { return tRETURN; }
short   { return tSHORT; }
signed  { return tSIGNED; }
sizeof  { return tSIZEOF; }
switch  { return tSWITCH; }
typedef { return tTYPEDEF; }
unsigned {return tUNSIGNED; }
void    { return tVOID; }
while   { return tWHILE; }

":"     { return ':'; }
";"     { return ';'; }
","     { return ','; }
"."     { return '.'; }
"("     { return '('; }
")"     { return ')'; }
"["     { return '['; }
"]"     { return ']'; }
"{"     { return '{'; }
"}"     { return '}'; }

"<"     { return '<'; }
">"     { return '>'; }
"<="    { return LEQ; }
">="    { return GEQ; }
"=="    { return EQU; }
"!="    { return NEQ; }

"->"    { return ARROW; }

"&&"    { return LOGICAL_AND;}
"||"    { return LOGICAL_OR; }

"+"     { return '+'; }
"-"     { return '-'; }
"*"     { return '*'; }
"/"     { return '/'; }
"%"     { return '%'; }
"&"     { return '&'; }
"!"     { return '!'; }
"="     { return '='; }
"?"     { return '?'; }
"|"     { return '|'; }
"^"     { return '^'; }
"~"     { return '~'; }

"++"    { return PLUSPLUS; }
"--"    { return MINUSMINUS; }
"+="    { return PLUS_ASSIGN; }
"-="    { return MINUS_ASSIGN; }
"*="    { return MUL_ASSIGN; }
"/="    { return DIV_ASSIGN; }
"%="    { return MOD_ASSIGN; }
"|="    { return OR_ASSIGN; }
"&="    { return AND_ASSIGN; }
"^="    { return XOR_ASSIGN; }
"<<="   { return LEFTSHIFT_ASSIGN; }
">>="   { return RIGHTSHIFT_ASSIGN; }

"<<"    { return LEFTSHIFT; }
">>"    { return RIGHTSHIFT; }

"..."   { return ELLIPSIS; }

{ident} { return check_type(); }
.       { Error(emEI_Unknown_character, yytext[0]); }

%%

int yywrap(void)
{
    return pop_include();
}

void get_include_str(const char ch)
{
    g_szIncludeName[g_nIncludeNameLen++] = ch;
    g_szIncludeName[g_nIncludeNameLen] = '\0';
}

void get_qstring(const char ch)
{
    g_szQString[g_nQStringLen++] = ch;
    g_szQString[g_nQStringLen] = '\0';
}

int lex_char_esc(char *str)
{
    int a, c;
    char *p;

    a = str[1];
    switch (a)
    {
    case 'n':  c = '\n'; break;
    case 'r':  c = '\r'; break;
    case 't':  c = '\t'; break;
    case '\\': c = '\\'; break;
    case '\'': c = '\''; break;
    default:
        if (a >= '0' && a <= '7')
        {
            c = a - '0';
            p = &str[2];
            while (*p >= '0' && *p <= '7')
            {
                c = c * 8 + (*p - '0');
                ++p;
            }
        }
        else
        {
            c = 0;
        }
        break;
    }

    return c;
}

void push_include(void)
{
    g_szIncludeName[g_nIncludeNameLen] = '\0';

    if (g_nInclCnt > defINCLUDE_MAX_LEVEL)
        Error(emEI_Include_nested_too_deep);

    strncpy(g_Incl[g_nInclCnt].prev_filename, g_pszCurrentFileName, MAX_PATH);
    g_Incl[g_nInclCnt].prev_yybuf = YY_CURRENT_BUFFER;
    g_Incl[g_nInclCnt].prev_fp = yyin;
    ++g_nInclCnt;

    yyin = fopen(g_szIncludeName, "r");
    if (NULL == yyin)
        Error(emEI_Cannot_open_include_file, g_szIncludeName);

    g_pszCurrentFileName = g_szIncludeName;

    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
}

int pop_include(void)
{
    if (--g_nInclCnt < 0)
    {
        return 1;
    }
    else
    {
        fclose(yyin);
        yyin = g_Incl[g_nInclCnt].prev_fp;

        g_pszCurrentFileName = g_Incl[g_nInclCnt].prev_filename;

        yy_delete_buffer(YY_CURRENT_BUFFER);
        yy_switch_to_buffer((YY_BUFFER_STATE)g_Incl[g_nInclCnt].prev_yybuf);

        return 0;
    }
}

int check_type()
{
    // yylval = Expr_NewIdent(yytext);
    strncpy(g_szSymName, yytext, defSYM_NAME_LEN);

    return IDENTIFIER;
}
