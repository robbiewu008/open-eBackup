/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


action������ʵ�ֺ���
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

void DoActionOpen(SStateElement* tempAction)
{
    DoPublisherOpen(tempAction->publihserName, tempAction);
}

void DoActionClose(SStateElement* tempAction)
{
    DoPublisherClose(tempAction->publihserName, tempAction);
}

void DoActionOutput(SStateElement* tempAction)
{
    //�õ���������
    GetMutatorElementValue(tempAction->binElement);

    //����Padding
    DoPadding(tempAction->binElement);

    //����relation
    DoRelation(tempAction->binElement);

    //����fixup
    DoFixup(tempAction->binElement);

    //����transformer
    DoTransformer(tempAction->binElement);

    //��ϱ���ֵ,�õ����buf
    tempAction->outLength = GetPitsBuf(tempAction->binElement, tempAction->outBuf);

    DoPublisherOutput(tempAction->publihserName, tempAction->outBuf, tempAction->outLength, tempAction);

}

// ��ûʵ��
void DoActionInput(SStateElement  * tempAction)
{

}

#ifdef __cplusplus
}
#endif

#endif // Open_source
