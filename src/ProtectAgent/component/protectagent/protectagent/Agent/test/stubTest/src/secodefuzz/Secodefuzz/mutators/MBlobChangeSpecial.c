/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					���������У�Blob���������������byte�ᱻ�����ĸı䣬
						��С�׵��滻ֵ(�Ӳ�������Ͽ���С���滻ֵ��������"01"��"00"��"FF"��"FE")��
						���췢����λ�ú�byte�仯������һ��������������ġ�

����:					���Ȳ���

����:					byte������8��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ��blob��FixBlobԪ��

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

const static u8 special[4] = {0x00, 0x01, 0xfe, 0xff};

int BlobChangeSpecialGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen, MAX_COUNT);
}

char *BlobChangeSpecialGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int count;
    int start, changeLen;

    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    InGetRegion(inLen, &start, &changeLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    count = sizeof(special) - 1;
    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = special[RAND_RANGE(0, count)];
    }

    HwMemcpy(pElement->para.value + start + changeLen, 
        pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

int BlobChangeSpecialGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_blobChangeSpecialGroup = {
    "BlobChangeSpecial",
    BlobChangeSpecialGetCount,
    BlobChangeSpecialGetValue,
    BlobChangeSpecialGetIsSupport,
    1
};

void InitBlobChangeSpecial(void)
{
    RegisterMutater(&g_blobChangeSpecialGroup, ENUM_BLOB_CHANGE_SPECIAL);
}

#ifdef __cplusplus
}
#endif