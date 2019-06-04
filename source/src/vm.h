////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   vm.h
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-28 17:02:19
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __VM_H__
#define __VM_H__

#ifndef sizeof_i
#define sizeof_i sizeof(INT)
#endif

typedef enum _VM_ERROR_INFO
{
    emVEI_Runtime_Error_Divided_By_Zero = 0,
    emVEI_Runtime_Error_Stack_Overflow,
} VM_ERROR_INFO;

static CONST PCHAR g_pszVMErrorInfo[] =
{
    "Runtime error: Divided by zero",
    "Runtime error: Stack overflow",
};

typedef enum _OPCODE
{
    emOP_NOP = 0,
    emOP_SubSp,
    emOP_Hlt,
    emOP_Ret,
    emOP_Pop,
    emOP_Add_i,
    emOP_Sub_i,
    emOP_Mul_i,
    emOP_Div_i,
    emOP_Mod_i,
    emOP_Jmp,
    emOP_Je,
    emOP_Jne,
    emOP_Call,
    emOP_Libcall,
    emOP_Li_i,
    emOP_Libp_i,
    emOP_Ld_i,
    emOP_Ld_b,
    emOP_St_i,
    emOP_St_b,
    emOP_Libp_data,
    emOP_And_i,
    emOP_Xor_i,
    emOP_Or_i,
    emOP_Shl_i,
    emOP_Shr_i,
    emOP_Neg_i,
    emOP_Not_i,
    emOP_Cvt_b_i,
    emOP_Cvt_b_w,
    emOP_CmpEq_i,
    emOP_CmpNe_i,
    emOP_CmpLt_i,
    emOP_CmpGt_i,
    emOP_CmpLe_i,
    emOP_CmpGe_i,
    emOP_MAX_COUNT,
} OPCODE;

typedef struct _OPER
{
    OPCODE op;      // opcode
    PCHAR  name;    // mnemonic name
    INT    len;     // oper's length
} OPER, *POPER;

static CONST OPER g_OperMap[] =
{
    { emOP_NOP,         "nop",      1 },
    { emOP_SubSp,       "subsp",    1 + sizeof_i },
    { emOP_Hlt,         "hlt",      1 },
    { emOP_Ret,         "ret",      1 },
    { emOP_Pop,         "pop",      1 },
    { emOP_Add_i,       "add_i",    1 },
    { emOP_Sub_i,       "sub_i",    1 },
    { emOP_Mul_i,       "mul_i",    1 },
    { emOP_Div_i,       "div_i",    1 },
    { emOP_Mod_i,       "mod_i",    1 },
    { emOP_Jmp,         "jmp",      1 + sizeof_i },
    { emOP_Je,          "je",       1 + sizeof_i },
    { emOP_Jne,         "jne",      1 + sizeof_i },
    { emOP_Call,        "call",     1 + sizeof_i },
    { emOP_Libcall,     "libcall",  1 + sizeof_i },
    { emOP_Li_i,        "li_i",     1 + sizeof_i },
    { emOP_Libp_i,      "libp_i",   1 + sizeof_i },
    { emOP_Ld_i,        "ld_i",     1 },
    { emOP_Ld_b,        "ld_b",     1 },
    { emOP_St_i,        "st_i",     1 },
    { emOP_St_b,        "st_b",     1 },
    { emOP_Libp_data,   "libp_data",1 },
    { emOP_And_i,       "and_i",    1 },
    { emOP_Xor_i,       "xor_i",    1 },
    { emOP_Or_i,        "or_i",     1 },
    { emOP_Shl_i,       "shl_i",    1 },
    { emOP_Shr_i,       "shr_i",    1 },
    { emOP_Neg_i,       "neg_i",    1 },
    { emOP_Not_i,       "not_i",    1 },
    { emOP_Cvt_b_i,     "cvt_b_i",  1 },
    { emOP_Cvt_b_w,     "cvt_b_w",  1 },
    { emOP_CmpEq_i,     "cmpeq_i",  1 },
    { emOP_CmpNe_i,     "cmpne_i",  1 },
    { emOP_CmpLt_i,     "cmplt_i",  1 },
    { emOP_CmpGt_i,     "cmpgt_i",  1 },
    { emOP_CmpLe_i,     "cmple_i",  1 },
    { emOP_CmpGe_i,     "cmpge_i",  1 },
};

typedef enum _VM_STATUS
{
    emVS_ExceptionOccured = 0,
    emVS_Halt,
    emVS_Running,
} VM_STATUS;

#ifdef __cplusplus
extern "C" {
#endif

PCHAR VM_Mnemonic(CONST OPCODE op);
INT VM_OpLen(CONST OPCODE op);

BOOL VM_CheckCodeValid(CONST INT nCodeSize, CONST PBYTE pbyCode);

VM_STATUS VM_Interpret(
    CONST INT nCodeSize,  // sizeof(pbyCode)
    CONST PBYTE pbyCode,
    CONST INT nDataSize,
    CONST PBYTE pbyData,
    CONST INT nStackSize, // sizeof(pStack) / sizeof(pStack[0])
    INT pStack[]
);

#ifdef __cplusplus
}
#endif

#endif  // __VM_H__