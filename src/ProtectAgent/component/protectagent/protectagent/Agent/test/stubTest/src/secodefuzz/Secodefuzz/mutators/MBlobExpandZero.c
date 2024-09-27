/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����������Blob�ᱻ�������������������null bytes��
						�����λ�ò��ô����			
						���������byte��������1��255��ʹ�ø�˹����

����:					ԭʼ���ȵ���󳤶�֮��

����:					MAX_COUNT/5

֧����������: 	�г�ʼֵ��blobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int BlobExpandZeroGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    return MAX_COUNT / 50;
}

static char *BlobExpandZeroGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen, inLen;
    int start;
    int changeLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    start = RAND_RANGE(0, inLen);

    // ֻ����1��255�����byte
    changeLen = GaussRandU32(RAND_32() % 8);

    outLen = inLen + changeLen;

    if (outLen > pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
        changeLen = pElement->para.maxLen - inLen;
    }

    SetElementInitoutBufEx(pElement, outLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = 0;
    }

    HwMemcpy(pElement->para.value + start + changeLen, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

static int BlobExpandZeroGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // ֻҪ���ַ�����֧��
    if ((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobExpandZeroGroup = {
    "BlobExpandZero",
    BlobExpandZeroGetCount,
    BlobExpandZeroGetValue,
    BlobExpandZeroGetIsSupport,
    1
};

void InitBlobExpandZero(void)
{
    RegisterMutater(&g_blobExpandZeroGroup, ENUM_BLOB_EXPAND_ZERO);
}

#ifdef __cplusplus
}
#endif