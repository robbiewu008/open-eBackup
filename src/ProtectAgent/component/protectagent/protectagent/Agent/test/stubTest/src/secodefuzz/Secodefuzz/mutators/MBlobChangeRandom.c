/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����������Blob�����������������bytes�ᱻ�ı䡣
						���췢����λ�ú�byte�仯������һ��������������ġ�
						ʹ�����byte���

����:					���Ȳ���

����:					bitֵ��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ��blob��FixBlobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int BlobChangeRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen, MAX_COUNT);
}

char *BlobChangeRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int start;
    int changeLen;
    
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    InGetRegion(inLen, &start, &changeLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = RAND_BYTE();
    }

    HwMemcpy(pElement->para.value + start + changeLen, 
        pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

int BlobChangeRandomGetIsSupport(SElement *pElement)
{
    // ֻҪ���ַ�����֧��
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobChangeRandomGroup = {
    "BlobChangeRandom",
    BlobChangeRandomGetCount,
    BlobChangeRandomGetValue,
    BlobChangeRandomGetIsSupport,
    1
};

void InitBlobChangeRandom(void)
{
    RegisterMutater(&g_blobChangeRandomGroup, ENUM_BLOB_CHANGE_RANDOM);
}

#ifdef __cplusplus
}
#endif