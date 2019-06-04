%{
////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   parse.y
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-17 14:33:25
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "scan.h"
#include "lc.h"

//
// disable warning C4065: switch statement contains 'default' but no 'case' labels
//
#pragma warning(disable:4065)

%}

// Reserved words
%token tBOOL tBREAK tCASE tCHAR tCONTINUE tDEFAULT tDO tELSE tFOR
%token tGOTO tIF tINT tLONG tCONST tRETURN tSHORT tSIGNED tSIZEOF tSWITCH
%token tTYPEDEF tUNSIGNED tVOID tWHILE

%token QSTRING TYPEDEF_IDENT

//     <=  >=  ==  !=
%token LEQ GEQ EQU NEQ

// Pointer
%token ARROW

// Logical
//     &&          ||
%token LOGICAL_AND LOGICAL_OR

// Identifier
%token IDENTIFIER

// Constant
%token CONSTANT

//
%token PLUSPLUS MINUSMINUS PLUS_ASSIGN MINUS_ASSIGN MUL_ASSIGN DIV_ASSIGN
%token MOD_ASSIGN OR_ASSIGN AND_ASSIGN XOR_ASSIGN LEFTSHIFT_ASSIGN
%token RIGHTSHIFT_ASSIGN LEFTSHIFT RIGHTSHIFT ELLIPSIS

%nonassoc LOWER_THAN_ELSE
%nonassoc tELSE

%start translation_unit

%%

primary_expression
    : IDENTIFIER { $$ = Expr_LookupIdent(g_szSymName); }
    | CONSTANT
    | QSTRING
    | '(' expression ')' { $$ = $2; }
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']' { $$ = Expr_ArrayRef($1, $3); }
    | postfix_expression '(' ')' { $$ = Expr_Call($1, NULL); }
    | postfix_expression '(' argument_expression_list ')' { $$ = Expr_Call($1, $3); }
    | postfix_expression '.' IDENTIFIER
    | postfix_expression ARROW IDENTIFIER
    | postfix_expression PLUSPLUS   { $$ = Expr_AssignOp($1, Expr_ConstInteger1(1), emET_Plus); }
    | postfix_expression MINUSMINUS { $$ = Expr_AssignOp($1, Expr_ConstInteger1(1), emET_Minus); }
    | '(' type_name ')' '{' initializer_list '}'
    | '(' type_name ')' '{' initializer_list ',' '}'
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list ',' assignment_expression { $$ = Tree_AppendParam($3, $1); }
    ;

unary_expression
    : postfix_expression
    | PLUSPLUS unary_expression   { $$ = Expr_AssignOp($2, Expr_ConstInteger1(1), emET_Plus); }
    | MINUSMINUS unary_expression { $$ = Expr_AssignOp($2, Expr_ConstInteger1(1), emET_Minus); }
    | unary_operator cast_expression { $$ = Expr_Unary($2, $1->ExprType); }
    | tSIZEOF unary_expression
    | tSIZEOF '(' type_name ')'
    ;

unary_operator
    : '&'
    | '*' { $$ = Tree_New(); $$->ExprType = emET_Indir; }
    | '+' { $$ = Tree_New(); $$->ExprType = emET_Plus; }
    | '-' { $$ = Tree_New(); $$->ExprType = emET_Neg; }
    | '~' { $$ = Tree_New(); $$->ExprType = emET_Not; }
    | '!' { $$ = Tree_New(); $$->ExprType = emET_LNot; }
    ;

cast_expression
    : unary_expression
    | '(' type_name ')' cast_expression
    ;

multiplicative_expression
    : cast_expression
    | multiplicative_expression '*' cast_expression { $$ = Expr_Binary($1, $3, emET_Mul); }
    | multiplicative_expression '/' cast_expression { $$ = Expr_Binary($1, $3, emET_Div); }
    | multiplicative_expression '%' cast_expression { $$ = Expr_Binary($1, $3, emET_Mod); }
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression { $$ = Expr_Binary($1, $3, emET_Plus); }
    | additive_expression '-' multiplicative_expression { $$ = Expr_Binary($1, $3, emET_Minus); }
    ;

shift_expression
    : additive_expression
    | shift_expression LEFTSHIFT additive_expression  { $$ = Expr_Binary($1, $3, emET_Shl); }
    | shift_expression RIGHTSHIFT additive_expression { $$ = Expr_Binary($1, $3, emET_Shr); }
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression { $$ = Expr_Binary($1, $3, emET_Lt); }
    | relational_expression '>' shift_expression { $$ = Expr_Binary($1, $3, emET_Gt); }
    | relational_expression LEQ shift_expression { $$ = Expr_Binary($1, $3, emET_Le); }
    | relational_expression GEQ shift_expression { $$ = Expr_Binary($1, $3, emET_Ge); }
    ;

equality_expression
    : relational_expression
    | equality_expression EQU relational_expression { $$ = Expr_Binary($1, $3, emET_Eq); }
    | equality_expression NEQ relational_expression { $$ = Expr_Binary($1, $3, emET_Ne); }
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression { $$ = Expr_Binary($1, $3, emET_And); }
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression { $$ = Expr_Binary($1, $3, emET_Xor); }
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression { $$ = Expr_Binary($1, $3, emET_Or); }
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression LOGICAL_AND inclusive_or_expression { $$ = Expr_Binary($1, $3, emET_LAnd); }
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression LOGICAL_OR logical_and_expression { $$ = Expr_Binary($1, $3, emET_LOr); }
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression { $$ = Expr_Cond($1, $3, $5); }
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
      {
          $$ = Expr_AssignOp($1, $3, $2->ExprType);
      }
    ;

assignment_operator
    : '='               { $$ = Tree_New(); Tree_SetExprType($$, emET_Assign); }
    | MUL_ASSIGN        { $$ = Tree_New(); Tree_SetExprType($$, emET_Mul); }
    | DIV_ASSIGN        { $$ = Tree_New(); Tree_SetExprType($$, emET_Div); }
    | MOD_ASSIGN        { $$ = Tree_New(); Tree_SetExprType($$, emET_Mod); }
    | PLUS_ASSIGN       { $$ = Tree_New(); Tree_SetExprType($$, emET_Plus); }
    | MINUS_ASSIGN      { $$ = Tree_New(); Tree_SetExprType($$, emET_Minus); }
    | LEFTSHIFT_ASSIGN  { $$ = Tree_New(); Tree_SetExprType($$, emET_Shl); }
    | RIGHTSHIFT_ASSIGN { $$ = Tree_New(); Tree_SetExprType($$, emET_Shr); }
    | AND_ASSIGN        { $$ = Tree_New(); Tree_SetExprType($$, emET_And); }
    | XOR_ASSIGN        { $$ = Tree_New(); Tree_SetExprType($$, emET_Xor); }
    | OR_ASSIGN         { $$ = Tree_New(); Tree_SetExprType($$, emET_Or); }
    ;

expression
    : expression_list   { $$ = Expr_List($1); }
    ;

expression_list
    : assignment_expression
    | expression_list ',' assignment_expression { $$ = Tree_Append($1, $3); }
    ;

constant_expression
    : conditional_expression { $$ = Expr_ConstEval($1); }
    ;

declaration
    : declaration_specifiers ';' { /* empty declaration, so delete it */ Sym_Free(Tree_GetSym($1)); }
    | declaration_specifiers init_declarator_list ';'
      {
          Type_VarList($1, $2);
          Var_Declare($1, $2);
      }
    ;

declaration_specifiers
    : storage_class_specifier
    | storage_class_specifier declaration_specifiers { Sym_UnionSpecType(Tree_GetSym($1), Tree_GetSym($2)); Sym_Free(Tree_GetSym($2)); }
    | type_specifier
    | type_specifier declaration_specifiers { Sym_UnionSpecType(Tree_GetSym($1), Tree_GetSym($2)); Sym_Free(Tree_GetSym($2)); }
    | type_qualifier
    | type_qualifier declaration_specifiers { Sym_UnionSpecType(Tree_GetSym($1), Tree_GetSym($2)); Sym_Free(Tree_GetSym($2)); }
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list ',' init_declarator { $$ = Tree_LinkDeclList($1, $3); }
    ;

init_declarator
    : declarator
    | declarator '=' initializer { $$ = Expr_AssignOp($1, $3, emET_Assign); }
    ;

storage_class_specifier
    : tTYPEDEF  { /* $$ = Sym_New(); Sym_SetSpecType($$, emSTORAGE_TYPEDEF); */ }
    ;

type_specifier
    : tVOID     { $$ = Tree_NewSpec(emTYPE_VOID); }
    | tCHAR     { $$ = Tree_NewSpec(emTYPE_CHAR); }
    | tSHORT    { $$ = Tree_NewSpec(emTYPE_SHORT); }
    | tINT      { $$ = Tree_NewSpec(emTYPE_INT); }
    | tLONG     { $$ = Tree_NewSpec(emTYPE_LONG); }
    | tSIGNED   { $$ = Tree_NewSpec(emTYPE_SIGNED); }
    | tUNSIGNED { $$ = Tree_NewSpec(emTYPE_SIGNED); }
    | tBOOL     { $$ = Tree_NewSpec(emTYPE_BOOL); }
    | TYPEDEF_IDENT { $$ = Tree_NewSpec(emTYPE_TYPEDEF_IDENT); }
    ;

type_qualifier
    : tCONST    { $$ = Tree_NewSpec(emQUALIF_CONST); }
    ;

specifier_qualifier_list
    : type_specifier specifier_qualifier_list
    | type_specifier
    ;

declarator
    : pointer direct_declarator { Sym_SetSpecType($2->pSym, emTYPE_POINTER); $$ = $2; }
    | direct_declarator
    ;

direct_declarator
    : IDENTIFIER { $$ = Expr_NewIdent(g_szSymName); }
    | '(' declarator ')'    { /* TODO: pointer needed */ }
    | direct_declarator '[' constant_expression ']' { $$ = Type_ArrayDeclare($1, $3); }
    | direct_declarator '[' ']'
    | direct_declarator '(' parameter_type_list ')' { $1->pSym->SymType = emST_Func; $$ = Tree_AppendParam($1, Type_FuncDeclare($3)); }
    | direct_declarator '(' identifier_list ')'
    | direct_declarator '(' ')' { Sym_SetSymType($1->pSym, emST_Func); $$ = Tree_AppendParam($1, NULL); }
    ;

pointer
    : '*'
    | '*' pointer
    ;

parameter_type_list
    : parameter_list
    | parameter_list ',' ELLIPSIS
    ;

parameter_list
    : parameter_declaration
    | parameter_list ',' parameter_declaration { $$ = Tree_AppendParam($1, $3); }
    ;

parameter_declaration
    : declaration_specifiers declarator { $$ = Type_VarList($1, $2); }
    | declaration_specifiers abstract_declarator
    | declaration_specifiers
    ;

identifier_list
    : IDENTIFIER
    | identifier_list ',' IDENTIFIER
    ;

type_name
    : specifier_qualifier_list
    | specifier_qualifier_list abstract_declarator
    ;

abstract_declarator
    : pointer
    | direct_abstract_declarator
    | pointer direct_abstract_declarator
    ;

direct_abstract_declarator
    : '(' abstract_declarator ')'
    | '[' ']'
    | '[' constant_expression ']'
    | direct_abstract_declarator '[' ']'
    | direct_abstract_declarator '[' constant_expression ']'
    | '(' ')'
    | '(' parameter_type_list ')'
    | direct_abstract_declarator '(' ')'
    | direct_abstract_declarator '(' parameter_type_list ')'
    ;

initializer
    : assignment_expression { $$ = Expr_ConstEval($1); }
    | '{' initializer_list '}'
    | '{' initializer_list ',' '}'
    ;

initializer_list
    : initializer
    | designation initializer
    | initializer_list ',' initializer
    | initializer_list ',' designation initializer
    ;

designation
    : designator_list '='
    ;

designator_list
    : designator
    | designator_list designator
    ;

designator
    : '[' constant_expression ']'
    | '.' IDENTIFIER
    ;

statement
    : labeled_statement
    | compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    ;

labeled_statement
    : IDENTIFIER ':' { $1 = Expr_NewIdent(g_szSymName); Gen_StmtLabel($1); } statement
    | tCASE constant_expression ':' statement
    | tDEFAULT ':' statement
    ;

compound_statement
    : lbrace rbrace
    | lbrace block_item_list rbrace
    ;

lbrace
    : '{'   { Block_Enter(emBT_Declaration); }
    ;

rbrace
    : '}'   { Block_Leave(); }
    ;

block_item_list
    : block_item
    | block_item_list block_item
    ;

block_item
    : declaration
    | statement
    ;

expression_statement
    : ';' { $$ = NULL; }
    | expression ';' { Gen_StmtExpr(Expr_ConstEval($1)); }
    ;

if_test_statement
    : tIF '(' expression ')' { Gen_StmtIf1($3); }
    ;

selection_statement
    : if_test_statement statement { Gen_StmtIf2(); } %prec LOWER_THAN_ELSE
    | if_test_statement statement tELSE
      {
          Gen_StmtIfElse2();
      }
      statement
      {
          Gen_StmtIfElse3();
      }
    | tSWITCH '(' expression ')' statement
    ;

for_init_statement
    : tFOR '(' expression_statement { Gen_StmtFor1(); }
    ;

iteration_statement
    : tWHILE '(' expression ')'
      {
          Gen_StmtWhile1($3);
      }
      statement
      {
          Gen_StmtWhile2();
      }
    | tDO
      {
          Gen_StmtDoWhile1();
      }
      statement tWHILE '(' expression ')' ';'
      {
          Gen_StmtDoWhile2($6);
      }
    | for_init_statement expression_statement ')'
      {
          Gen_StmtFor2($2);
      }
      statement
      {
          Gen_StmtFor3(NULL);
      }
    | for_init_statement expression_statement expression ')'
      {
          Gen_StmtFor2($2);
      }
      statement
      {
          Gen_StmtFor3($3);
      }
    ;

jump_statement
    : tGOTO IDENTIFIER ';'   { $2 = Expr_NewIdent(g_szSymName); Gen_StmtGoto($2); }
    | tCONTINUE ';'          { Gen_StmtContinue(); }
    | tBREAK ';'             { Gen_StmtBreak(); }
    | tRETURN ';'            { Gen_StmtReturn(NULL); }
    | tRETURN expression ';' { Gen_StmtReturn($2); }

translation_unit
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_definition
    | declaration
    ;

function_definition
    : declaration_specifiers declarator declaration_list compound_statement
    | declaration_specifiers declarator
      {
          Func_Declare($1, $2, TRUE);
      }
      compound_statement
      {
          Func_End($2, TRUE);
      }
    ;

declaration_list
    : declaration
    | declaration_list declaration
    ;

%%


