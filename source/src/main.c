////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   main.c
//  Version     :   1.0
//  Creator     :   Luo Cong
//  Date        :   2008-6-27 15:27:07
//  Comment     :   
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scan.h"
#include "lc.h"
#include "config.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#ifndef defVM_STACK_DEFAULT_SIZE
#define defVM_STACK_DEFAULT_SIZE 8192
#endif

PCHAR g_pszCurrentFileName = NULL;
INT g_nStack[defVM_STACK_DEFAULT_SIZE];

static VOID Usage(CONST CHAR cszExecFileName[])
{
    printf(
        "%s, Version %d.%d\n"
        "Copyleft (C) %s. Last built on " __DATE__ ", " __TIME__ "\n"
        "Virtual Machine's stack size is %d\n\n"
        "usage: %s source_filename [-p]\n\n"
        "Options:\n"
        "  -p : print virtual machine opcode\n",
        COMPILER_NAME, VERSION_HI, VERSION_LO, AUTHOR,
        defVM_STACK_DEFAULT_SIZE,
        cszExecFileName
    );
}

VOID Init(VOID)
{
    Tree_Init();

    Gen_Init();

    Block_Init();

    Gen_EnterGlobalFrame();
}

VOID UnInit(VOID)
{
    //
    // destroy buffer that created by "#include"
    //
    while (g_nInclCnt-- > 0)
    {
        yy_delete_buffer((YY_BUFFER_STATE)g_Incl[g_nInclCnt].prev_yybuf);
        fclose(g_Incl[g_nInclCnt].prev_fp);
    }

    //
    // yylex uninit
    //
    yylex_destroy();

    Gen_LeaveGlobalFrame();

    Block_UnInit();

    Gen_UnInit();

    Tree_UnInit();
}

int main(int argc, char *argv[])
{
    INT i;
    INT nFileNameIndex;
    BOOL bPrintCode = FALSE;
    INT nRetResult = EXIT_FAILURE;
    VM_STATUS VMStatus;

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (argc < 2 || argc > 3)
    {
        Usage(argv[0]);
        goto Exit0;
    }

    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (0 == strnicmp(argv[i], "-p", sizeof("-p")))
                bPrintCode = TRUE;
        }
        else
            nFileNameIndex = i;
    }

    g_pszCurrentFileName = argv[nFileNameIndex];

    Init();

    yyin = fopen(argv[nFileNameIndex], "r");
    if (NULL == yyin)
        Error(emEI_Cannot_open_file, argv[nFileNameIndex]);

    yyparse();

    Gen_SetEntryPoint();

    if (0 == g_nErrorCount)
    {
        if (bPrintCode)
            Gen_PrintCode();

        VMStatus = VM_Interpret(
            g_nCodeSize, g_pbyCode,
            g_nDataSize, g_pbyData,
            sizeof(g_nStack) / sizeof(*g_nStack), g_nStack
        );

        if (VMStatus == emVS_Halt)
            // Successfully halt
            printf("-----\nVM executed successfully.\n");
        else
            printf("-----\nVM running failed.\n");

        printf(
            "-----\n"
            "Code size : %d bytes\nData size : %d bytes\nTotal size: %d bytes\n"
            "-----\n",
            g_nCodeSize, g_nDataSize, g_nCodeSize + g_nDataSize
        );
    }

    UnInit();

    nRetResult = EXIT_SUCCESS;
Exit0:
    if (yyin)
    {
        fclose(yyin);
        yyin = NULL;
    }
    return nRetResult;
}