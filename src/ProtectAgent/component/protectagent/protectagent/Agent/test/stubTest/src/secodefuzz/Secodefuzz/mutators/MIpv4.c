/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					ipv4数据类型专有变异算法
						

长度:					长度不变		

数量:					n个

支持数据类型: 	ipv4

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static u8 g_ipv4Table[][4] = {
    {0, 0, 0, 0},
    {0, 0, 0, 255},
    {0, 0, 255, 255},
    {0, 255, 255, 255},
    {255, 255, 255, 255},
    {255, 255, 255, 0},
    {255, 255, 0, 0},
    {255, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 1},
    {1, 0, 0, 255},
    {1, 255, 255, 255},
    {128, 0, 0, 0},
    {128, 0, 0, 1},
    {128, 0, 0, 255},
    {128, 0, 255, 255},
    {192, 0, 0, 0},
    {192, 0, 0, 1},
    {192, 0, 0, 255},
    {224, 0, 0, 0},
    {224, 0, 0, 1},
    {224, 0, 0, 255},
    {127, 0, 0, 1},
};

static u8 g_ipv4Table2[][4] = {
    {255, 255, 255, 0},
    {255, 255, 0, 0},
    {255, 0, 0, 0},
};

static u8 g_ipv4Table3[][4] = {
    {0, 0, 0, 255},
    {0, 0, 255, 255},
    {0, 255, 255, 255},
};

int Ipv4GetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return sizeof(g_ipv4Table) / 4 + sizeof(g_ipv4Table2) / 4 + sizeof(g_ipv4Table3) / 4;
}

char* Ipv4GetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    int pos1 = sizeof(g_ipv4Table) / 4;
    int pos2 = sizeof(g_ipv4Table2) / 4 + pos1;
    int pos3 = sizeof(g_ipv4Table3) / 4 + pos2;

    SetElementInitoutBufEx(pElement, 4);

    if (pos < pos1)
    {
        *((u32 *)pElement->para.value) = *(u32 *)g_ipv4Table[pos];
    }
    else if (pos < pos2)
    {
        *((u32 *)pElement->para.value) = (*(u32 *)g_ipv4Table2[pos - pos1]) & (*((u32 *)pElement->inBuf));
    }
    else if (pos < pos3)
    {
        *((u32 *)pElement->para.value) = (*(u32 *)g_ipv4Table3[pos - pos2]) | (*((u32 *)pElement->inBuf));
    }

    return pElement->para.value;
}


int Ipv4GetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_IPV4)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_ipv4Group = {
    "Ipv4",
    Ipv4GetCount,
    Ipv4GetValue,
    Ipv4GetIsSupport,
    1
};

void InitIpv4(void)
{
    RegisterMutater(&g_ipv4Group, ENUM_MIIP4);
}

#ifdef __cplusplus
}
#endif

