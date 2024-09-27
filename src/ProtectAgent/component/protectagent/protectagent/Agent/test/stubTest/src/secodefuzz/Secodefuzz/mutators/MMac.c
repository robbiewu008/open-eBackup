/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					mac��������ר�б����㷨
						

����:					���Ȳ���		,��ֵΪ6byte

����:					n��

֧����������: 	mac

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static u8 g_macTable[][6] = {
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x00},
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01},
    {0x01, 0x00, 0x5e, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

 int MacGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return sizeof(g_macTable) / 6 + 8;
}

char* MacGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    int i;

    SetElementInitoutBufEx(pElement, 6);

    HwMemcpy(pElement->para.value, pElement->inBuf, 6);

    // ��λ����bit����
    if (pos == 0)
    {
        FILL_BIT(pElement->para.value, 0);
        FILL_BIT(pElement->para.value, 1);
    }

    if (pos == 1)
    {
        FILL_BIT(pElement->para.value, 0);
        ZERO_BIT(pElement->para.value, 1);
    }

    if (pos == 2)
    {
        ZERO_BIT(pElement->para.value, 0);
        FILL_BIT(pElement->para.value, 1);
    }

    if (pos == 3)
    {
        ZERO_BIT(pElement->para.value, 0);
        ZERO_BIT(pElement->para.value, 1);
    }

    // ��֯��ʶ������
    if (pos == 4)
    {
        for (i = 2; i <= 23; i++)
        {
            FILL_BIT(pElement->para.value, i);
        }
    }

    if (pos == 5)
    {
        for (i = 2; i <= 23; i++)
        {
            ZERO_BIT(pElement->para.value, i);
        }
    }

    // ����id����
    if (pos == 6)
    {
        for (i = 24; i <= 47; i++)
        {
            FILL_BIT(pElement->para.value, i);
        }
    }

    if (pos == 7)
    {
        for (i = 24; i <= 47; i++)
        {
            ZERO_BIT(pElement->para.value, i);
        }
    }
    if (pos >= 8)
    {
        // ip�ಥ
        pos = pos - 8;
        for (i = 0; i < 6; i++)
        {
            pElement->para.value[i] = g_macTable[pos][i];
        }
    }

    // �������������mac��ַ����������

    return pElement->para.value;
}

int MacGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_MAC)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_macGroup = {
    "Mac",
    MacGetCount,
    MacGetValue,
    MacGetIsSupport,
    1
};

void InitMac(void)
{
    RegisterMutater(&g_macGroup, ENUM_MMAC);
}

#ifdef __cplusplus
}
#endif