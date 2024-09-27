/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


Padding�������ļ���ĳ��Ԫ��ʵ�ֶ��ٸ��ֽڶ���

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
            // Ŀǰֻ�����Լ����׵��µ�

            // �õ����ϱ�Ԫ�ص�Ŀǰ�ĳ���
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
