/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����������Blob�ᱻ�������������������bytes��
						�����λ�ò��ô����			
						���������byte��������1��255��ʹ�ø�˹����

						��һ���������ֵ��ȡֵ��Χ��0x00��0xff�����ѡȡ��ֵ
						�ڶ����������ֵ����ǰһ��ֵ�Ļ����ϼ�1��
						�������������ֵ������ټ�1��
						��0xff������ʱ�򣬺����ֵ��ع���0x00.
						�˺��ֵ�������1��ֱ�����еĲ���ֵ����ֵ��Ϊֹ��

����:					ԭʼ���ȵ���󳤶�֮��

����:					MAX_COUNT/5

֧����������: 	�г�ʼֵ��blobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int BlobExpandSingleIncrementingGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    return MAX_COUNT / 50;
}

static char *BlobExpandSingleIncrementingGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen, inLen;
    int start;
    int changeLen;
    char value;
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

    value = RAND_BYTE();
    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = value;
        value++;
    }

    HwMemcpy(pElement->para.value + start + changeLen, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

static int BlobExpandSingleIncrementingGetIsSupport(SElement *pElement)
{
    // ֻҪ���ַ�����֧��
    if ((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobExpandSingleIncrementingGroup = {
    "BlobExpandSingleIncrementing",
    BlobExpandSingleIncrementingGetCount,
    BlobExpandSingleIncrementingGetValue,
    BlobExpandSingleIncrementingGetIsSupport,
    1
};

void InitBlobExpandSingleIncrementing(void)
{
    RegisterMutater(&g_blobExpandSingleIncrementingGroup, ENUM_BLOB_EXPAND_SINGLE_INCREMENTING);
}

#ifdef __cplusplus
}
#endif