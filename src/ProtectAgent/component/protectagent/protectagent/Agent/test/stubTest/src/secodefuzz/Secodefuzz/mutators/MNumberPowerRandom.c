/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					��ȡ�Գ�ʼֵΪ�������ܱ���չ�ı��죬
						����ֵΪ��ʼֵ�Ӽ�����bit�ٽ�ֵ
						

����:					���Ȳ���			(�ַ����Ļ����33 ��������)	

����:					bit����2 *2

֧����������: 	�г�ʼֵ�������������ֳ�ʼֵ���ַ���

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberPowerRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // �ַ������ְ���s64��ͬ
    if (pElement->para.type == ENUM_STRING)
    {
        return 64 * 2 * 2;
    }

    return pElement->inLen * 2 * 2;
}

char* NumberPowerRandomGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);

        s64 temp;

        if (pos < 64 * 2)
        {
            temp = *(s64 *)pElement->numberValue + (s64)g_edgeCaseTable[pos];
        }
        else
        {
            temp = *(s64 *)pElement->numberValue - (s64)g_edgeCaseTable[pos - 64 * 2];
        }

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

    if (pos < pElement->inLen * 2)
    {
        // ���޷��ŷֱ�Դ�
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = *(s8  *)pElement->numberValue + (s8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = *(s16  *)pElement->numberValue + (s16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = *(s32  *)pElement->numberValue + (s32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = *(s64  *)pElement->numberValue + (s64)g_edgeCaseTable[pos];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = *(u8  *)pElement->numberValue + (u8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = *(u16  *)pElement->numberValue + (u16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = *(u32  *)pElement->numberValue + (u32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = *(u64  *)pElement->numberValue + (u64)g_edgeCaseTable[pos];
            }
        }
    }
    else
    {
        pos = pos - pElement->inLen * 2;
        // ���޷��ŷֱ�Դ�
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = *(s8 *)pElement->numberValue - (s8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = *(s16 *)pElement->numberValue - (s16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = *(s32 *)pElement->numberValue - (s32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = *(s64 *)pElement->numberValue - (s64)g_edgeCaseTable[pos];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = *(u8 *)pElement->numberValue  - (u8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = *(u16 *)pElement->numberValue  - (u16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = *(u32 *)pElement->numberValue  - (u32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = *(u64 *)pElement->numberValue  - (u64)g_edgeCaseTable[pos];
            }
        }
    }

    return pElement->para.value;
}

int NumberPowerRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ��֧���޳�ʼֵ�����
    if (pElement->isHasInitValue != ENUM_YES)
    {
        return ENUM_NO;
    }
    
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

const struct MutaterGroup g_numberPowerRandomGroup = {
    "NumberPowerRandom",
    NumberPowerRandomGetCount,
    NumberPowerRandomGetValue,
    NumberPowerRandomGetIsSupport,
    1
};

void InitNumberPowerRandom(void)
{
    RegisterMutater(&g_numberPowerRandomGroup, ENUM_NUMBER_POWER_RANDOM);
}

#ifdef __cplusplus
}
#endif