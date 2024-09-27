/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180
wangchengyun 	wwx412654
*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif
 
void OpenLog(void)
{
    g_global.isLogOpen = 1;
}

void CloseLog(void)
{
    g_global.isLogOpen = 0;
}

// 一般在调试代码中，调用此接口，许多参数用默认的，不需要了解太多细节
void DebugElement(SElement *pElement)
{
    int i;

    hw_printf("\n---------------------------------DEBUG	SElement\n");
    hw_printf("pElement->para.type is		 	 %d\n", pElement->para.type);
    hw_printf("pElement->isHasInitValue is		 %d\n", pElement->isHasInitValue);
    hw_printf("pElement->isNeedFree is		 	 %d\n", pElement->isNeedFree);
    hw_printf("pElement->isAddWholeRandom is		 %d\n", pElement->isAddWholeRandom);
    hw_printf("pElement->inLen is			 %d\n", (int)pElement->inLen);

    if (pElement->inBuf != NULL)
    {
        hw_printf("pElement->inBuf is:		\n");
        HexDump((u8 *)(pElement->inBuf), (u32)(pElement->inLen / 8)+ IS_ADD_ONE(pElement->inLen % 8));
    }

    hw_printf("pElement->pos is		 	 %d\n", pElement->pos);
    hw_printf("pElement->count is		 	 %d\n", pElement->count);
    hw_printf("pElement->para.len is		 	 %d\n", (int)pElement->para.len);
    hw_printf("pElement->isNeedFreeOutBuf is		 %d\n", pElement->isNeedFreeOutBuf);	

    if (pElement->para.value != NULL)
    {
        hw_printf("pElement->para.value is:		\n");
        HexDump((u8 *)(pElement->para.value), (u32)(pElement->para.len / 8)+ IS_ADD_ONE(pElement->inLen % 8));
    }

    for (i = 0; i < enum_MutatedMAX; i++)
    {
        if (g_global.mutaterGroup[i] == NULL)
        {
            continue;
        }

        hw_printf("[%d] Mutator: %s\n", i, g_global.mutaterGroup[i]->name);
        hw_printf("isMutatedClose %d isMutatedSupport %d posStart %d num %d\n", 
            pElement->isMutatedClose[i], pElement->isMutatedSupport[i], pElement->posStart[i], pElement->num[i]);
    }
}

void DebugSuportMutator(SElement *pElement)
{
    int i;

    hw_printf("SElement support mutator:\n");
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        if (ENUM_YES == pElement->isMutatedSupport[i])
        {
            hw_printf("\t%s\n", g_global.mutaterGroup[i]->name);
        }
    }

    hw_printf("SElement not support mutator:\n");
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        if (ENUM_NO == pElement->isMutatedSupport[i])
        {
            hw_printf("\t%s\n", g_global.mutaterGroup[i]->name);
        }
    }
}

void DebugClosedMutator(void)
{
    int i;

    hw_printf("closed mutator:\n");
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        if (ENUM_YES == g_global.isMutatedClose[i])
        {
            hw_printf("\t%s\n", g_global.mutaterGroup[i]->name);
        }
    }
}

#ifdef __cplusplus
}
#endif

// 需要增加一个打印，把其他的全局变量打印出来
