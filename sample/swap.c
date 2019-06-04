#include "lclib.h"

/*
 * ÑÝÊ¾swap²Ù×÷
 */

int main()
{
    int a, b;

    a = 1;
    b = 2;

    printnum(a);
    printstr("\n");
    printnum(b);
    printstr("\n");

    //
    // swap
    //
    a ^= b;
    b ^= a;
    a ^= b;

    printnum(a);
    printstr("\n");
    printnum(b);
    printstr("\n");

    return 0;
}
