////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   lc.h
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-28 17:42:17
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __LC_H__
#define __LC_H__

#include "stdafx.h"
#include "vm.h"


////////////////////////////////////////////////////////////////////////////////
// Include
//
#ifndef defINCLUDE_MAX_LEVEL
#define defINCLUDE_MAX_LEVEL 32
#endif

typedef struct _INCLUDE_ITEM
{
    CHAR prev_filename[MAX_PATH];
    INT yylineno;
    FILE *prev_fp;
    PVOID prev_yybuf;
} INCLUDE_ITEM, *PINCLUDE_ITEM;

extern INCLUDE_ITEM g_Incl[defINCLUDE_MAX_LEVEL];
extern INT g_nInclCnt;


////////////////////////////////////////////////////////////////////////////////
//
// Pre-define
//
#ifndef defSYM_NAME_LEN
#define defSYM_NAME_LEN 100
#endif

#ifndef defCODE_DATA_MAX_SIZE
#define defCODE_DATA_MAX_SIZE 8192
#endif

#ifndef defCODE_DATA_INC_SIZE
#define defCODE_DATA_INC_SIZE 4096
#endif

struct _SYMBOL;
struct _BLOCK;
struct _TREE;
typedef struct _TREE *PTREE;
typedef struct _BLOCK *PBLOCK;


////////////////////////////////////////////////////////////////////////////////
//
// General
//
extern PCHAR g_pszCurrentFileName;
extern CHAR g_szSymName[defSYM_NAME_LEN];

#ifndef defQSTRING_LEN
#define defQSTRING_LEN 512
#endif

extern CHAR g_szQString[defQSTRING_LEN];
extern INT g_nQStringLen;

int yyparse();
VOID Init(VOID);
VOID UnInit(VOID);


////////////////////////////////////////////////////////////////////////////////
//
// Error
//
extern INT g_nErrorCount;

typedef enum _ERROR_INFO
{
    emEI_Not_enough_memory  = 0,
    emEI_Invalid_specifier,
    emEI_Function_redefined,
    emEI_Undeclared_identifier,
    emEI_Left_operand_must_be_l_value,
    emEI_Unknown_character,
    emEI_Illegal_on_operands_of_type_x,
    emEI_Potential_divide_by_0,
    emEI_Illegal_break,
    emEI_Illegal_continue,
    emEI_Redefinition,
    emEI_Void_cannot_be_an_argument_type,
    emEI_Illegal_use_of_type_x,
    emEI_No_main_function,
    emEI_Function_does_not_take_x_parameters,
    emEI_Void_function_returning_a_value,
    emEI_Must_return_a_value,
    emEI_Label_redefined,
    emEI_Label_x_was_undefined,
    emEI_Expected_constant_expression,
    emEI_Negative_subscript_or_subscript_is_too_large,
    emEI_Include_path_too_long,
    emEI_Cannot_open_include_file,
    emEI_Cannot_open_file,
    emEI_Include_nested_too_deep,
} ERROR_INFO;

static CONST PCHAR g_pszErrorInfo[] =
{
    "internal error : Not enough memory",
    "invalid specifier",
    "function '%s' redefined",
    "'%s' : undeclared identifier",
    "'%s' : left operand must be l-value",
    "unknown character '0x%x'",
    "'%s' : illegal on operands of type '%s'",
    "potential divide by 0",
    "illegal break",
    "illegal continue",
    "'%s' : redefinition",
    "'void' cannot be an argument type, except for '(void)'",
    "'%s' : illegal use of type '%s'",
    "no 'main' function",
    "'%s' : function does not take %d parameters",
    "'%s' : 'void' function returning a value",
    "'%s' : must return a value",
    "'%s' : label redefined",
    "label '%s' was undefined",
    "expected constant expression",
    "negative subscript or subscript is too large",
    "include path too long",
    "Cannot open include file: '%s': No such file or directory",
    "Cannot open file: '%s': No such file or directory",
    "include nested too deep",
};

void yyerror(const char *format, ...);
VOID Error(CONST ERROR_INFO ErrInfo, ...);
VOID ErrorWithLineNo(CONST ERROR_INFO ErrInfo, CONST INT nLineNo, ...);


////////////////////////////////////////////////////////////////////////////////
//
// Symbol
//
typedef enum _SPECIFIER_TYPE
{
    emSPEC_TYPE_UNKNOWN = 0,

    emTYPE_SPEC_Mask = 0x10,
    emTYPE_VOID,
    emTYPE_CHAR,
    emTYPE_SHORT,
    emTYPE_INT,
    emTYPE_LONG,
    emTYPE_SIGNED,
    emTYPE_UNSIGNED,
    emTYPE_BOOL,

    emQUALIF_CONST = 0x20,
    
    emTYPE_POINTER_Mask = 0x40,
    emTYPE_POINTER,
    emTYPE_ARRAY,

    emTYPE_TYPEDER_Mask = 0x80,
    emTYPE_TYPEDEF_IDENT,

    emSTORAGE_TYPEDEF,
} SPECIFIER_TYPE;

CONST static PCHAR g_pszSpecTypeName[] =
{
    "",
    "void",
    "char",
    "short",
    "int",
    "long",
    "signed",
    "unsigned",
    "bool",
};

typedef enum _EXPR_TYPE
{
    emET_UNKNOWN = 0,

    emET_Const,
    emET_Ident,
    emET_Cast,
    emET_List,
    emET_Call,
    emET_Cond,
    emET_String,
    emET_Indir,

    emET_Binary_Mask = 0x10,    // do not use, it's only a mask
    emET_Assign,
    emET_Mul,
    emET_Div,
    emET_Mod,
    emET_Plus,
    emET_Minus,
    emET_Shl,
    emET_Shr,
    emET_And,
    emET_Xor,
    emET_Or,

    emET_Unary_Mask = 0x20,     // do not use, it's only a mask
    emET_Neg,
    emET_Not,
    emET_LNot,
    emET_LAnd,
    emET_LOr,

    emET_Compare_Mask = 0x40,
    emET_Eq,
    emET_Ne,
    emET_Lt,
    emET_Gt,
    emET_Le,
    emET_Ge,

    emET_Array_Mask = 0x80,

    emET_Pointer_Ref_Mask = 0x100,
} EXPR_TYPE;

typedef enum _SYMBOL_TYPE
{
    emST_UNKNOWN = 0,
    emST_Constant,
    emST_Variable,
    emST_LabelNotDefined,           // e.g: goto Label_XXXX; (but Label_XXXX has not yet been defined)
    emST_Label,
    emST_Func,
} SYMBOL_TYPE;

typedef struct _SYMBOL
{
    CHAR szName[defSYM_NAME_LEN];

    INT nLineNo;

    SPECIFIER_TYPE SpecType;

    INT nOffset;                    // offset of variable, if var
    INT nArrayCount;                // count of array, if any

    INT nStrOffset;                 // offset of string in data section, if str

    SYMBOL_TYPE SymType;            // when set to emST_Label, nConstVal is the label index

    INT nConstVal;

    BOOL bDelayFree;

    struct _SYMBOL *pNext_InBlock;  // next element in block
} SYMBOL, *PSYMBOL;

PSYMBOL Sym_New(VOID);
VOID Sym_Free(PSYMBOL pSym);
PSYMBOL Sym_NewString(CONST CHAR cszString[], CONST INT nStringLen);
PSYMBOL Sym_Dup(PSYMBOL pSym);
VOID Sym_SetSpecType(PSYMBOL pSym, SPECIFIER_TYPE SpecType);
VOID Sym_UnionSpecType(PSYMBOL pSym1, PSYMBOL pSym2);
VOID Sym_SetName(PSYMBOL pSym, CONST CHAR cszName[]);
BOOL Sym_IsSymFunc(PSYMBOL pSym);   // Is the symbol a function?
VOID Sym_SetLocalVarOffset(PSYMBOL pSym);
VOID Sym_SetParamVarOffset(PSYMBOL pSym, CONST BOOL bIsLibCall);
PSYMBOL Sym_LookupSym(CONST CHAR cszName[]);
PSYMBOL Sym_LookupLocalSym(CONST CHAR cszVarName[]);
PSYMBOL Sym_LookupLabel(CONST CHAR cszName[]);
VOID Sym_AddLabel(PSYMBOL pSym);
VOID Sym_SetSymType(PSYMBOL pSym, SYMBOL_TYPE SymType);
VOID Sym_SetConstant(PSYMBOL pSym, CONST INT nConstVal);
BOOL Sym_IsLValue(PSYMBOL pSym);
VOID Sym_AddToSymTbl(PSYMBOL *ppSymTbl, PSYMBOL pSym);
BOOL Sym_RemoveFromSymTbl(PSYMBOL *ppSymTbl, PSYMBOL pSym);
PBLOCK Sym_RemoveSymFromFuncLocalBlock(PSYMBOL pSym);
INT Sym_GetSpecSize(CONST SPECIFIER_TYPE Spec, CONST BOOL bNeedCheckPointer);


////////////////////////////////////////////////////////////////////////////////
//
// Block
//
typedef enum _BLOCK_TYPE
{
    emBT_Global = 0,
    emBT_Declaration,
    emBT_Iteration,             // for, while, do...while
    emBT_Switch,
    emBT_If,
} BLOCK_TYPE;

typedef struct _FUNC_ITEM
{
    PTREE pDecl;
    PTREE pParams;

    INT nCodeOffset;
    BOOL bReturned;

    BOOL bIsLib;

    struct _FUNC_ITEM *pNext;
} FUNC_ITEM, *PFUNC_ITEM;

extern PFUNC_ITEM g_func_current;

typedef struct _BLOCK
{
    BLOCK_TYPE Type;

    INT nVarSize;               // starting from 0
    INT nParamOffset;           // starting from -12. param_size + bp + return_pc

    PSYMBOL pSymTbl;            // symbol table
    PSYMBOL pLabelTbl;          // label table

    PFUNC_ITEM pFuncTbl;        // only available in global block, otherwise it should be NULL

    INT nLabel_Break;
    INT nLabel_Continue;
    INT nLabel_Restart;

    struct _BLOCK *pFather;
} BLOCK, *PBLOCK;

extern PBLOCK g_block_global;   // the root
extern PBLOCK g_block_current;  // always point to the last block
extern PBLOCK g_block_func;     // function block
extern INT g_nCurrentBlockVarSize;

VOID Block_Init(VOID);
VOID Block_UnInit(VOID);
VOID Block_Enter(CONST BLOCK_TYPE Type);
VOID Block_Leave(VOID);
VOID Block_LeaveAll(VOID);


////////////////////////////////////////////////////////////////////////////////
// Expr
//
PTREE Expr_LookupIdent(CONST CHAR cszName[]);
PTREE Expr_NewIdent(CONST CHAR cszName[]);
PTREE Expr_Binary(PTREE e1, PTREE e2, EXPR_TYPE AssignOp);
PTREE Expr_Assign(PTREE e1, PTREE e2);
PTREE Expr_AssignOp(PTREE e1, PTREE e2, EXPR_TYPE CONST ExprType);
PTREE Type_VarList(PTREE pSpec, PTREE pDecls);
VOID Var_Declare(PTREE pSpec, PTREE pDecls);
PTREE Expr_Cast(SPECIFIER_TYPE NewType, PTREE pOldType);
PTREE Expr_Unary(PTREE e, EXPR_TYPE CONST ExprType);
PTREE Expr_List(PTREE pExprList);
PTREE Type_FuncDeclare(PTREE pParams);
VOID Func_Declare(PTREE pSpec, PTREE pDecl, BOOL bDefine);
VOID Func_End(PTREE pDecl, BOOL bDefine);
PFUNC_ITEM Func_Lookup(CONST CHAR cszFuncName[]);
PTREE Expr_Call(PTREE pDecl, PTREE pParams);
PTREE Type_ArrayDeclare(PTREE pDecl, PTREE pExpr);
PTREE Expr_ArrayRef(PTREE pDecl, PTREE pExpr);
PTREE Expr_Cond(PTREE e, PTREE e1, PTREE e2);
PTREE Expr_LogicalInteger(PTREE pExpr);


////////////////////////////////////////////////////////////////////////////////
// Const
//
PTREE Expr_ConstEval(PTREE pExpr);
PTREE Expr_CEval(PTREE pExpr);
PTREE Expr_CCast(PTREE pExpr);
PTREE Expr_CUnary(PTREE pExpr);
PTREE Expr_CBinary(PTREE pExpr);
PTREE Expr_ConstInteger1(CONST INT val);


////////////////////////////////////////////////////////////////////////////////
//
// Gen
//
extern PBYTE g_pbyCode;
extern INT g_nCodeSize;
extern INT g_nCodeMaxSize;

extern PBYTE g_pbyData;
extern INT g_nDataSize;
extern INT g_nDataMaxSize;

VOID Gen_Init(VOID);
VOID Gen_UnInit(VOID);
VOID Gen_PrintCode(VOID);
VOID Gen_GetCode(PINT pCodeSize, PBYTE pCode);
VOID Gen_CodeFA(CONST BYTE f, CONST INT a);
VOID Gen_CodeF(CONST BYTE f);
VOID Gen_EnterGlobalFrame(VOID);
VOID Gen_LeaveGlobalFrame(VOID);
VOID Gen_EnterFrame(VOID);
VOID Gen_LeaveFrame(VOID);
VOID Gen_SetEntryPoint(VOID);
VOID Gen_SetFuncOffset(PFUNC_ITEM pFunc);
VOID Gen_Const(PTREE e);
VOID Gen_RIdent(PTREE e);
VOID Gen_Binary(PTREE e1, PTREE e2, CONST BYTE f);
VOID Gen_Assign(PTREE e);
VOID Gen_LVal(PTREE e);
VOID Gen_RVal(PTREE e);
VOID Gen_LI(CONST BYTE f, CONST INT a);
VOID Gen_ExprUnary(PTREE pExpr, CONST BYTE f);
VOID Gen_Expr(PTREE pExpr);
VOID Gen_StmtExpr(PTREE pExpr);
INT Gen_NewLabel(VOID);
VOID Gen_Label(CONST INT l);
VOID Gen_Jmp(CONST BYTE f, CONST INT l);
VOID Gen_StmtIf1(PTREE pExpr);
VOID Gen_StmtIf2(VOID);
VOID Gen_StmtIfElse2(VOID);
VOID Gen_StmtIfElse3(VOID);
VOID Gen_StmtFor1(VOID);
VOID Gen_StmtFor2(PTREE pExpr2);
VOID Gen_StmtFor3(PTREE pExpr3);
VOID Gen_StmtWhile1(PTREE pExpr);
VOID Gen_StmtWhile2(VOID);
VOID Gen_StmtDoWhile1(VOID);
VOID Gen_StmtDoWhile2(PTREE pExpr);
VOID Gen_StmtBreak(VOID);
VOID Gen_StmtContinue(VOID);
VOID Gen_ExprCall(PTREE pExpr);
VOID Gen_StmtReturn(PTREE pExpr);
VOID Gen_StmtGoto(PTREE pExpr);
VOID Gen_StmtLabel(PTREE pExpr);
VOID Gen_ExprList(PTREE pExprList);
VOID Gen_Cond(PTREE pExpr);
PVOID Gen_ReallocSpace(PBYTE pbyCode, INT *pnCodeMaxSize);
VOID Gen_String(PTREE pExpr);


////////////////////////////////////////////////////////////////////////////////
// Tree
//
typedef struct _TREE
{
    struct _TREE *pChild1;
    struct _TREE *pChild2;
    EXPR_TYPE ExprType;

    PSYMBOL pSym;

    struct _TREE *pNext_InList;
    struct _TREE *pNext_InDecls;
    struct _TREE *pNext_InParams;

    PBLOCK pBlock;
} TREE, *PTREE;

#define YYSTYPE PTREE

PTREE Tree_New(VOID);
VOID Tree_FreeInBlock(PBLOCK pBlock);
PTREE Tree_Dup(PTREE pTree);
VOID Tree_Init(VOID);
VOID Tree_UnInit(VOID);
PTREE Tree_LinkDeclList(PTREE pTree1, PTREE pTree2);
PTREE Tree_Append(PTREE pTree1, PTREE pTree2);
PTREE Tree_AppendParam(PTREE pTree1, PTREE pTree2);
VOID Tree_AssignSym(PTREE pTree, PSYMBOL pSym);
PSYMBOL Tree_GetSym(PTREE pTree);
VOID Tree_SetExprType(PTREE pTree, EXPR_TYPE ExprType);
PTREE Tree_NewIdent(VOID);
PTREE Tree_NewConst(CONST INT nConstVal);
PTREE Tree_NewSpec(SPECIFIER_TYPE SpecType);
PTREE Tree_NewString(CONST CHAR cszString[], CONST INT nStringLen);
PFUNC_ITEM Tree_LookupFunc(CONST CHAR cszName[]);


#endif  // __LC_H__