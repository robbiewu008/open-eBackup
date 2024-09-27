/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:						常见容易引起问题的字符串，被插入，覆盖，替换元素
							只要在表中填入数据，你的数据就会出现在测试例里						

长度:						0到最大长度之间

数量:						MAX_COUNT

支持数据类型: 		string,Blob,FixBlob


*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementStringStaticGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    int count = g_stringStaticTableLen;
    if (count == 0)
    {
        return 0;
    }
    
    return MAX_COUNT;
}

char* DataElementStringStaticGetValue(SElement *pElement, int pos)
{
    int count = g_stringStaticTableLen;
    int llvmLen;

    ASSERT_NULL(pElement);

    // 库没有值，则空
    if (count == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // 库里随机抽取一个
    size_t idx  = RAND_32() % count;

    llvmLen = Instrlen(g_stringStaticTable[idx]);
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // 一半加上/0
    if (RAND_32() % 2)
    {
        llvmLen = llvmLen + 1;
    }
    
    int isInsert = RAND_32() % 3;
    
    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, (char*)g_stringStaticTable[idx], llvmLen, isInsert);

    return pElement->para.value;
}

int DataElementStringStaticGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 对于fixblob,因为算法不会减小长度，对最大长度还有判断，所以可以支持
    if ((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementStringStaticGroup = {
    "DataElementStringStatic",
    DataElementStringStaticGetCount,
    DataElementStringStaticGetValue,
    DataElementStringStaticGetIsSupport,
    1
};

void InitDataElementStringStatic(void)
{
    RegisterMutater(&g_dataElementStringStaticGroup, ENUM_DATAELEMENT_STRING_STATIC);
}

#ifdef __cplusplus
}
#endif

