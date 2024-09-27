/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块实现编译器回调函数

实现收集内存数据的功能

如果编译器不支持，需要注释掉HAS_LLVM声明
本模块只在高版本的gcc(6.0)和clang(3.1)以上支持

*/

#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_HOOK

__attribute__((weak)) void __sanitizer_set_death_callback(void (*)(void));
__attribute__((weak)) void __sanitizer_set_nodeath_callback(void (*)(void));
__attribute__((weak)) void __asan_set_error_report_callback(void (*)(char*));
__attribute__((weak)) void __sanitizer_print_stack_trace();


ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmpf(float Arg1, float Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));
    uint32_t a1 = *(uint32_t *)&Arg1;
    uint32_t a2 = *(uint32_t *)&Arg2;

    LlvmDataNumberAddValue(PC, a1, a2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmpd(double Arg1, double Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));
    uint64_t a1 = *(uint64_t *)&Arg1;
    uint64_t a2 = *(uint64_t *)&Arg2;

    LlvmDataNumberAddValue(PC, a1, a2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmp8(uint64_t Arg1, uint64_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_const_cmp8(uint64_t Arg1, uint64_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmp4(uint32_t Arg1, uint32_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_const_cmp4(uint32_t Arg1, uint32_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmp2(uint16_t Arg1, uint16_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_const_cmp2(uint16_t Arg1, uint16_t Arg2) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Arg1, Arg2);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_cmp1(uint8_t Arg1, uint8_t Arg2) 
{
    // 数值小，数量多，靠变异吧
    return;
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_const_cmp1(uint8_t Arg1, uint8_t Arg2) 
{
    // 数值小，数量多，靠变异吧
    return;
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_switch(uint64_t Val, uint64_t *Cases) 
{
    int* PC = (int *)(__builtin_return_address(0));

    uint64_t N = Cases[0];
    uint64_t i;

    for (i = 0; i < N; i++)
    {
        if (i == 0)
        {
            LlvmDataNumberAddValue(PC + i, Cases[i + 2], Val);
        }
        else
        {
            LlvmDataNumberAddValue(PC + i, Cases[i + 2], 0);
        }
    }
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_div4(uint32_t Val)
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Val, 0);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_div8(uint64_t Val) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Val, 0);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
ATT_TARGET_POPCNT
void __sanitizer_cov_trace_gep(uintptr_t Idx) 
{
    int* PC = (int *)(__builtin_return_address(0));

    LlvmDataNumberAddValue(PC, Idx, 0);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_memcmp(void *caller_pc, const void *s1,
    const void *s2, size_t n, int result) 
{
    if (n <= 1) 
    {
        return;
    }
    LlvmDataMemAddValue(caller_pc, (char *)s1, (char *)s2, n, n);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strncmp(void *caller_pc, const char *s1,
    const char *s2, size_t n, int result) 
{                    
    if (n <= 1) 
    {
        return;
    }

    size_t Len1 = 0;
    for (; Len1 < n && s1[Len1]; Len1++) 
    {
    }

    size_t Len2 = 0;
    for (; Len2 < n && s2[Len2]; Len2++) 
    {
    }

    LlvmDataMemAddValue(caller_pc, s1, s2, Len1, Len2);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strcmp(void *caller_pc, const char *s1,
    const char *s2, int result) 
{
    LlvmDataMemAddValueEx(caller_pc, s1, s2);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strncasecmp(void *called_pc, const char *s1,
    const char *s2, size_t n, int result) 
{			
    if (n <= 1) 
    {
        return;
    }
    __sanitizer_weak_hook_strncmp(called_pc, s1, s2, n, result);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strcasecmp(void *called_pc, const char *s1,
    const char *s2, int result) 
{
    LlvmDataMemAddValueEx(called_pc, s1, s2);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strstr(void *called_pc, const char *s1,
    const char *s2, char *result) 
{
    LlvmDataMemAddValueEx(called_pc, s1, s2);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_strcasestr(void *called_pc, const char *s1,
    const char *s2, char *result) 
{
    LlvmDataMemAddValueEx(called_pc, s1, s2);
}

ATTRIBUTE_INTERFACE ATT_NO_SANITIZE_MEMORY
void __sanitizer_weak_hook_memmem(void *called_pc, const void *s1, size_t len1,
    const void *s2, size_t len2, void *result) 
{
    if ((len1 <= 1) && (len2 <=1))
    {
        return;
    }
    
    LlvmDataMemAddValue(called_pc, (char *)s1, (char *)s2, len1, len2);
}

int LlvmHookIsSupport()
{
    return 1;
}

void LlvmHookRegisterAsanCallBack(void (*fun)(void))
{
    if (__sanitizer_set_death_callback)
    {
        __sanitizer_set_death_callback(fun);
    }
}

void LlvmHookRegisterAsanCallBackno(void (*fun)(void))
{
    if (__sanitizer_set_nodeath_callback)
    {
        __sanitizer_set_nodeath_callback(fun);
    }
}

void LlvmHookRegisterAsanCallBackReport(void (*fun)(char *))
{
    if (__asan_set_error_report_callback)
    {
        __asan_set_error_report_callback(fun);
    }
}

void LlvmHookPrintStackTrace(void)
{
    if (__sanitizer_print_stack_trace)
    {
        __sanitizer_print_stack_trace();
    }
}   

#else

int LlvmHookIsSupport()
{
    return 0;
}

void LlvmHookRegisterAsanCallBack(void (*fun)(void))
{
}

void LlvmHookRegisterAsanCallBackno(void (*fun)(void))
{
}

void LlvmHookRegisterAsanCallBackReport(void (* fun)(char *))
{
}

void LlvmHookPrintStackTrace(void)
{
}
#endif

#ifdef __cplusplus
}
#endif