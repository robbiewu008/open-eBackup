/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ���ṩ��������ִ�г�ʱbug�Ĺ���
���ṩ���ò���������ʱ�������

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void CallBackAddCrash(void (*fun)(void))
{
    g_global.crashCallBack = fun;
}


void CallBackAddTestCase(void (*fun)(void))
{
    g_global.testCaseCallBack = fun;
}


void CallBackRunCrash(void)
{
    if (g_global.crashCallBack)
    {
        g_global.crashCallBack();
    }
}

void CallBackRunTestCase(void)
{
    if (g_global.testCaseCallBack)
    {
        g_global.testCaseCallBack();
    }
}

#ifdef __cplusplus
}
#endif

