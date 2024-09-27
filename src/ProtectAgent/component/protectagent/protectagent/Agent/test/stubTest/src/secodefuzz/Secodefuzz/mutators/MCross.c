/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����֮�䣬����֮�佻������
						��������㷨���õ����ⲿ�����϶࣬�Ƚ�Σ��

����:					

����:					

֧����������: 

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

    // ��һ������
    int corpusPos = CorpusSelect();

    // ��һ������
    int paraPos =  RAND_RANGE(0, InGetParaNum() - 1);

    // �õ�ֵ�ͳ���
    char* buf;
    int len;
    char* buf1;
    int len1;

    CorpusParaValueGet(corpusPos, paraPos, &buf, &len);

    // �ҵ�Ҫ�滻��ֵ�ͳ���
    int temp_pos = RAND_RANGE(0, len);
    buf1 = buf + temp_pos;
    len1 = RAND_RANGE(0, len - temp_pos);
    

     int temp = RAND_32() % 100;
     int isChangeLength = InGetTypeIsChangeLength(pElement->para.type);
    
    // �ٷ�֮30�ĸ����滻�������г��ȣ����Ȳ���
    if ((temp < 40) || (isChangeLength == ENUM_NO))
    {
        MagicGetValue(pElement, buf1, len1, ENUM_CHANGE);
    }
    // �����滻���ɱ䳤��
    else if ((temp < 70) && (isChangeLength == ENUM_YES))
    {
        MagicGetValue(pElement, buf1, len1, ENUM_REPLACE);
    }
    // �м����
    else if (isChangeLength == ENUM_YES)
    {
        MagicGetValue(pElement, buf1, len1, ENUM_INSERT);
    }
    
    return pElement->para.value;
}

int CrossGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    // ö�ٲ�֧��
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self�����㷨�����Σ���Ҫ���Լ���
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