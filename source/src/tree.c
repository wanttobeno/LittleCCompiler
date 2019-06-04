////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   tree.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-7-24 21:25:20
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lc.h"

#ifndef defTREE_CAPABILITY
#define defTREE_CAPABILITY 256
#endif

#ifndef defTREE_INC_SIZE
#define defTREE_INC_SIZE 32
#endif

static PTREE *g_pTreeCell;
static INT g_nTreeCapability;
static INT g_nTreeCount;

static VOID Tree_AddToCell(PTREE pTree)
{
    INT i;

    if (g_nTreeCount + 1 > g_nTreeCapability)
    {
        g_nTreeCapability += defTREE_INC_SIZE;

        g_pTreeCell = (PTREE *)realloc(g_pTreeCell, g_nTreeCapability * sizeof(*g_pTreeCell));
        if (NULL == g_pTreeCell)
            Error(emEI_Not_enough_memory);

        for (i = g_nTreeCount; i < g_nTreeCapability; ++i)
            g_pTreeCell[i] = NULL;
    }

    for (i = 0; i < g_nTreeCapability; ++i)
    {
        if (g_pTreeCell[i] == NULL)
        {
            g_pTreeCell[i] = pTree;
            ++g_nTreeCount;
            break;
        }
    }
}

static VOID Tree_RemoveFromCellByIndex(CONST INT nIndex)
{
    g_pTreeCell[nIndex] = NULL;
    --g_nTreeCount;
}

PTREE Tree_New(VOID)
{
    PTREE t;

    t = (PTREE)malloc(sizeof(*t));
    if (NULL == t)
        Error(emEI_Not_enough_memory);

    memset(t, 0, sizeof(*t));

    t->pBlock = g_block_current;

    // Add to cell
    Tree_AddToCell(t);

    return t;
}

VOID Tree_FreeInBlock(PBLOCK pBlock)
{
    INT i;

    for (i = 0; i < g_nTreeCapability; ++i)
    {
        if (g_pTreeCell[i] && g_pTreeCell[i]->pBlock == pBlock)
        {
            free(g_pTreeCell[i]);
            Tree_RemoveFromCellByIndex(i);
        }
    }
}

PTREE Tree_Dup(PTREE pTree)
{
    PTREE t;

    t = Tree_New();
    memcpy(t, pTree, sizeof(*t));

    return t;
}

VOID Tree_Init(VOID)
{
    g_pTreeCell = (PTREE *)malloc(defTREE_CAPABILITY * sizeof(*g_pTreeCell));
    if (NULL == g_pTreeCell)
        Error(emEI_Not_enough_memory);

    memset(g_pTreeCell, 0, defTREE_CAPABILITY * sizeof(*g_pTreeCell));

    g_nTreeCapability = defTREE_CAPABILITY;
    g_nTreeCount = 0;
}

VOID Tree_UnInit(VOID)
{
    INT i;

    for (i = 0; i < g_nTreeCapability; ++i)
    {
        if (g_pTreeCell[i])
        {
            free(g_pTreeCell[i]);
            g_pTreeCell[i] = NULL;
        }
    }

    free(g_pTreeCell);

    g_nTreeCapability = defTREE_CAPABILITY;
    g_nTreeCount = 0;
}

VOID Tree_AssignSym(PTREE pTree, PSYMBOL pSym)
{
    pTree->pSym = pSym;
}

PSYMBOL Tree_GetSym(PTREE pTree)
{
    return pTree->pSym;
}

VOID Tree_SetExprType(PTREE pTree, EXPR_TYPE ExprType)
{
    pTree->ExprType = ExprType;
}

PTREE Tree_NewIdent(VOID)
{
    PTREE t;
    PSYMBOL s;

    t = Tree_New();
    s = Sym_New();

    Tree_AssignSym(t, s);
    Tree_SetExprType(t, emET_Ident);

    return t;
}

PTREE Tree_NewConst(CONST INT nConstVal)
{
    PTREE t;
    PSYMBOL s;

    t = Tree_New();
    s = Sym_New();

    Sym_SetConstant(s, nConstVal);

    Tree_AssignSym(t, s);
    Tree_SetExprType(t, emET_Const);

    return t;
}

PTREE Tree_NewSpec(SPECIFIER_TYPE SpecType)
{
    PTREE t;
    PSYMBOL s;

    t = Tree_New();
    s = Sym_New();

    Sym_SetSpecType(s, SpecType);

    Tree_AssignSym(t, s);

    return t;
}

PTREE Tree_NewString(CONST CHAR cszString[], CONST INT nStringLen)
{
    PTREE t;
    PSYMBOL s;

    t = Tree_New();
    s = Sym_NewString(cszString, nStringLen);

    Tree_AssignSym(t, s);

    Tree_SetExprType(t, emET_String);

    return t;
}

PTREE Tree_LinkDeclList(PTREE pTree1, PTREE pTree2)
{
    PTREE p;

    if (NULL == pTree1 || NULL == pTree2)
        return pTree1;

    p = pTree1;
    while (p->pNext_InDecls)
        p = p->pNext_InDecls;
    p->pNext_InDecls = pTree2;

    return pTree1;
}

PTREE Tree_Append(PTREE pTree1, PTREE pTree2)
{
    PTREE p;

    if (NULL == pTree1 || NULL == pTree2)
        return pTree1;

    p = pTree1;
    while (p->pNext_InList)
        p = p->pNext_InList;
    p->pNext_InList = pTree2;

    return pTree1;
}

PTREE Tree_AppendParam(PTREE pTree1, PTREE pTree2)
{
    PTREE p;

    if (NULL == pTree1 || NULL == pTree2)
        return pTree1;

    p = pTree1;
    while (p->pNext_InParams)
        p = p->pNext_InParams;
    p->pNext_InParams = pTree2;

    return pTree1;
}

/*
 * FIXME: This is Rubbish!
VOID Tree_AddFunc(PSYMBOL pDecl, PSYMBOL pParams)
{
    PFUNC_ITEM pFunc;

    pFunc = Tree_LookupFunc(pDecl->szName);
    if (pFunc)
        // already defined
        Error(emEI_Function_redefined);

    Sym_RemoveFromSymTbl(&g_block_current->pSymTbl, pDecl);

    pFunc = (PFUNC_ITEM)malloc(sizeof(*pFunc));
    if (NULL == pFunc)
        Error(emEI_Not_enough_memory);

    pFunc->pDecl = pDecl;
    pFunc->pParams = pParams;

    pFunc->pNext = g_block_global->pFuncTbl;
    g_block_global->pFuncTbl = pFunc;
}
*/

PFUNC_ITEM Tree_LookupFunc(CONST CHAR cszName[])
{
    PFUNC_ITEM p;

    for (p = g_block_global->pFuncTbl; p; p = p->pNext)
    {
        if (0 == strcmp(cszName, p->pDecl->pSym->szName))
            return p;
    }

    return NULL;
}