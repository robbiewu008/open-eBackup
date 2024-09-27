/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����������Blob�����������������null bytes��hex��00���ᱻ�ı䡣
						���췢����λ�ú�byte�仯������һ��������������ġ�
						ʹ�����byte���

����:					���Ȳ���

����:					0��������8��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ��blob��FixBlobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int BlobChangeFromNullGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((InGetBufZeroNumber(pElement->inBuf, pElement->inLen / 8) * 8), MAX_COUNT);
}

// ��Ϊ�Ǳ���0�����Ի���Ҫ�Ż�
static char *BlobChangeFromNullGetValue(SElement *pElement, int pos)
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

    HwMemcpy(pElement->para.value + start + changeLen , 
        pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

static int BlobChangeFromNullGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobChangeFromNullGroup = {
    "BlobChangeFromNull",
    BlobChangeFromNullGetCount,
    BlobChangeFromNullGetValue,
    BlobChangeFromNullGetIsSupport,
    1
};

void InitBlobChangeFromNull(void)
{
    RegisterMutater(&g_blobChangeFromNullGroup, ENUM_BLOB_CHANGE_FROM_NULL);
}

#ifdef __cplusplus
}
#endif
