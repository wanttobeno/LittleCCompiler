////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   const.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-8-1 16:49:48
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lc.h"

static BOOL Expr_IsConst(PTREE e)
{
    return e->ExprType == emET_Const;
}

static INT Expr_ConstVal(PTREE e)
{
    return e->pSym->nConstVal;
}

PTREE Expr_ConstInteger1(CONST INT val)
{
    PTREE t;
    CHAR name[defSYM_NAME_LEN];

    itoa(val, name, 10);

    t = Tree_NewConst(val);
    Sym_SetName(t->pSym, name);

    return t;
}

PTREE Expr_ConstEval(PTREE pExpr)
{
    PTREE res;

    res = Expr_CEval(pExpr);

    return res;
}

PTREE Expr_CEval(PTREE pExpr)
{
    EXPR_TYPE ExprType = pExpr->ExprType;

    if (ExprType & emET_Binary_Mask)
        return Expr_CBinary(pExpr);

    if (ExprType & emET_Unary_Mask)
        return Expr_CUnary(pExpr);

    if (ExprType == emET_Cast)
        return Expr_CCast(pExpr);

    return Tree_Dup(pExpr);
}

PTREE Expr_CCast(PTREE pExpr)
{
    //
    // pChild1 = NewType
    // pChild2 = OldType
    //

    PTREE ec1, ec2;
    EXPR_TYPE op;
    INT val;

    op = pExpr->ExprType;
    ec1 = pExpr->pChild1;
    ec2 = Expr_CEval(pExpr->pChild2);

    if (Expr_IsConst(ec2))
    {
        val = Expr_ConstVal(ec2);
        switch (ec2->pSym->SpecType)
        {
        case emTYPE_CHAR:
            val = (CHAR)val;
            break;

        case emTYPE_SHORT:
            val = (SHORT)val;
            break;
        }
        return Expr_ConstInteger1(val);
    }

    return Expr_Binary(ec1, ec2, op);
}

PTREE Expr_CUnary(PTREE pExpr)
{
    PTREE ec1;
    EXPR_TYPE op;
    INT val1;

    op = pExpr->ExprType;

    ec1 = Expr_CEval(pExpr->pChild1);

    if (Expr_IsConst(ec1))
    {
        val1 = Expr_ConstVal(ec1);

        switch (op)
        {
        case emET_Neg:
            val1 = -val1;
            break;

        case emET_Not:
            val1 = ~val1;
            break;
        }

        return Expr_ConstInteger1(val1);
    }

    return Expr_Binary(ec1, ec1, op);
}

PTREE Expr_CBinary(PTREE pExpr)
{
    PTREE ec1, ec2;
    EXPR_TYPE op;
    INT val, val1, val2;

    op = pExpr->ExprType;

    ec1 = Expr_CEval(pExpr->pChild1);
    ec2 = Expr_CEval(pExpr->pChild2);

    if (!Expr_IsConst(ec1) || !Expr_IsConst(ec2))
    {
        return Expr_Binary(ec1, ec2, op);
    }

    val1 = Expr_ConstVal(ec1);
    val2 = Expr_ConstVal(ec2);

    switch (op)
    {
    case emET_Plus:
        val = val1 + val2;
        break;

    case emET_Minus:
        val = val1 - val2;
        break;

    case emET_Mul:
        val = val1 * val2;
        break;

    case emET_Div:
        if (0 == val2)
            Error(emEI_Potential_divide_by_0);
        val = val1 / val2;
        break;

    case emET_Mod:
        if (0 == val2)
            Error(emEI_Potential_divide_by_0);
        val = val1 % val2;
        break;

    case emET_Shl:
        val = val1 << val2;
        break;

    case emET_Shr:
        val = val1 >> val2;
        break;

    case emET_And:
        val = val1 & val2;
        break;

    case emET_Xor:
        val = val1 ^ val2;
        break;

    case emET_Or:
        val = val1 | val2;
        break;
    }

    return Expr_ConstInteger1(val);
}