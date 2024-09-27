/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					mac数据类型专有变异算法
						

长度:					长度不变		,定值为6byte

数量:					n个

支持数据类型: 	mac

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


__attribute__((weak))  size_t DT_CustomMutator(uint8_t* data, size_t size, size_t max_size, unsigned int seed);
__attribute__((weak))  size_t DT_CustomCrossOver(const uint8_t *data, size_t size1,const uint8_t *data2, size_t size2, uint8_t *out,size_t max_size, unsigned int Seed);

 int CustomMutatorGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if (DT_CustomMutator)
    {
        return MAX_COUNT;
    }

    if (DT_CustomCrossOver)
    {
        return MAX_COUNT;
    }
    
    return 0;
}

char* CustomMutatorGetValue1(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    int inLen;
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    if (DT_CustomMutator)
    {
        pElement->para.len = (int)DT_CustomMutator((uint8_t*)pElement->para.value, inLen, pElement->para.maxLen, RAND_32());
    }
    
    return pElement->para.value;
}

char* CustomCrossOverGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    // 找一个样本
    int corpusPos = CorpusSelect();

    // 找一个参数
    int paraPos =  RAND_RANGE(0, InGetParaNum() - 1);

    // 得到值和长度
    char* buf;
    int len;

    CorpusParaValueGet(corpusPos, paraPos, &buf, &len);

    int inLen;
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, pElement->para.maxLen);

    if (DT_CustomCrossOver)
    {
        pElement->para.len = (int)DT_CustomCrossOver((uint8_t*)pElement->inBuf, inLen, (uint8_t*)buf, len, (uint8_t*)pElement->para.value,pElement->para.maxLen, RAND_32());
    }
    
    return pElement->para.value;
}

char* CustomMutatorGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);

    int all = 0;

    if (DT_CustomMutator)
    {
        all++;
    }

    if (DT_CustomCrossOver)
    {
        all++;
    }


    int temp = RAND_32() % all;

    // 只有一个，谁有用谁
    if(all == 1)
    {
         if (DT_CustomMutator)
        {
            return CustomMutatorGetValue1(pElement, pos);
        }

         if (DT_CustomCrossOver)
        {
            return CustomCrossOverGetValue(pElement, pos);
        }
    }

    // 两者都有，一人一半
    if(all == 2)
    {
         if ((DT_CustomMutator) && (temp == 0))
        {
            return CustomMutatorGetValue1(pElement, pos);
        }

         if ((DT_CustomCrossOver) && (temp == 1))
        {
            return CustomCrossOverGetValue(pElement, pos);
        }
    }
    
    return CustomMutatorGetValue1(pElement, pos);
}

int CustomMutatorGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_BLOB)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_customMutatorGroup = {
    "CustomMutator",
    CustomMutatorGetCount,
    CustomMutatorGetValue,
    CustomMutatorGetIsSupport,
    1
};

void InitCustomMutator(void)
{
    RegisterMutater(&g_customMutatorGroup, ENUM_CUSTOMMUTATOR);
}

#ifdef __cplusplus
}
#endif