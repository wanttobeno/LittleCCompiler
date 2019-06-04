////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   vm.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-7-3 17:07:12
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vm.h"
#include "lib.h"

#include <stdarg.h>

#define Push_i(__a__)   _Push_i(&sp, (__a__))
#define Pop_i()         _Pop_i(&sp)

static VOID VM_Abort(VM_STATUS *pVMStatus)
{
    *pVMStatus = emVS_ExceptionOccured;
}

static VOID VM_Printf(CONST PCHAR format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    vfprintf(stdout, format, arglist);
    va_end(arglist);

    fprintf(stdout, "\n");
}

VOID VM_Error(VM_ERROR_INFO ErrInfo, ...)
{
    va_list arglist;

    fprintf(stderr, "VM Runtime Error: ");

    va_start(arglist, ErrInfo);
    vfprintf(stderr, g_pszVMErrorInfo[ErrInfo], arglist);
    va_end(arglist);

    fprintf(stderr, "\n");
}

PCHAR VM_Mnemonic(CONST OPCODE op)
{
    return g_OperMap[op].name;
}

INT VM_OpLen(CONST OPCODE op)
{
    return g_OperMap[op].len;
}

static VOID _Push_i(PINT *_sp, CONST INT nVal)
{
    --(*_sp);
    (*_sp)[0] = nVal;
}

static INT _Pop_i(PINT *_sp)
{
    INT a = (*_sp)[0];
    ++(*_sp);
    return a;
}

BOOL VM_CheckCodeValid(CONST INT nCodeSize, CONST PBYTE pbyCode)
{
    INT i;
    INT nSize = 0;

    for (i = 0; i < nCodeSize; i += VM_OpLen((OPCODE)pbyCode[i]))
    {
        if ((CHAR)pbyCode[i] < emOP_NOP || pbyCode[i] >= emOP_MAX_COUNT)
            return FALSE;

        nSize += VM_OpLen((OPCODE)pbyCode[i]);
    }

    if (nSize != nCodeSize)
        return FALSE;

    return TRUE;
}

VM_STATUS VM_Interpret(
    CONST INT nCodeSize,  CONST PBYTE pbyCode,
    CONST INT nDataSize,  CONST PBYTE pbyData,
    CONST INT nStackSize, INT pStack[]
)
{
    VM_STATUS VMStatus;
    PINT sp;
    PINT bp;
    INT pc;
    BYTE f;
    INT a;
    INT b;

#ifndef get_a
#define get_a() *(INT *)&pbyCode[pc], pc += sizeof_i
#endif

    if (FALSE == VM_CheckCodeValid(nCodeSize, pbyCode))
    {
        VM_Printf("-----\nVM code checking not valid.");
        VM_Abort(&VMStatus);
        return VMStatus;
    }

    // VM initialize
    pc = 0;
    VMStatus = emVS_Running;
    bp = sp = pStack + nStackSize;

    while (emVS_Running == VMStatus)
    {
        if (sp - pStack < 0)
        {
            VM_Error(emVEI_Runtime_Error_Stack_Overflow);
            VM_Abort(&VMStatus);
            continue;
        }

        f = pbyCode[pc++];

        switch (f)
        {
        case emOP_NOP:
            break;

        case emOP_Pop:
            ++sp;
            break;

        case emOP_SubSp:
            a = get_a();
            sp = (PINT)((INT)sp - a);
            break;

        case emOP_Hlt:
            VMStatus = emVS_Halt;
            break;

        case emOP_Add_i:
            ++sp;
            sp[0] += sp[-1];
            break;

        case emOP_Sub_i:
            ++sp;
            sp[0] -= sp[-1];
            break;

        case emOP_Mul_i:
            ++sp;
            sp[0] *= sp[-1];
            break;

        case emOP_Div_i:
            ++sp;
            if (0 == sp[-1])
            {
                VM_Error(emVEI_Runtime_Error_Divided_By_Zero);
                VM_Abort(&VMStatus);
                continue;
            }
            sp[0] /= sp[-1];
            break;

        case emOP_Mod_i:
            ++sp;
            if (0 == sp[-1])
            {
                VM_Error(emVEI_Runtime_Error_Divided_By_Zero);
                VM_Abort(&VMStatus);
                continue;
            }
            sp[0] %= sp[-1];
            break;

        case emOP_And_i:
            ++sp;
            sp[0] &= sp[-1];
            break;

        case emOP_Xor_i:
            ++sp;
            sp[0] ^= sp[-1];
            break;

        case emOP_Or_i:
            ++sp;
            sp[0] |= sp[-1];
            break;

        case emOP_Shl_i:
            ++sp;
            sp[0] <<= sp[-1];
            break;

        case emOP_Shr_i:
            ++sp;
            sp[0] >>= sp[-1];
            break;

        case emOP_Neg_i:
            sp[0] = -sp[0];
            break;

        case emOP_Not_i:
            sp[0] = ~sp[0];
            break;

        case emOP_Cvt_b_i:
            sp[0] = (CHAR)sp[0];
            break;

        case emOP_Cvt_b_w:
            sp[0] = (SHORT)sp[0];
            break;

        case emOP_CmpEq_i:
            ++sp;
            sp[0] = sp[0] == sp[-1];
            break;

        case emOP_CmpNe_i:
            ++sp;
            sp[0] = sp[0] != sp[-1];
            break;

        case emOP_CmpLt_i:
            ++sp;
            sp[0] = sp[0] < sp[-1];
            break;

        case emOP_CmpGt_i:
            ++sp;
            sp[0] = sp[0] > sp[-1];
            break;

        case emOP_CmpLe_i:
            ++sp;
            sp[0] = sp[0] <= sp[-1];
            break;

        case emOP_CmpGe_i:
            ++sp;
            sp[0] = sp[0] >= sp[-1];
            break;

        case emOP_Jmp:
            a = get_a();
            pc = a;
            break;

        case emOP_Je:
            a = get_a();
            if (sp[0] == 0)
                pc = a;
            ++sp;
            break;

        case emOP_Jne:
            a = get_a();
            if (sp[0] != 0)
                pc = a;
            ++sp;
            break;

        case emOP_Libcall:
            a = get_a();        // function index
            --sp;
            LibCall(bp, sp, a);
            b = sp[0];          // return value
            ++sp;
            sp[0] = b;          // return value
            break;

        case emOP_Call:
            b = sp[0];          // li_i XXXXXXXX, means function's offset
            a = get_a();        // get param-size
            Push_i(pc);         // return pc
            Push_i((INT)bp);    // bp
            Push_i(a);          // param-size
            bp = sp;            // mov ebp, esp
            pc = b;             // set pc to new position
            break;

        case emOP_Ret:
            a = sp[0];          // return value
            sp = bp;            // mov esp, ebp
            b = Pop_i();        // param-size
            bp = (PINT)Pop_i(); // pop ebp
            pc = Pop_i();       // restore return position
            sp = (PINT)((INT)sp + b);   // eat up param's space
            sp[0] = a;          // return value
            break;

        case emOP_Li_i:
            a = get_a();
            Push_i(a);
            break;

        case emOP_Libp_i:
            a = get_a();
            Push_i((INT)bp - a);
            break;

        case emOP_Ld_i:
            sp[0] = *((INT *)sp[0]);
            break;

        case emOP_Ld_b:
            sp[0] = *((CHAR *)sp[0]);
            break;

        case emOP_St_i:
            *((INT *)sp[0]) = sp[1];
            ++sp;
            break;

        case emOP_St_b:
            *((BYTE *)sp[0]) = sp[1] & 0xff;
            ++sp;
            break;

        case emOP_Libp_data:
            sp[0] = (INT)&pbyData[sp[0]];
            break;

        default:
            break;
        } // switch
    } // while

    return VMStatus;
}