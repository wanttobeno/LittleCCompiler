////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   lib.h
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-8-25 23:25:49
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __LIB_H__
#define __LIB_H__

////////////////////////////////////////////////////////////////////////////////
//
// Lib-call define
//
typedef enum _LIB_CALL_INDEX
{
    emLCI_printnum = 0,
    emLCI_printstr,
    emLCI_strcpy,
    emLCI_strcat,
    emLCI_strlen,
    emLCI_DeleteFile,
    emLCI_MAX_COUNT,
} LIB_CALL_INDEX;

typedef struct _LIB_CALL_ITEM
{
    LIB_CALL_INDEX FuncIdx;
    PCHAR pszFuncName;
} LIB_CALL_ITEM, *PLIB_CALL_ITEM;

static CONST LIB_CALL_ITEM g_LibCall[] =
{
    { emLCI_printnum,   "printnum"  },
    { emLCI_printstr,   "printstr"  },
    { emLCI_strcpy,     "strcpy"    },
    { emLCI_strcat,     "strcat"    },
    { emLCI_strlen,     "strlen"    },
    { emLCI_DeleteFile, "DeleteFile"},
};


////////////////////////////////////////////////////////////////////////////////
// Lib-call function
//

VOID LibCall(PINT bp, PINT sp, INT LibCallIdx);

VOID lib_printnum(INT a);
VOID lib_println(VOID);
VOID lib_printstr(CONST CHAR *string);
CHAR *lib_strcpy(CHAR *strDestination, CONST CHAR *strSource);
CHAR *lib_strcat(CHAR *strDestination, CONST CHAR *strSource);
INT lib_strlen(CONST CHAR *string);
BOOL lib_DeleteFile(CONST CHAR *lpFileName);

#endif  // __LIB_H__