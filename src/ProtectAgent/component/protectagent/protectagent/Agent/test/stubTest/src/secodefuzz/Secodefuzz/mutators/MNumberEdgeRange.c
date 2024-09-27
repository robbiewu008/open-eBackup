/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����ʱֵ�ܱ߱���
						

����:					���Ȳ���	(�ַ����Ļ����33 ��������)	

����:					100

֧����������: 	�����������ֳ�ʼֵ���ַ���

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberEdgeRangeGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT / 5;
}

char* NumberEdgeRangeGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);

        s64 temp;

        int pos1 = RAND_RANGE(0, 64 * 4 - 1);

        if (pos1 < 64 * 2)
        {
            temp = 0 + (s64)g_edgeCaseTable[pos1];
        }
        else
        {
            temp = 0 - (s64)g_edgeCaseTable[pos1 - 64 * 2];
        }

        int pos2 = RAND_RANGE(-50, 50);

        temp = temp + pos2;

        Inltoa(temp, pElement->para.value, 10);

        len = Instrlen(pElement->para.value) + 1;
        if (len > pElement->para.maxLen)
        {
            len = pElement->para.maxLen;
        }

        pElement->para.value[len - 1] = 0;

        // ���ó���Ϊ�ַ���ʵ�ʳ���
        pElement->para.len = len;
        
        return pElement->para.value;
    }
    
    // ���bit���Ȳ���8���������1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    int pos1 = RAND_RANGE(0, pElement->inLen * 4 - 1);

    int pos2 = RAND_RANGE(-50, 50);

    if (pos1 < pElement->inLen * 2)
    {
        // ���޷��ŷֱ�Դ�
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = pos2 + (s8)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = pos2 + (s16)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = pos2 + (s32)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = pos2 + (s64)g_edgeCaseTable[pos1];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = pos2 + (u8)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = pos2 + (u16)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = pos2 + (u32)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = pos2 + (u64)g_edgeCaseTable[pos1];
            }
        }
    }
    else
    {
        pos1 = pos1 - pElement->inLen * 2;
        // ���޷��ŷֱ�Դ�
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = pos2 - (s8)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = pos2 - (s16)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = pos2 - (s32)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = pos2 - (s64)g_edgeCaseTable[pos1];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = pos2 - (u8)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = pos2 - (u16)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = pos2 - (u32)g_edgeCaseTable[pos1];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = pos2 - (u64)g_edgeCaseTable[pos1];
            }
        }
    }

    return pElement->para.value;
}

int NumberEdgeRangeGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if ((pElement->para.type == ENUM_NUMBER_S) 
        || (pElement->para.type == ENUM_NUMBER_U))
    {
        return ENUM_YES;
    }

    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberEdgeRangeGroup = {
    "NumberEdgeRange",
    NumberEdgeRangeGetCount,
    NumberEdgeRangeGetValue,
    NumberEdgeRangeGetIsSupport,
    1
};

void InitNumberEdgeRange(void)
{
    RegisterMutater(&g_numberEdgeRangeGroup, ENUM_NUMBER_EDGE_RANGE);
}

#ifdef __cplusplus
}
#endif