/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					利用hook来源数据替换

长度:					

数量:					MAX_COUNT*2

支持数据类型: 	可变长数据类型
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementMagicChangeGetCount(SElement *pElement)
{
    if ((LlvmDataMemGetCount() == 0) && (LlvmDataNumberGetCount() == 0))
    {
        return 0;
    }
    
    return MAX_COUNT;
}

char* DataElementMagicChangeGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char* value =NULL;
    ASSERT_NULL(pElement);

    int temp = RAND_32() % 20;

    if (temp < 6)
    {
        s64 number = LlvmDataNumberGetValue();

        //库没有值
        if (number == 0)
        {
            return SetElementOriginalValue(pElement);
        }
        
        value = (char*)(&number);
        llvmLen = 8;

        // 找到起始位置
        int start = RAND_RANGE(0, llvmLen);
        // 找到替换的长度
        int len = RAND_RANGE(0, llvmLen - start);

        // 0 Insert 1 Overwrite 2 replace
        MagicGetValue(pElement, value + start, len, ENUM_CHANGE);
    }
    else
    {
        // \0增加的逻辑在函数里边
        value = LlvmDataMemGetValue(&llvmLen);

        //库没有值，或者就一个字节
        if (llvmLen <= 1)
        {
            return SetElementOriginalValue(pElement);
        }

        // 找到起始位置
        int start = RAND_RANGE(0, llvmLen);
        // 找到替换的长度
        int len = RAND_RANGE(0, llvmLen - start);

        // 0 Insert 1 Overwrite 2 replace 3 change
        MagicGetValue(pElement, value + start, len, ENUM_CHANGE);
    }

    return (char *)pElement->para.value;
}

int DataElementMagicChangeGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 枚举不支持
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self变异算法先屏蔽，想要打开自己打
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_NO;
    }
    
    // 只要有初始值，就支持，对所有数据类型开放有意义否?
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }
    
    return ENUM_NO;
}


const struct MutaterGroup g_dataElementMagicChangeGroup = {
    "DataElementMagicChange",
    DataElementMagicChangeGetCount,
    DataElementMagicChangeGetValue,
    DataElementMagicChangeGetIsSupport,
    1
};

void InitDataElementMagicChange(void)
{
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_dataElementMagicChangeGroup, ENUM_DATAELEMENT_MAGIC_CHANGE);
}

#ifdef __cplusplus
}
#endif

