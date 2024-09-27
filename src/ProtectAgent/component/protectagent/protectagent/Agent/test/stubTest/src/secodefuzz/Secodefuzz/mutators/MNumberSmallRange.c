/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					��ȡ��ʼֵ��Χ�Ӽ�50��ֵ,��100�����������������������ת��

����:					���Ȳ���			(�ַ����Ļ����33 ��������)	

����:					100

֧����������: 	�����������ֳ�ʼֵ���ַ���

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberSmallRangeGetCount(SElement *pElement)
{
    // ��ϸ���룬����������Ƿ�ת������
    return MAX_COUNT / 5;
}

char* NumberSmallRangeGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);
        
        s64 temp = (*(s64  *)pElement->numberValue + pos - 50);
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

    // ���޷��ŷֱ�Դ�
    if (pElement->para.type == ENUM_NUMBER_S)
    {
        if (pElement->inLen <= 8)
        {
            *((s8 *)pElement->para.value)  = *(s8 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 16)
        {
            *((s16 *)pElement->para.value) = *(s16 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 32)
        {
            *((s32 *)pElement->para.value) = *(s32 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 64)
        {
            *((s64 *)pElement->para.value) = *(s64 *)pElement->numberValue + pos - 50;
        }
    }
    else if (pElement->para.type == ENUM_NUMBER_U)
    {
        if (pElement->inLen <= 8)
        {
            *((u8 *)pElement->para.value)  = *(u8 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 16)
        {
            *((u16 *)pElement->para.value) = *(u16 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 32)
        {
            *((u32 *)pElement->para.value) = *(u32 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 64)
        {
            *((u64 *)pElement->para.value) = *(u64 *)pElement->numberValue + pos - 50;
        }
    }
    
    return pElement->para.value;
}

int NumberSmallRangeGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_numberSmallRangeGroup = {
    "NumberSmallRange",
    NumberSmallRangeGetCount,
    NumberSmallRangeGetValue,
    NumberSmallRangeGetIsSupport,
    1
};

void InitNumberSmallRange(void)
{
    RegisterMutater(&g_numberSmallRangeGroup, ENUM_NUMBER_SMALL_RANGE);
}

#ifdef __cplusplus
}
#endif

