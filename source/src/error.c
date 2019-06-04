////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   error.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-28 17:34:40
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scan.h"
#include "lc.h"

#include <stdarg.h>

INT g_nErrorCount = 0;

static VOID Abort(VOID)
{
    UnInit();

    exit(EXIT_FAILURE);
}

void yyerror(const char *format, ...)
{
    va_list arglist;

    fprintf(stderr, "%s(%d) : ", g_pszCurrentFileName, yylineno);

    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    va_end(arglist);

    fprintf(stderr, "\n");

    ++g_nErrorCount;

    Abort();
}

VOID Error(CONST ERROR_INFO ErrInfo, ...)
{
    va_list arglist;

    fprintf(
        stderr,
        "%s(%d) : error C%04d: ",
        g_pszCurrentFileName, yylineno, ErrInfo
    );

    va_start(arglist, ErrInfo);
    vfprintf(stderr, g_pszErrorInfo[ErrInfo], arglist);
    va_end(arglist);

    fprintf(stderr, "\n");

    ++g_nErrorCount;

    Abort();
}

VOID ErrorWithLineNo(CONST ERROR_INFO ErrInfo, CONST INT nLineNo, ...)
{
    va_list arglist;

    fprintf(
        stderr,
        "%s(%d) : error C%04d: ",
        g_pszCurrentFileName, nLineNo, ErrInfo
    );

    va_start(arglist, nLineNo);
    vfprintf(stderr, g_pszErrorInfo[ErrInfo], arglist);
    va_end(arglist);

    fprintf(stderr, "\n");

    ++g_nErrorCount;

    Abort();
}