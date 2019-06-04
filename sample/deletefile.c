#include "lclib.h"

void main()
{
    int nRetCode = DeleteFile("1.txt");
    if (nRetCode)
        printstr("Delete file succeed!\n");
    else
        printstr("Delete file failed!\n");
}
