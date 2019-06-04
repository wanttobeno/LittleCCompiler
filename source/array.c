#include "lclib.h"

// 演示数组的操作

int main()
{
    int i, a[10];

    for (i = 0; i < 10; ++i)
    {
        a[i] = i + 1;
        printnum(a[i] - 1);
        printstr("\n");
    }

    return 0;
}
