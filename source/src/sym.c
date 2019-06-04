////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   sym.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-18 12:12:28
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lc.h"
#include "scan.h"

PBLOCK g_block_global  = NULL;
PBLOCK g_block_current = NULL;
PBLOCK g_block_func    = NULL;
INT g_nCurrentBlockVarSize = 0;

static PSYMBOL g_SymTbl_DelayFree = NULL;

VOID Sym_AddToSymTbl(PSYMBOL *ppSymTbl, PSYMBOL pSym)
{
    pSym->pNext_InBlock = *ppSymTbl;
    *ppSymTbl = pSym;
}

//
// Return TRUE if successful removed, or FALSE if not found
//
BOOL Sym_RemoveFromSymTbl(PSYMBOL *ppSymTbl, PSYMBOL pSym)
{
    // PSYMBOL *ppSymTbl = &g_block_current->pSymTbl;
    PSYMBOL p, pPrev;

    for (p = *ppSymTbl; p; p = p->pNext_InBlock)
    {
        if (p != pSym)
        {
            // Get the previous node of the target node
            pPrev = p;
            continue;
        }

        if (*ppSymTbl == pSym)
        {
            // If head node, modify the head node's pointer to the next
            *ppSymTbl = pSym->pNext_InBlock;
        }
        else
        {
            // Point the previous node's next node to the next node
            pPrev->pNext_InBlock = p->pNext_InBlock;
        }

        return TRUE;
    }

    return FALSE;
}

PSYMBOL Sym_New(VOID)
{
    PSYMBOL pSym;

    pSym = (PSYMBOL)malloc(sizeof(*pSym));
    if (NULL == pSym)
        Error(emEI_Not_enough_memory);

    memset(pSym, 0, sizeof(*pSym));

    pSym->nLineNo = yylineno;

    Sym_SetName(pSym, yytext);

    // Add to current block's symbol table
    Sym_AddToSymTbl(&g_block_current->pSymTbl, pSym);

    return pSym;
}

PSYMBOL Sym_NewString(CONST CHAR cszString[], CONST INT nStringLen)
{
    PSYMBOL s;

    while (
        (INT)(g_nDataSize + nStringLen + sizeof(CHAR)/* '\0' */) > g_nDataMaxSize
    )
    {
        g_pbyData = (PBYTE)Gen_ReallocSpace(g_pbyData, &g_nDataMaxSize);
    }

    s = Sym_New();
    Sym_SetSpecType(s, emTYPE_POINTER);

    strncpy((PCHAR)&g_pbyData[g_nDataSize], cszString, nStringLen + sizeof(CHAR));

    s->nStrOffset = g_nDataSize;
    g_nDataSize += nStringLen + sizeof(CHAR)/* '\0' */;

    return s;
}

PSYMBOL Sym_Dup(PSYMBOL pSym)
{
    PSYMBOL s;
    PSYMBOL pNew_Next_InBlock;

    s = Sym_New();
    pNew_Next_InBlock = s->pNext_InBlock;

    memcpy(s, pSym, sizeof(*s));
    s->pNext_InBlock = pNew_Next_InBlock;

    return s;
}

static BOOL Sym_ExchangeToDelay(PSYMBOL pSym)
{
    if (pSym->bDelayFree)
    {
        Sym_RemoveFromSymTbl(&g_block_current->pSymTbl, pSym);
        Sym_AddToSymTbl(&g_SymTbl_DelayFree, pSym);
        return TRUE;
    }
    return FALSE;
}

VOID Sym_Free(PSYMBOL pSym)
{
    if (NULL == pSym)
        return ;

    if (Sym_ExchangeToDelay(pSym))
        return ;

    // Remove symbol from current block's symbol table
    if (Sym_RemoveFromSymTbl(&g_block_current->pSymTbl, pSym))
        free(pSym);
}

VOID Sym_SetSpecType(PSYMBOL pSym, SPECIFIER_TYPE SpecType)
{
    pSym->SpecType = SpecType;
}

VOID Sym_UnionSpecType(PSYMBOL pSym1, PSYMBOL pSym2)
{
    if (NULL == pSym1 || NULL == pSym2)
        return ;

    pSym1->SpecType = (SPECIFIER_TYPE)((INT)pSym1->SpecType | (INT)pSym2->SpecType);
}

VOID Sym_SetName(PSYMBOL pSym, CONST CHAR cszName[])
{
    strncpy(pSym->szName, cszName, defSYM_NAME_LEN);
}

BOOL Sym_IsSymFunc(PSYMBOL pSym)
{
    PFUNC_ITEM p;

    for (p = g_block_global->pFuncTbl; p; p = p->pNext)
    {
        if (0 == strcmp(p->pDecl->pSym->szName, pSym->szName))
            return TRUE;
    }

    return FALSE;
}

INT Sym_GetSpecSize(CONST SPECIFIER_TYPE Spec, CONST BOOL bNeedCheckPointer)
{
    SPECIFIER_TYPE SpecType = Spec;

    if (bNeedCheckPointer)
    {
        if (SpecType & emTYPE_POINTER_Mask)
            return sizeof(INT *);
    }

    /*
    if (SpecType & emTYPE_POINTER_Mask)
        SpecType = (SPECIFIER_TYPE)(SpecType ^ emTYPE_POINTER_Mask);
    */
    //SpecType = (SPECIFIER_TYPE)(SpecType & ~(emTYPE_SPEC_Mask));

    if (SpecType & emTYPE_POINTER_Mask)
    {
        if (SpecType & emTYPE_POINTER)
            SpecType = (SPECIFIER_TYPE)(SpecType ^ emTYPE_POINTER);
        else if (SpecType & emTYPE_ARRAY)
            SpecType = (SPECIFIER_TYPE)(SpecType ^ emTYPE_ARRAY);
    }

    switch (SpecType)
    {
    case emTYPE_CHAR:       return sizeof(CHAR);
    case emTYPE_SHORT:      return sizeof(SHORT);
    case emTYPE_INT:        return sizeof(INT);
    case emTYPE_LONG:       return sizeof(LONG);
    case emTYPE_SIGNED:     return sizeof(signed);
    case emTYPE_UNSIGNED:   return sizeof(unsigned);
    case emTYPE_BOOL:       return sizeof(BOOL);
    default:                return 0;
    }
}

VOID Sym_SetLocalVarOffset(PSYMBOL pSym)
{
    INT a;

    if (Sym_IsSymFunc(pSym))
        // Symbol is a function, so return
        return ;

    a = Sym_GetSpecSize(pSym->SpecType, TRUE);
    if (0 == a)
        Error(emEI_Invalid_specifier);

    if (pSym->nArrayCount)
        g_block_current->nVarSize += a * pSym->nArrayCount;
    else
        g_block_current->nVarSize += a;

    pSym->nOffset = g_block_current->nVarSize;
}

VOID Sym_SetParamVarOffset(PSYMBOL pSym, CONST BOOL bIsLibCall)
{
    INT a;

    if (Sym_IsSymFunc(pSym))
        // Symbol is a function, so return
        return ;

    a = Sym_GetSpecSize(pSym->SpecType, TRUE);
    if (0 == a)
        Error(emEI_Invalid_specifier);

    if (bIsLibCall)
    {
        pSym->nOffset = g_block_current->nParamOffset;
        g_block_func->nParamOffset -= a;
    }
    else
    {
        // sizeof_i is custom defined function's offset that pushed before call it
        pSym->nOffset = g_block_current->nParamOffset - sizeof_i;
        g_block_func->nParamOffset -= a;
        //pSym->nOffset = g_block_current->nParamOffset - sizeof_i;
        //g_block_func->nParamOffset -= a + sizeof_i;
    }

    /*
    pSym->nOffset = g_block_current->nParamOffset - a;

    g_block_func->nParamOffset -= a;
    */
}

PSYMBOL Sym_LookupSym(CONST CHAR cszName[])
{
    PBLOCK b;
    PSYMBOL p;

    // check the symbol block by block, except global block
    for (b = g_block_current; b && b != g_block_global; b = b->pFather)
    {
        for (p = b->pSymTbl; p; p = p->pNext_InBlock)
        {
            if (0 != strncmp(p->szName, cszName, defSYM_NAME_LEN))
                continue;

            return p;
        }
    }

    // in global block, just check the symbol that SymType == emST_Func
    // this is to check the pre-define functions declaration.
    // b = g_block_global; // now b is already g_block_global
    for (p = b->pSymTbl; p; p = p->pNext_InBlock)
    {
        if (p->SymType != emST_Func)
            continue;

        if (0 != strncmp(p->szName, cszName, defSYM_NAME_LEN))
            continue;

        return p;
    }

    return NULL;
}

PSYMBOL Sym_LookupLocalSym(CONST CHAR cszVarName[])
{
    PBLOCK b;
    PSYMBOL p;

    for (b = g_block_current; b != g_block_global; b = b->pFather)
    {
        for (p = b->pSymTbl; p; p = p->pNext_InBlock)
        {
            if (0 != strncmp(p->szName, cszVarName, defSYM_NAME_LEN))
                continue;

            return p;
        }
    }

    return NULL;
}

PSYMBOL Sym_LookupLabel(CONST CHAR cszName[])
{
    PBLOCK b;
    PSYMBOL p;

    // back to the top-level of function block
    for (b = g_block_current; b->pFather != g_block_global; b = b->pFather);

    // lookup label
    for (p = b->pLabelTbl; p; p = p->pNext_InBlock)
    {
        if (0 != strncmp(p->szName, cszName, defSYM_NAME_LEN))
            continue;

        return p;
    }

    return NULL;
}

PBLOCK Sym_RemoveSymFromFuncLocalBlock(PSYMBOL pSym)
{
    PBLOCK b;
    PSYMBOL p;

    for (b = g_block_current; b != g_block_global; b = b->pFather)
    {
        for (p = b->pSymTbl; p; p = p->pNext_InBlock)
        {
            if (0 != strncmp(p->szName, pSym->szName, defSYM_NAME_LEN))
                continue;

            Sym_RemoveFromSymTbl(&b->pSymTbl, pSym);
            return b;
        }
    }

    return NULL;
}

VOID Sym_AddLabel(PSYMBOL pSym)
{
    PBLOCK b;

    b = Sym_RemoveSymFromFuncLocalBlock(pSym);
    if (NULL == b)
        return ;

    for (b = g_block_current; b->pFather != g_block_global; b = b->pFather);

    Sym_AddToSymTbl(&b->pLabelTbl, pSym);
}

VOID Sym_SetSymType(PSYMBOL pSym, SYMBOL_TYPE SymType)
{
    pSym->SymType = SymType;
}

VOID Sym_SetConstant(PSYMBOL pSym, CONST INT nConstVal)
{
    if (emST_Constant != pSym->SymType)
        Sym_SetSymType(pSym, emST_Constant);

    Sym_SetSpecType(pSym, emTYPE_INT);

    pSym->nConstVal = nConstVal;
}

BOOL Sym_IsLValue(PSYMBOL pSym)
{
    switch (pSym->SymType)
    {
    case emST_Constant:
        return FALSE;

    // TODO:
    // Check if is array
    // ...
    }

    return TRUE;
}

VOID Block_Enter(CONST BLOCK_TYPE Type)
{
    PBLOCK b;

    b = (PBLOCK)malloc(sizeof(*b));
    if (NULL == b)
        Error(emEI_Not_enough_memory);

    memset(b, 0, sizeof(*b));
    b->nParamOffset
       -= sizeof_i  // param_size
        + sizeof_i  // bp
        + sizeof_i  // return pc
        ;

    b->Type    = Type;
    b->pFather = g_block_current; // g_block_current init to NULL in the first time

    g_block_current = b;

    g_nCurrentBlockVarSize = 0;
}

VOID Block_Leave(VOID)
{
    PBLOCK b;
    PSYMBOL s, s1;

    Tree_FreeInBlock(g_block_current);

    g_nCurrentBlockVarSize += g_block_current->nVarSize;

    b = g_block_current;

    // free symbol table
    for (s = b->pSymTbl; s; s = s1)
    {
        s1 = s->pNext_InBlock;

        if (Sym_ExchangeToDelay(s))
            continue;

        free(s);
    }

    // free label table
    for (s = b->pLabelTbl; s; s = s1)
    {
        s1 = s->pNext_InBlock;
        free(s);
    }

    g_block_current = g_block_current->pFather;

    free(b);
}

VOID Block_LeaveAll(VOID)
{
    PBLOCK b;

    b = g_block_current;
    while (b)
    {
        b = b->pFather;
        Block_Leave();
    }
}

VOID Block_Init(VOID)
{
    g_block_current = NULL;
    Block_Enter(emBT_Global);
    g_block_global = g_block_current;
}

VOID Block_UnInit(VOID)
{
    PFUNC_ITEM p;
    PSYMBOL s, s1;

    //
    // free function table
    //
    p = g_block_global->pFuncTbl;
    while (p)
    {
        g_block_global->pFuncTbl = g_block_global->pFuncTbl->pNext;
        free(p);
        p = g_block_global->pFuncTbl;
    }

    Block_LeaveAll();

    //
    // free delay-free symbol table
    //
    for (s = g_SymTbl_DelayFree; s; s = s1)
    {
        s1 = s->pNext_InBlock;
        free(s);
    }

    g_block_global = NULL;
}