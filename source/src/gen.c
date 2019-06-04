////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   gen.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-28 17:06:12
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lc.h"

PBYTE g_pbyCode = NULL;
INT g_nCodeSize = 0;
INT g_nCodeMaxSize = 0;

PBYTE g_pbyData = NULL;
INT g_nDataSize = 0;
INT g_nDataMaxSize = 0;

typedef struct _LABEL
{
    INT nIndex;         // 1, 2, 3, ...
    INT nLabelOffset;
    struct _LABEL *pNext;
} LABEL, *PLABEL;
static PLABEL g_pLabel = NULL;
static PLABEL g_pBackfillLabel = NULL;

typedef struct _BACK_FILL
{
    INT nSubSp;
} BACK_FILL;
static BACK_FILL g_BackFill;

VOID Gen_Init(VOID)
{
    // Code
    g_pbyCode = (PBYTE)malloc(defCODE_DATA_MAX_SIZE * sizeof(*g_pbyCode));
    if (NULL == g_pbyCode)
        Error(emEI_Not_enough_memory);

    g_nCodeMaxSize = defCODE_DATA_MAX_SIZE;

    // Data
    g_pbyData = (PBYTE)malloc(defCODE_DATA_MAX_SIZE * sizeof(*g_pbyData));
    if (NULL == g_pbyData)
        Error(emEI_Not_enough_memory);

    g_nDataMaxSize = defCODE_DATA_MAX_SIZE;
}

VOID Gen_UnInit(VOID)
{
    PLABEL p;

    p = g_pBackfillLabel;
    while (g_pBackfillLabel)
    {
        p = g_pBackfillLabel->pNext;
        free(g_pBackfillLabel);
        g_pBackfillLabel = p;
    }

    p = g_pLabel;
    while (g_pLabel)
    {
        p = g_pLabel->pNext;
        free(g_pLabel);
        g_pLabel = p;
    }

    if (g_pbyData)
    {
        free(g_pbyData);
        g_pbyData = NULL;
    }
    g_nDataMaxSize = 0;

    if (g_pbyCode)
    {
        free(g_pbyCode);
        g_pbyCode = NULL;
    }
    g_nCodeMaxSize = 0;
}

PVOID Gen_ReallocSpace(PBYTE pbyCode, INT *pnCodeMaxSize)
{
    *pnCodeMaxSize += defCODE_DATA_INC_SIZE;
    pbyCode = (PBYTE)realloc(pbyCode, (*pnCodeMaxSize) * sizeof(*pbyCode));
    if (NULL == pbyCode)
        Error(emEI_Not_enough_memory);

    return pbyCode;
}

static VOID _Gen_Code(
    PBYTE *ppbyCode, INT *pnCodeMaxSize, INT *pnCodeSize,
    CONST BYTE f, CONST BOOL bHasA, CONST INT a
)
{
    if (*pnCodeSize + 1 >= *pnCodeMaxSize)
        *ppbyCode = (PBYTE)Gen_ReallocSpace(*ppbyCode, pnCodeMaxSize);

    (*ppbyCode)[*pnCodeSize] = f;
    (*pnCodeSize) += sizeof(f);

    if (bHasA)
    {
        *(INT *)&((*ppbyCode)[*pnCodeSize]) = a;
        (*pnCodeSize) += sizeof(a);
    }
}

VOID Gen_CodeFA(CONST BYTE f, CONST INT a)
{
    _Gen_Code(&g_pbyCode, &g_nCodeMaxSize, &g_nCodeSize, f, TRUE, a);
}

VOID Gen_CodeF(CONST BYTE f)
{
    _Gen_Code(&g_pbyCode, &g_nCodeMaxSize, &g_nCodeSize, f, FALSE, 0);
}

static VOID Gen_BackFill(CONST INT nIndex, CONST INT a)
{
    *(INT *)&g_pbyCode[nIndex] = a;
}

static VOID Gen_Unput(VOID)
{
    --g_nCodeSize;
}

VOID Gen_PrintCode(VOID)
{
    INT i;

    for (i = 0; i < g_nCodeSize; i += VM_OpLen((OPCODE)g_pbyCode[i]))
    {
        fprintf(stdout, "%05d\t%s", i, VM_Mnemonic((OPCODE)g_pbyCode[i]));
        if (VM_OpLen((OPCODE)g_pbyCode[i]) != 1)
            fprintf(stdout, " %d", *(INT *)&g_pbyCode[i + sizeof(BYTE)]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "-----\n");
}

VOID Gen_EnterGlobalFrame(VOID)
{
    Gen_LI(emOP_Li_i, 0);       // need to back-fill, entrypoint address
    Gen_CodeFA(emOP_Call, 0);   // main()'s parameter-size is 0
    Gen_CodeF(emOP_Pop);        // eat up Gen_LI(emOP_Li_i, 0)'s stack
    Gen_CodeF(emOP_Hlt);        // machine shut down
}

VOID Gen_LeaveGlobalFrame(VOID)
{
}

VOID Gen_EnterFrame(VOID)
{
    //
    // push esp
    // mov ebp, esp
    //

    Gen_CodeFA(emOP_SubSp, 0); // need to back-fill with g_nCurrentBlockVarSize

    // record the back-fill offset
    g_BackFill.nSubSp = g_nCodeSize - sizeof(INT);
}

VOID Gen_LeaveFrame(VOID)
{
    //
    // mov esp, ebp
    // pop ebp
    // ret
    //

    // back-fill g_nCurrentBlockVarSize
    Gen_BackFill(g_BackFill.nSubSp, g_nCurrentBlockVarSize);
}

VOID Gen_SetEntryPoint(VOID)
{
    PFUNC_ITEM pFunc;
    BOOL bFound = FALSE;

    for (pFunc = g_block_global->pFuncTbl; pFunc; pFunc = pFunc->pNext)
    {
        if (0 == strncmp(pFunc->pDecl->pSym->szName, "main", sizeof("main")))
        {
            // back-fill
            // li_i XXXXXXXX
            Gen_BackFill(1, pFunc->nCodeOffset);
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
        Error(emEI_No_main_function);
}

VOID Gen_SetFuncOffset(PFUNC_ITEM pFunc)
{
    pFunc->nCodeOffset = g_nCodeSize;
}

VOID Gen_Const(PTREE e)
{
    Gen_CodeFA(emOP_Li_i, e->pSym->nConstVal);
}

static VOID Gen_LIdent(PTREE e)
{
    if (e->ExprType & emET_Array_Mask)
    {
        Gen_LI(emOP_Libp_i, e->pSym->nOffset);
        //Gen_CodeF(emOP_Ld_i);
        Gen_ExprList(e);
        Gen_LI(emOP_Li_i, Sym_GetSpecSize(e->pSym->SpecType, FALSE));
        Gen_CodeF(emOP_Mul_i);
        Gen_CodeF(emOP_Add_i);
    }
    else
        Gen_LI(emOP_Libp_i, e->pSym->nOffset);
}

VOID Gen_RIdent(PTREE e)
{
    //PSYMBOL pSym = e->pSym;

    if (e->ExprType & emET_Array_Mask)
    {
        Gen_LI(emOP_Libp_i, e->pSym->nOffset);
        //Gen_CodeF(emOP_Ld_i);
        Gen_ExprList(e);
        Gen_LI(emOP_Li_i, Sym_GetSpecSize(e->pSym->SpecType, FALSE));
        Gen_CodeF(emOP_Mul_i);
        Gen_CodeF(emOP_Add_i);
        Gen_CodeF(emOP_Ld_i);
    }
    else if (e->ExprType & emET_Pointer_Ref_Mask)
    {
        Gen_LI(emOP_Libp_i, e->pSym->nOffset);
    }
    else
    {
        Gen_LI(emOP_Libp_i, e->pSym->nOffset);
        Gen_CodeF(emOP_Ld_i);
    }

    //Gen_CodeF(emOP_Ld_i);

    /*
    if (pSym->nArrayCount)
    {
        Gen_LI(emOP_Libp_i, pSym->nOffset);
        Gen_LI(emOP_Li_i, Sym_GetSpecSize(pSym->SpecType) * pSym->nArrayCount);
        Gen_CodeF(emOP_Sub_i);
        Gen_CodeF(emOP_Ld_i);
    }
    else
        Gen_Lod(pSym->nOffset);
    */
}

VOID Gen_Binary(PTREE e1, PTREE e2, CONST BYTE f)
{
    Gen_RVal(e1);
    Gen_RVal(e2);
    Gen_CodeF(f);
}

VOID Gen_Assign(PTREE e)
{
    SPECIFIER_TYPE SpecType;

    Gen_RVal(e->pChild2);
    Gen_LVal(e->pChild1);

    if (e->pChild1->ExprType == emET_Indir)
        SpecType = e->pChild1->pChild1->pSym->SpecType;
    else
        SpecType = e->pChild1->pSym->SpecType;

    if (SpecType & emTYPE_POINTER_Mask)
    {
        Gen_CodeF(emOP_St_i);
    }
    else
    {
        switch (SpecType)
        {
        case emTYPE_CHAR:
            Gen_CodeF(emOP_St_b);
            break;

        case emTYPE_INT:
        case emTYPE_BOOL:
            Gen_CodeF(emOP_St_i);
            break;
        }
    }
    /*
    switch (SpecType)
    {
    case emTYPE_CHAR:
        Gen_CodeF(emOP_St_b);
        break;

    default:
        Gen_CodeF(emOP_St_i);
        break;
    }
    */
}

VOID Gen_LVal(PTREE e)
{
    PSYMBOL pSym;

    if (
        (e->ExprType & emET_Array_Mask) ||
        (e->ExprType & emET_Pointer_Ref_Mask)
    )
    {
        Gen_LIdent(e);
        return ;
    }

    switch (e->ExprType)
    {
    case emET_Ident:
        pSym = e->pSym;
        if (!Sym_IsLValue(pSym))
            Error(emEI_Left_operand_must_be_l_value, e->pSym->szName);
        Gen_LIdent(e);
        break;

    case emET_Indir:
        Gen_RVal(e);
        break;
    }

    /*
    if (pSym->nArrayCount)
    {
        Gen_LI(emOP_Libp_i, pSym->nOffset);
        Gen_LI(emOP_Li_i, Sym_GetSpecSize(pSym->SpecType) * pSym->nArrayCount);
        Gen_CodeF(emOP_Sub_i);
        Gen_CodeF(emOP_St_i);
    }
    else
        Gen_Sto(pSym->nOffset);
    */
}

VOID Gen_Cast(PTREE e1, PTREE e2)
{
    SPECIFIER_TYPE NewSpecType, OldSpecType;

    NewSpecType = e1->pSym->SpecType;
    OldSpecType = e2->pSym->SpecType;

    Gen_RVal(e2);

    switch (OldSpecType)
    {
    case emTYPE_CHAR:
        switch (NewSpecType)
        {
        case emTYPE_CHAR:
            break;
        default:
            Gen_CodeF(emOP_Cvt_b_i);
            break;
        }
        break;

    case emTYPE_SHORT:
        switch (NewSpecType)
        {
        case emTYPE_CHAR:
            Gen_CodeF(emOP_Cvt_b_w);
            break;
        }
        break;
    }
}

VOID Gen_ExprList(PTREE pExprList)
{
    PTREE e;

    for (e = pExprList->pNext_InList; e; e = e->pNext_InList)
    {
        Gen_RVal(e);
    }
}

VOID Gen_LI(CONST BYTE f, CONST INT a)
{
    Gen_CodeFA(f, a);
}

static VOID Gen_ExprBinaryLogical(PTREE pTree1, PTREE pTree2, EXPR_TYPE op)
{
    INT l1, l2;

    l1 = Gen_NewLabel();
    l2 = Gen_NewLabel();

    // test tree1
    Gen_RVal(pTree1);
    if (emET_LAnd == op)
        Gen_Jmp(emOP_Je, l1);
    else
        Gen_Jmp(emOP_Jne, l1);

    // test tree2
    Gen_RVal(pTree2);
    if (emET_LAnd == op)
        Gen_Jmp(emOP_Je, l1);
    else
        Gen_Jmp(emOP_Jne, l1);

    // if all pass tree1 and tree2, set the corresponding flag
    if (emET_LAnd == op)
        Gen_LI(emOP_Li_i, 1);
    else
        Gen_LI(emOP_Li_i, 0);
    Gen_Jmp(emOP_Jmp, l2);

    // the fail exit
    Gen_Label(l1);
    if (emET_LAnd == op)
        Gen_LI(emOP_Li_i, 0);
    else
        Gen_LI(emOP_Li_i, 1);

    // the success exit
    Gen_Label(l2);
}

VOID Gen_ExprCall(PTREE pExpr)
{
    PFUNC_ITEM pFunc;
    PTREE pParam;
    INT nParamSize = 0;

    pFunc = Func_Lookup(pExpr->pChild1->pSym->szName);
    if (NULL == pFunc)
        Error(emEI_Undeclared_identifier);

    for (pParam = pExpr->pChild2; pParam; pParam = pParam->pNext_InParams)
    {
        nParamSize += sizeof_i;
        Gen_RVal(pParam);
    }

    if (pFunc->bIsLib)
    {
        Gen_CodeFA(emOP_Libcall, pFunc->nCodeOffset);
    }
    else
    {
        Gen_LI(emOP_Li_i, pFunc->nCodeOffset);
        Gen_CodeFA(emOP_Call, nParamSize);
    }
}

VOID Gen_Cond(PTREE pExpr)
{
    PTREE e, e1, e2;
    INT l1, l2;

    e = pExpr->pChild1;
    e1 = pExpr->pChild2->pChild1;
    e2 = pExpr->pChild2->pChild2;

    l1 = Gen_NewLabel();
    l2 = Gen_NewLabel();

    Gen_RVal(e);
    Gen_Jmp(emOP_Je, l1);

    Gen_RVal(e1);
    Gen_Jmp(emOP_Jmp, l2);

    Gen_Label(l1);
    Gen_RVal(e2);

    Gen_Label(l2);
}

VOID Gen_Indir(PTREE e)
{
//    SPECIFIER_TYPE SpecType;

    Gen_RVal(e->pChild1);

    /*
    SpecType = e->pChild1->pSym->SpecType;
    switch (SpecType)
    {
    case emTYPE_CHAR:
        Gen_CodeF(emOP_Ld_b);
        break;

    case emTYPE_INT:
        Gen_CodeF(emOP_Ld_i);
        break;
    }
    */
}

VOID Gen_RVal(PTREE e)
{
    if (NULL == e)
        return ;

    if (
        (e->ExprType & emET_Array_Mask) ||
        (e->ExprType & emET_Pointer_Ref_Mask)
    )
    {
        Gen_RIdent(e);
        return ;
    }

    switch (e->ExprType)
    {
    case emET_Const:
        Gen_Const(e);
        break;

    case emET_Ident:
        Gen_RIdent(e);
        break;

    case emET_Cast:
        Gen_Cast(e->pChild1, e->pChild2);
        break;

    case emET_String:
        Gen_String(e);
        break;

    case emET_Indir:
        Gen_Indir(e);
        break;

    case emET_List:
        Gen_ExprList(e);
        break;

    case emET_Call:
        Gen_ExprCall(e);
        break;

    case emET_Cond:
        Gen_Cond(e);
        break;

    case emET_Assign:
        Gen_Assign(e);
        break;

    case emET_Mul:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Mul_i);
        break;

    case emET_Div:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Div_i);
        break;

    case emET_Mod:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Mod_i);
        break;

    case emET_Plus:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Add_i);
        break;

    case emET_Minus:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Sub_i);
        break;

    case emET_Shl:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Shl_i);
        break;

    case emET_Shr:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Shr_i);
        break;

    case emET_And:
        Gen_Binary(e->pChild1, e->pChild2, emOP_And_i);
        break;

    case emET_Xor:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Xor_i);
        break;

    case emET_Or:
        Gen_Binary(e->pChild1, e->pChild2, emOP_Or_i);
        break;

    case emET_Neg:
        Gen_ExprUnary(e, emOP_Neg_i);
        break;

    case emET_Not:
        Gen_ExprUnary(e, emOP_Not_i);
        break;

    case emET_LAnd:
    case emET_LOr:
        Gen_ExprBinaryLogical(e->pChild1, e->pChild2, e->ExprType);
        break;

    case emET_Eq:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpEq_i);
        break;

    case emET_Ne:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpNe_i);
        break;

    case emET_Lt:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpLt_i);
        break;

    case emET_Gt:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpGt_i);
        break;

    case emET_Le:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpLe_i);
        break;

    case emET_Ge:
        Gen_Binary(e->pChild1, e->pChild2, emOP_CmpGe_i);
        break;
    }
}

VOID Gen_ExprUnary(PTREE pExpr, CONST BYTE f)
{
    Gen_RVal(pExpr->pChild1);
    Gen_CodeF(f);
}

VOID Gen_Expr(PTREE pExpr)
{
    Gen_RVal(pExpr);
}

VOID Gen_StmtExpr(PTREE pExpr)
{
    Gen_Expr(pExpr);
    Gen_CodeF(emOP_Pop);
}

static PLABEL _Gen_Get_Label(CONST INT l)
{
    PLABEL p = NULL;

    for (p = g_pLabel; p; p = p->pNext)
    {
        if (p->nIndex == l)
            break;
    }

    return p;
}

VOID Gen_Jmp(CONST BYTE f, CONST INT l)
{
    PLABEL p, pb;

    p = _Gen_Get_Label(l);
    if (!p)
        return ;

    //
    // label's offset already existed
    //
    if (p->nLabelOffset != -1)
    {
        Gen_CodeFA(f, p->nLabelOffset);
        return ;
    }

    //
    // need back-fill
    //
    pb = (PLABEL)malloc(sizeof(*pb));
    if (pb == NULL)
        Error(emEI_Not_enough_memory);

    pb->nIndex = l;
    pb->nLabelOffset = g_nCodeSize + sizeof(f); // nLabelOffset == Jmp 'XXXXXXXX'

    Gen_CodeFA(f, p->nLabelOffset);

    pb->pNext = g_pBackfillLabel;
    g_pBackfillLabel = pb;
}

VOID Gen_Label(CONST INT l)
{
    PLABEL p, pb;

    p = _Gen_Get_Label(l);
    if (p)
        p->nLabelOffset = g_nCodeSize;

    //
    // back-fill all the label's offset(maybe more than one place)
    //
    for (pb = g_pBackfillLabel; pb; pb = pb->pNext)
    {
        if (pb->nIndex != l)
            continue;

        Gen_BackFill(pb->nLabelOffset, p->nLabelOffset);
    }
}

INT Gen_NewLabel(VOID)
{
    PLABEL p;
    static INT nLabelNum = 0;

    p = (PLABEL)malloc(sizeof(*p));
    if (NULL == p)
        Error(emEI_Not_enough_memory);

    p->nIndex = ++nLabelNum;
    p->nLabelOffset = -1;

    p->pNext = g_pLabel;
    g_pLabel = p;

    return nLabelNum;
}

VOID Gen_StmtIf1(PTREE pExpr)
{
    INT l1, l2;

    l1 = Gen_NewLabel();
    l2 = Gen_NewLabel();

    Gen_Expr(pExpr);
    Gen_Jmp(emOP_Je, l1);

    Block_Enter(emBT_If);
    g_block_current->nLabel_Break = l1;
    g_block_current->nLabel_Continue = l2;
}

static VOID ProcessIfBlockLastExprAndSymbol(VOID)
{
}

VOID Gen_StmtIf2(VOID)
{
    Gen_Label(g_block_current->nLabel_Break);
    Block_Leave();
}

VOID Gen_StmtIfElse2(VOID)
{
    Gen_Jmp(emOP_Jmp, g_block_current->nLabel_Continue);
    Gen_Label(g_block_current->nLabel_Break);
}

VOID Gen_StmtIfElse3(VOID)
{
    Gen_Label(g_block_current->nLabel_Continue);
    Block_Leave();
}

VOID Gen_StmtFor1(VOID)
{
    INT l1;

    l1 = Gen_NewLabel();
    Gen_Label(l1);

    Block_Enter(emBT_Iteration);
    g_block_current->nLabel_Restart = l1;
}

VOID Gen_StmtFor2(PTREE pExpr2)
{
    INT l2, l3;

    l2 = Gen_NewLabel();
    l3 = Gen_NewLabel();

    //
    // eat up Gen_StmtExpr()->emOP_Pop;
    // in order to gen and run je XXXXXXXX,
    // coz je will use the stack-top value to verify the jmp flag
    //
    Gen_Unput();

    if (pExpr2)
        Gen_Jmp(emOP_Je, l2);

    g_block_current->nLabel_Break = l2;
    g_block_current->nLabel_Continue = l3;
}

VOID Gen_StmtFor3(PTREE pExpr3)
{
    Gen_Label(g_block_current->nLabel_Continue);

    if (pExpr3)
        Gen_StmtExpr(pExpr3);

    Gen_Jmp(emOP_Jmp, g_block_current->nLabel_Restart);

    Gen_Label(g_block_current->nLabel_Break);

    Block_Leave();
}

VOID Gen_StmtWhile1(PTREE pExpr)
{
    INT l1, l2;

    l1 = Gen_NewLabel();
    l2 = Gen_NewLabel();

    Gen_Label(l1);
    Gen_Expr(pExpr);
    Gen_Jmp(emOP_Je, l2);

    Block_Enter(emBT_Iteration);
    g_block_current->nLabel_Continue = l1;
    g_block_current->nLabel_Break = l2;
}

VOID Gen_StmtWhile2(VOID)
{
    Gen_Jmp(emOP_Jmp, g_block_current->nLabel_Continue);
    Gen_Label(g_block_current->nLabel_Break);
    Block_Leave();
}

VOID Gen_StmtDoWhile1(VOID)
{
    INT l1, l2, l3;

    l1 = Gen_NewLabel();
    l2 = Gen_NewLabel();
    l3 = Gen_NewLabel();

    Gen_Label(l1);

    Block_Enter(emBT_Iteration);
    g_block_current->nLabel_Restart = l1;
    g_block_current->nLabel_Continue = l2;
    g_block_current->nLabel_Break = l3;
}

VOID Gen_StmtDoWhile2(PTREE pExpr)
{
    Gen_Label(g_block_current->nLabel_Continue);

    Gen_Expr(pExpr);

    Gen_Jmp(emOP_Jne, g_block_current->nLabel_Restart);

    Gen_Label(g_block_current->nLabel_Break);

    Block_Leave();
}

VOID Gen_StmtBreak(VOID)
{
    PBLOCK b = g_block_current;

    while (1)
    {
        if (b->Type == emBT_Global)
            Error(emEI_Illegal_break);
        else if (b->Type == emBT_Iteration || b->Type == emBT_Switch)
            break;

        b = b->pFather;
    }

    Gen_Jmp(emOP_Jmp, b->nLabel_Break);
}

VOID Gen_StmtContinue(VOID)
{
    PBLOCK b = g_block_current;

    while (1)
    {
        if (b->Type == emBT_Global)
            Error(emEI_Illegal_continue);
        else if (b->Type == emBT_Iteration)
            break;

        b = b->pFather;
    }

    Gen_Jmp(emOP_Jmp, b->nLabel_Continue);
}

VOID Gen_StmtReturn(PTREE pExpr)
{
    PSYMBOL pFuncSym = g_func_current->pDecl->pSym;
    SPECIFIER_TYPE FuncSpecType = pFuncSym->SpecType;

    if (pExpr)
    {
        if (FuncSpecType == emTYPE_VOID)
            Error(emEI_Void_function_returning_a_value, pFuncSym->szName);

        Gen_Expr(pExpr);
    }
    else
    {
        if (FuncSpecType != emTYPE_VOID)
            Error(emEI_Must_return_a_value, pFuncSym->szName);
    }

    Gen_CodeF(emOP_Ret);

    g_func_current->bReturned = TRUE;
}

VOID Gen_StmtGoto(PTREE pExpr)
{
    INT l;
    PSYMBOL pSym;
    PCHAR pszLabelName = pExpr->pSym->szName;

    pSym = Sym_LookupLabel(pszLabelName);
    if (NULL == pSym)
    {
        l = Gen_NewLabel();
        Sym_SetSymType(pExpr->pSym, emST_LabelNotDefined);
        pExpr->pSym->nConstVal = l;

        Sym_AddLabel(pExpr->pSym);
    }
    else
    {
        l = pSym->nConstVal;
    }

    Gen_Jmp(emOP_Jmp, l);
}

VOID Gen_StmtLabel(PTREE pExpr)
{
    INT l;
    PSYMBOL pSym;
    PCHAR pszLabelName = pExpr->pSym->szName;

    pSym = Sym_LookupLabel(pszLabelName);
    if (pSym)
    {
        if (pSym->SymType != emST_LabelNotDefined)
            Error(emEI_Label_redefined, pszLabelName);

        pSym->SymType = emST_Label;
        l = pSym->nConstVal;
    }
    else
    {
        l = Gen_NewLabel();
        Sym_SetSymType(pExpr->pSym, emST_Label);
        pExpr->pSym->nConstVal = l;

        Sym_AddLabel(pExpr->pSym);
    }

    Gen_Label(l);
}

VOID Gen_String(PTREE pExpr)
{
    Gen_LI(emOP_Li_i, pExpr->pSym->nStrOffset);
    Gen_CodeF(emOP_Libp_data);
}