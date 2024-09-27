/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����������Blob�����������������bytes�ᱻ�ı��null��
						���췢����λ�ú�byte�仯������һ��������������ġ�

����:					���Ȳ���

����:					byte������8��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ��blob��FixBlobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int BlobChangeToNullGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return  MIN((int)pElement->inLen, MAX_COUNT);
}

char *BlobChangeToNullGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int start, changeLen;

    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    InGetRegion(inLen, &start, &changeLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = 0;
    }

    HwMemcpy(pElement->para.value + start + changeLen, pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

int BlobChangeToNullGetIsSupport(SElement *pElement)
{
    // ֻҪ���ַ�����֧��
    ASSERT_NULL(pElement);
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobChangeToNullGroup = {
    "BlobChangeToNull",
    BlobChangeToNullGetCount,
    BlobChangeToNullGetValue,
    BlobChangeToNullGetIsSupport,
    1
};

void InitBlobChangeToNull(void)
{
    RegisterMutater(&g_blobChangeToNullGroup, ENUM_BLOB_CHANGE_TO_NULL);
}

#ifdef __cplusplus
}
#endif