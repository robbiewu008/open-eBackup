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
    // ��һ������
    int corpusPos = CorpusSelect();

    // ��һ������
    int paraPos =  RAND_RANGE(0, InGetParaNum() - 1);

    // �õ�ֵ�ͳ���
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

    // ֻ��һ����˭����˭
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

    // ���߶��У�һ��һ��
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