////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   lib.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-8-25 23:26:12
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lib.h"

VOID LibCall(PINT bp, PINT sp, INT LibCallIdx)
{
    switch (LibCallIdx)
    {
    case emLCI_printnum:
        lib_printnum(sp[1]);
        break;

    case emLCI_printstr:
        lib_printstr((CHAR *)sp[1]);
        break;

    case emLCI_strcpy:
        sp[0] = (INT)lib_strcpy((CHAR *)sp[1], (CHAR *)sp[2]);
        break;

    case emLCI_strcat:
        sp[0] = (INT)lib_strcat((CHAR *)sp[1], (CHAR *)sp[2]);
        break;

    case emLCI_strlen:
        sp[0] = (INT)lib_strlen((CHAR *)sp[1]);
        break;

    case emLCI_DeleteFile:
        sp[0] = (BOOL)lib_DeleteFile((CHAR *)sp[1]);
        break;
    }
}

VOID lib_printnum(INT a)
{
    printf("%d", a);
}

VOID lib_println(VOID)
{
    printf("\n");
}

VOID lib_printstr(CONST CHAR *string)
{
    printf("%s", string);
}

CHAR *lib_strcpy(CHAR *strDestination, CONST CHAR *strSource)
{
    return strcpy(strDestination, strSource);
}

CHAR *lib_strcat(CHAR *strDestination, CONST CHAR *strSource)
{
    return strcat(strDestination, strSource);
}

INT lib_strlen(CONST CHAR *string)
{
    return strlen(string);
}

BOOL lib_DeleteFile(CONST CHAR *lpFileName)
{
    return DeleteFile(lpFileName);
}