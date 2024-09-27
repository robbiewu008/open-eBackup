/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					利用hook来源数据替换

长度:					长度不变

数量:					MAX_COUNT*2

支持数据类型: 	整数类型
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberMagicGetCount(SElement *pElement)
{
    if (LlvmDataNumberGetCount() == 0)
    {
        return 0;
    }
    
    return MAX_COUNT * 2;
}

char* NumberMagicGetValue(SElement *pElement, int pos)
{
    int len;

    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);

        s64 temp = LlvmDataNumberGetValue();

        //百分之10的概率改为负数
        if (RAND_32() % 10 == 4)
            temp = temp * -1;

        Inltoa(temp, pElement->para.value, 10);

        len = Instrlen(pElement->para.value) + 1;
        if (len > pElement->para.maxLen)
        {
            len = pElement->para.maxLen;
        }

        pElement->para.value[len - 1] = 0;

        // 重置长度为字符串实际长度
        pElement->para.len = len;
        
        return pElement->para.value;
    }
    
    // 如果bit长度不被8整除，则加1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    u64 temp = LlvmDataNumberGetValue();
    u16 temp1;
    u32 temp2;
    u64 temp3;

    if (pElement->para.type == ENUM_NUMBER_S)
    {
        switch (len)
        {
            case 1:
                *((s8  *)pElement->para.value) = temp;
                break;
            case 2:
                temp1 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp1 = InBswap16(temp1);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp1 = temp1 - RAND_RANGE(-50, 50);
                }
        
                *((s16 *)pElement->para.value) = temp1;
                break;
            case 4:
                temp2 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp2 = InBswap32(temp2);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp2 = temp2 - RAND_RANGE(-50, 50);
                }
                
                *((s32 *)pElement->para.value) = temp2;
                break;
            case 8:
                temp3 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp3 = InBswap64(temp3);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp3 = temp3 - RAND_RANGE(-50, 50);
                }
                
                *((s64 *)pElement->para.value) = temp3;
                break;
            default:  ;
        }
    }
    else
    {
        switch (len)
        {
            case 1:
                *((u8  *)pElement->para.value) = temp;
                break;
            case 2:
                temp1 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp1 = InBswap16(temp1);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp1 = temp1 + RAND_RANGE(-50, 50);
                }
                
                *((u16 *)pElement->para.value) = temp;
                break;
            case 4:
                temp2 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp2 = InBswap32(temp2);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp2 = temp2 - RAND_RANGE(-50, 50);
                }

                *((u32 *)pElement->para.value) = temp2;
                break;
            case 8:
                temp3 = temp;
                if (RAND_32() % 10 == 0)
                {
                    temp3 = InBswap64(temp3);
                }

                if (RAND_32() % 10 == 1)
                {
                    temp3 = temp3 - RAND_RANGE(-50, 50);
                }
                
                *((u64 *)pElement->para.value) = temp3;
                break;
            default:  ;
        }
    }

    return pElement->para.value;
}

int NumberMagicGetIsSupport(SElement *pElement)
{
    // 先不支持字符串，以后再加
    if ((pElement->para.type == ENUM_NUMBER_U)
        || (pElement->para.type == ENUM_NUMBER_S))
    {
        return ENUM_YES;
    }

    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberMagicGroup = {
    "NumberMagic",
    NumberMagicGetCount,
    NumberMagicGetValue,
    NumberMagicGetIsSupport,
    1
};

void InitNumberMagic(void)
{
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_numberMagicGroup, ENUM_NUMBER_MAGIC);
}

#ifdef __cplusplus
}
#endif