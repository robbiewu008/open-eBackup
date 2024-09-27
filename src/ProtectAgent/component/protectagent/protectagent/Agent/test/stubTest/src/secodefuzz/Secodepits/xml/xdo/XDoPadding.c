/*
版权所有 (c) 华为技术有限公司 2012-2018


Padding多用于文件，某个元素实现多少个字节对齐

*/
#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

void DoPadding(SBinElement* temp)
{
    For_Tree_Start(temp)
    {
        SXMLElement* tempXml = temp->mutatorElement->xmlElement;
        if ((tempXml != NULL)
            && (strcmp(tempXml->typeName, "Padding") == 0)
            && (tempXml->alignment ))
        {
            // 目前只关心自己父亲底下的

            // 得到从上边元素到目前的长度
            int length = BinElementGetOffsetLength(temp->parent, temp);

            int alignment = atol(tempXml->alignment) / 8;

            temp->mutaterLength = (alignment - length % alignment) % alignment;
            temp->mutaterValue = malloc(temp->mutaterLength);
            g_onerunMemory[g_onerunMemoryCount++] = temp->mutaterValue;
            Hw1Memset(temp->mutaterValue, 0, temp->mutaterLength);
        }
    }
    For_Tree_End(temp)
}

#ifdef __cplusplus
}
#endif
