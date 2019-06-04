////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   expr.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-7-24 21:39:02
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lc.h"
#include "lib.h"

PFUNC_ITEM g_func_current = NULL;

PTREE Expr_LookupIdent(CONST CHAR cszName[])
{
    PTREE t;
    PSYMBOL pSym;

    pSym = Sym_LookupSym(cszName);
    if (NULL == pSym)
        Error(emEI_Undeclared_identifier, cszName);

    t = Tree_New();
    Tree_AssignSym(t, pSym);
    Tree_SetExprType(t, emET_Ident);

    if (pSym->nArrayCount != 0)
        t->ExprType = (EXPR_TYPE)(t->ExprType | emET_Pointer_Ref_Mask);

    return t;
}

PTREE Expr_NewIdent(CONST CHAR cszName[])
{
    PTREE t = Tree_NewIdent();
    PSYMBOL s;

    s = Tree_GetSym(t);

    Sym_SetName(s, cszName);
    Sym_SetSymType(s, emST_Variable);

    return t;
}

PTREE Expr_Binary(PTREE e1, PTREE e2, EXPR_TYPE ExprType)
{
    PTREE t;

    t = Tree_New();

    t->pChild1 = e1;
    t->pChild2 = e2;
    t->ExprType = ExprType;

    return t;
}

PTREE Expr_CastAssign(SPECIFIER_TYPE type1, PTREE e2)
{
    return Expr_Cast(type1, e2);
}

PTREE Expr_Assign(PTREE e1, PTREE e2)
{
    /*
    PTREE t;

    t = Expr_CastAssign(e1->pSym->SpecType, e2);
    return Expr_Binary(e1, t, emET_Assign);
    */

    return Expr_Binary(e1, e2, emET_Assign);
}

PTREE Expr_AssignOp(PTREE e1, PTREE e2, CONST EXPR_TYPE ExprType)
{
    if (ExprType == emET_Assign)
        return Expr_Assign(e1, e2);

    return Expr_Assign(e1, Expr_Binary(e1, e2, ExprType));

    /*
    PTREE t;

    if (emET_Assign == ExprType)
        return Expr_Assign(e1, e2);

    // Expr_Assign(e1, Expr_CastAssign(e1->ExprType, e2)

    t = Tree_New();

    t->pChild1 = e1;
    t->pChild2 = Expr_Binary(e1, Expr_Cast(e1->pSym->SpecType, e2), ExprType);
    t->ExprType = emET_Assign;

    return t;
    */
}

static PSYMBOL Tree_GetLSymFromDecl(PTREE pDecl)
{
    if (pDecl->ExprType == emET_Assign)
        return pDecl->pChild1->pSym; // e.g. int i = 1; return 'i';
    return pDecl->pSym; // e.g. int i; return 'i';
}

PTREE Type_VarList(PTREE pSpec, PTREE pDecls)
{
    PTREE decl_list;

    for (decl_list = pDecls; decl_list; decl_list = decl_list->pNext_InDecls)
    {
        Sym_UnionSpecType(Tree_GetLSymFromDecl(decl_list), pSpec->pSym);
    }

    return pDecls;
}

static VOID Local_Declare(PTREE pDecl)
{
    PSYMBOL pSym;
    PBLOCK b;

    pSym = Tree_GetLSymFromDecl(pDecl);

    //
    // We need to remove this symbol first,
    // because it has already been added to the symbol table,
    // otherwise it will ALWAYS be found!
    //
    b = Sym_RemoveSymFromFuncLocalBlock(pSym);

    if (Sym_LookupLocalSym(pSym->szName))
    {
        //
        // Add to the current block's symbol again, before Error(),
        // to avoid memory leak
        //
        Sym_AddToSymTbl(&g_block_current->pSymTbl, pSym);

        Error(emEI_Redefinition, pSym->szName);
    }

    //
    // Add to the current block's symbol table again
    //
    Sym_AddToSymTbl(&b->pSymTbl, pSym);

    Sym_SetLocalVarOffset(pSym);
    if (pDecl->ExprType == emET_Assign)
        Gen_StmtExpr(pDecl);
}

VOID Var_Declare(PTREE pSpec, PTREE pDecls)
{
    PTREE decl_list;

    for (decl_list = pDecls; decl_list; decl_list = decl_list->pNext_InDecls)
    {
        if (g_block_current == g_block_global)
        {
            if (decl_list->pSym->SymType == emST_Func)
            {
                Func_Declare(pSpec, decl_list, FALSE);
                Func_End(decl_list, FALSE);
            }
        }
        else
        {
            Local_Declare(decl_list);
        }
    }
}

static BOOL Type_IsInteger(PTREE e)
{
    switch (e->pSym->SpecType)
    {
    case emTYPE_CHAR:
    case emTYPE_SHORT:
    case emTYPE_INT:
    case emTYPE_LONG:
    case emTYPE_SIGNED:
    case emTYPE_UNSIGNED:
        return TRUE;

    default:
        return FALSE;
    }
}

static BOOL Type_IsArith(PTREE e)
{
    return Type_IsInteger(e);
}

static PCHAR Type_GetSpecTypeName(SPECIFIER_TYPE SpecType)
{
    return g_pszSpecTypeName[SpecType - emTYPE_SPEC_Mask];
}

static bool Type_Compare(SPECIFIER_TYPE type1, SPECIFIER_TYPE type2)
{
    if (type1 == emTYPE_VOID)
    {
        if (type2 == emTYPE_VOID)
            return TRUE;
        return FALSE;
    }

    if (type2 == emTYPE_VOID)
    {
        if (type1 == emTYPE_VOID)
            return TRUE;
        return FALSE;
    }

    return TRUE;
}

PTREE Expr_Cast(SPECIFIER_TYPE NewType, PTREE pOldType)
{
    if (Type_Compare(NewType, pOldType->pSym->SpecType))
        return pOldType;

    return Expr_Binary(Tree_NewSpec(NewType), pOldType, emET_Cast);
}

static PSYMBOL Tree_GetBinaryLSym(PTREE pExpr)
{
    if (pExpr->ExprType & emET_Binary_Mask)
        return pExpr->pChild1->pSym;
    return pExpr->pSym;
}

static PTREE Expr_PromoteInteger(PTREE e)
{
    PSYMBOL pSym = Tree_GetBinaryLSym(e);
    switch (pSym->SpecType)
    {
    case emTYPE_CHAR:
    case emTYPE_SHORT:
    // case emTYPE_LONG:
    // case emTYPE_SIGNED:
    // case emTYPE_UNSIGNED:
        return Expr_Cast(emTYPE_INT, e);

    default:
        return e;
    }
}

PTREE Expr_Unary(PTREE e, CONST EXPR_TYPE ExprType)
{
    PTREE e1;

    switch (ExprType)
    {
    case emET_Not:
    case emET_Plus:
    case emET_Neg:
        if (!Type_IsArith(e))
        {
            Error(
                emEI_Illegal_on_operands_of_type_x,
                e->pSym->szName,
                Type_GetSpecTypeName(e->pSym->SpecType)
            );
        }
        e1 = Expr_PromoteInteger(e);
        if (ExprType == emET_Neg || ExprType == emET_Not)
            return Expr_Binary(e1, e1, ExprType);
        else
            return e1;

    case emET_LNot:
        return Expr_Binary(e, Tree_NewConst(0), emET_Eq);

    case emET_Indir:
        return Expr_Binary(e, NULL, emET_Indir);
    }

    return NULL;
}

PTREE Expr_List(PTREE pExprList)
{
    PTREE t;

    if (NULL == pExprList->pNext_InList)
        return pExprList;

    t = Tree_New();
    t->ExprType = emET_List;
    t->pNext_InList = pExprList;

    return t;
}

PTREE Type_FuncDeclare(PTREE pParams)
{
    return pParams;
}

VOID Func_Declare(PTREE pSpec, PTREE pDecl, BOOL bDefine)
{
    PTREE pTreeParam;
    PSYMBOL pSym;
    PFUNC_ITEM pFunc;
    INT nParamCount = 0;
    BOOL bIsLib = FALSE;
    INT i;

    Block_Enter(emBT_Declaration);
    g_block_func = g_block_current;

    Sym_UnionSpecType(Tree_GetSym(pDecl), Tree_GetSym(pSpec));
    Sym_Free(pSpec->pSym);

    for (i = 0; i < emLCI_MAX_COUNT; ++i)
    {
        if (0 == strncmp(pDecl->pSym->szName, g_LibCall[i].pszFuncName, defSYM_NAME_LEN))
        {
            bIsLib = TRUE;
            break;
        }
    }

    for (
        pTreeParam = pDecl->pNext_InParams;
        pTreeParam;
        pTreeParam = pTreeParam->pNext_InParams
    )
    {
        ++nParamCount;
        pSym = Tree_GetSym(pTreeParam);

        // if param type is 'void'
        if (emTYPE_VOID == pSym->SpecType)
        {
            if (0 != strncmp(pSym->szName, "void", sizeof("void")))
                // if "foo(void a)"
                Error(emEI_Illegal_use_of_type_x, pSym->szName, Type_GetSpecTypeName(pSym->SpecType));

            if (nParamCount > 1)
                // if not only one param, e.g. "foo(void a, void b)"
                Error(emEI_Void_cannot_be_an_argument_type);

            // no errors, just skip this function's param
            pDecl->pNext_InParams = NULL; // set to NULL because 'foo(void)' equal to 'foo()'
            continue;
        }

        //if (g_block_current != g_block_global)
        if (bDefine)
        {
            // Remove from global-block
            Sym_RemoveFromSymTbl(&g_block_current->pFather->pSymTbl, pSym);
            // Add to func-block
            Sym_AddToSymTbl(&g_block_func->pSymTbl, pSym);
            // Set to delay free
            pSym->bDelayFree = TRUE;

            Sym_SetParamVarOffset(pSym, bIsLib);
        }
    }

    pFunc = Func_Lookup(pDecl->pSym->szName);

    if (pFunc == NULL)
    {
        pFunc = (PFUNC_ITEM)malloc(sizeof(*pFunc));
        if (NULL == pFunc)
            Error(emEI_Not_enough_memory);

        memset(pFunc, 0, sizeof(*pFunc));

        pFunc->pDecl = pDecl;
        pFunc->pParams = pDecl->pNext_InParams;

        pFunc->pNext = g_block_global->pFuncTbl;
        g_block_global->pFuncTbl = pFunc;
    }

    g_func_current = pFunc;

    if (bDefine)
    {
        Gen_SetFuncOffset(pFunc);

        Gen_EnterFrame();
    }
}

VOID Func_End(PTREE pDecl, BOOL bDefine)
{
    PFUNC_ITEM pFunc;
    PBLOCK b;
    PSYMBOL s;
    INT i;

    pFunc = Func_Lookup(pDecl->pSym->szName);
    if (NULL == pFunc)
        Error(emEI_Undeclared_identifier, pDecl->pSym->szName);

    for (i = 0; i < emLCI_MAX_COUNT; ++i)
    {
        if (0 == strncmp(pFunc->pDecl->pSym->szName, g_LibCall[i].pszFuncName, defSYM_NAME_LEN))
        {
            pFunc->bIsLib = TRUE;
            pFunc->nCodeOffset = g_LibCall[i].FuncIdx;
            goto Exit1;
        }
    }

    if (bDefine)
        Gen_LeaveFrame();

    if (pFunc->pDecl->pSym->SpecType == emTYPE_VOID)
    {
        if (!pFunc->bReturned && bDefine)
            Gen_CodeF(emOP_Ret);
    }
    else
    {
        if (!pFunc->bReturned)
            Error(emEI_Must_return_a_value, pFunc->pDecl->pSym->szName);
    }

    //
    // Check whether still has not defined label
    //
    b = g_block_func;
    for (s = b->pLabelTbl; s; s = s->pNext_InBlock)
    {
        if (s->SymType == emST_LabelNotDefined)
            ErrorWithLineNo(emEI_Label_x_was_undefined, s->nLineNo, s->szName);
    }

Exit1:
    Block_Leave();
    g_block_func = NULL;
}

PFUNC_ITEM Func_Lookup(CONST CHAR cszFuncName[])
{
    PFUNC_ITEM p;

    for (p = g_block_global->pFuncTbl; p; p = p->pNext)
    {
        if (0 == strncmp(p->pDecl->pSym->szName, cszFuncName, defSYM_NAME_LEN))
        {
            return p;
        }
    }

    return NULL;
}

PTREE Expr_Call(PTREE pDecl, PTREE pParams)
{
    PSYMBOL pSym;
    PFUNC_ITEM pFunc;
    PTREE pTreeParam;
    PTREE pArgu;
    INT nParamCntNeeded = 0;
    INT nParamCount = 0;

    pSym = pDecl->pSym;
    pFunc = Func_Lookup(pSym->szName);
    if (NULL == pFunc)
        Error(emEI_Undeclared_identifier, pSym->szName);

    //
    // Get param count needed
    //
    for (pTreeParam = pFunc->pParams; pTreeParam; pTreeParam = pTreeParam->pNext_InParams)
        ++nParamCntNeeded;

    //
    // Get argument count
    //
    for (pArgu = pParams; pArgu; pArgu = pArgu->pNext_InParams)
        ++nParamCount;

    //
    // Check param count valid
    //
    if (nParamCntNeeded != nParamCount)
        Error(emEI_Function_does_not_take_x_parameters, pSym->szName, nParamCount);

    //
    // Check params' type
    //
    // For technological reason, I did not finish it.
    /*
    for (
        pTreeParam = pFunc->pParams, pArgu = pParams;
        pArgu;
        pTreeParam = pTreeParam->pNext_InParams, pArgu = pArgu->pNext_InParams
    )
    {
        if (
            NULL == pTreeParam ||
            !Type_Compare(pTreeParam->pSym->SpecType, pArgu->pSym->SpecType)
        )
        {
            Error(emEI_Function_does_not_take_these_types_of_parameters, pSym->szName);
        }
    }
    */

    return Expr_Binary(pFunc->pDecl, pParams, emET_Call);
}

PTREE Type_ArrayDeclare(PTREE pDecl, PTREE pExpr)
{
    INT nSize;
    PSYMBOL pSym = Tree_GetBinaryLSym(pExpr);

    if (NULL == pExpr)
        nSize = 0;
    else
    {
        if (!(pSym->SpecType & emTYPE_SPEC_Mask) || Sym_GetSpecSize(pSym->SpecType, FALSE) == 0)
            Error(emEI_Expected_constant_expression);
        nSize = pSym->nConstVal;
        if (nSize <= 0)
            Error(emEI_Negative_subscript_or_subscript_is_too_large);
    }

    if (pDecl)
    {
        if (pDecl->pSym)
            pDecl->pSym->nArrayCount = nSize;

        pDecl->ExprType = (EXPR_TYPE)(pDecl->ExprType | emET_Array_Mask);
    }

    return pDecl;
}

PTREE Expr_ArrayRef(PTREE pDecl, PTREE pExpr)
{
    /*
    PTREE t;
    PSYMBOL pSymArray;
    PSYMBOL pSym = Tree_GetArrayConstSym(pExpr);

    pSymArray = Sym_Dup(pDecl->pSym);
    pSymArray->nConstVal = pSym->nConstVal;
    Tree_AssignSym(pDecl, pSymArray);

    t = Tree_New();
    Tree_AssignSym(t, pSymArray);

    Sym_Free(pSym);

    return Tree_Append(pDecl, t);
    */
    pDecl->ExprType = (EXPR_TYPE)(pDecl->ExprType | emET_Array_Mask);
    return Tree_Append(pDecl, pExpr);
}

PTREE Expr_Cond(PTREE e, PTREE e1, PTREE e2)
{
    return Expr_Binary(e, Expr_Binary(e1, e2, emET_UNKNOWN), emET_Cond);
}