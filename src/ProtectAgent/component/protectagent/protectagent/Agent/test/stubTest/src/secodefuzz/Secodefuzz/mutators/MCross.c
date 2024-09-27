/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					样本之间，参数之间交换数据
						这个变异算法调用到的外部函数较多，比较危险

长度:					

数量:					

支持数据类型: 

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

 int CrossGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT;
}

char* CrossGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);

    // 找一个样本
    int corpusPos = CorpusSelect();

    // 找一个参数
    int paraPos =  RAND_RANGE(0, InGetParaNum() - 1);

    // 得到值和长度
    char* buf;
    int len;
    char* buf1;
    int len1;

    CorpusParaValueGet(corpusPos, paraPos, &buf, &len);

    // 找到要替换的值和长度
    int temp_pos = RAND_RANGE(0, len);
    buf1 = buf + temp_pos;
    len1 = RAND_RANGE(0, len - temp_pos);
    

     int temp = RAND_32() % 100;
     int isChangeLength = InGetTypeIsChangeLength(pElement->para.type);
    
    // 百分之30的概率替换，在所有长度，长度不变
    if ((temp < 40) || (isChangeLength == ENUM_NO))
    {
        MagicGetValue(pElement, buf1, len1, ENUM_CHANGE);
    }
    // 完整替换，可变长度
    else if ((temp < 70) && (isChangeLength == ENUM_YES))
    {
        MagicGetValue(pElement, buf1, len1, ENUM_REPLACE);
    }
    // 中间插入
    else if (isChangeLength == ENUM_YES)
    {
        MagicGetValue(pElement, buf1, len1, ENUM_INSERT);
    }
    
    return pElement->para.value;
}

int CrossGetIsSupport(SElement *pElement)
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

    return ENUM_YES;
}

const struct MutaterGroup g_crossGroup = {
    "Cross",
    CrossGetCount,
    CrossGetValue,
    CrossGetIsSupport,
    1
};

void InitCross(void)
{
    RegisterMutater(&g_crossGroup, ENUM_CROSS);
}

#ifdef __cplusplus
}
#endif