#include "lclib.h"

// 最大公约数
int gcd(int a, int b)
{
    // if b equals 0, return a
    // else return gcd(b,a%b)
    return b ? gcd(b, a % b) : a;
}

// 最小公倍数
int lcm(int a, int b)
{
    return a * b / gcd(a, b);
}

void main()
{
    // 求两个数的最大公约数
    printnum(gcd(12, 20));
    printstr("\n");

    // 求两个数的最小公倍数
    printnum(lcm(8, 17));
    printstr("\n");
}
