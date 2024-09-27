/*
版权所有 (c) 华为技术有限公司 2012-2018


action操作的实现函数
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
    //得到变异数据
    GetMutatorElementValue(tempAction->binElement);

    //处理Padding
    DoPadding(tempAction->binElement);

    //处理relation
    DoRelation(tempAction->binElement);

    //处理fixup
    DoFixup(tempAction->binElement);

    //处理transformer
    DoTransformer(tempAction->binElement);

    //组合变异值,得到输出buf
    tempAction->outLength = GetPitsBuf(tempAction->binElement, tempAction->outBuf);

    DoPublisherOutput(tempAction->publihserName, tempAction->outBuf, tempAction->outLength, tempAction);

}

// 还没实现
void DoActionInput(SStateElement  * tempAction)
{

}

#ifdef __cplusplus
}
#endif

#endif // Open_source
