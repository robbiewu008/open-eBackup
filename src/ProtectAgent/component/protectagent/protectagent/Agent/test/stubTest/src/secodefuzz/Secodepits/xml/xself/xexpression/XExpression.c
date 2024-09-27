/*
版权所有 (c) 华为技术有限公司 2012-2018


和relation相关的expression的函数
*/

#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

int function_AAAA(int size)
{
    return size - 2;
}

int function_BBBB(int size)
{
    return size + 2;
}

int function_mp4get(int size)
{
    return size - 16;
}

int function_mp4set(int size)
{
    return size + 16;
}

int function_mp4get1(int size)
{
    return size - 8;
}

int function_mp4set1(int size)
{
    return size + 8;
}

int function_IIIIG(int size)
{
    return size * 4;
}

int function_IIIIS(int size)
{
    return size / 4;
}

int function_IPV4_get(int size)
{
    return size * 4;
}

int function_IPV4_set(int size)
{
    return size / 4;
}

int function_get_mul10(int size)
{
    return size / 10;
}

int function_set_mul10(int size)
{
    return size * 10;
}

#ifdef __cplusplus
}
#endif
