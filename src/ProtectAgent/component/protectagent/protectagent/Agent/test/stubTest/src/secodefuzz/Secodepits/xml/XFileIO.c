/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

��ȡ���õ�bin�ļ�
*/
#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

int GetBinBuf(char* fileName, char** buf, int *length)
{
    if (g_onOffDebugGetBinBuf)
    {
        printf("+++++++++++++++++++++				 Bin Buf is:\r\n");  
    }

    ReadFromFile(buf, length, fileName);
    g_testcaseMemory[g_testcaseMemoryCount++] = *buf;

    if (g_onOffDebugGetBinBuf)
    {
        printf("	----lenght is %d----\r\n", *length);  
    }

    int len = *length;

    if (g_onOffDebugGetBinBuf)
    {
        if (len > DISPLAY_MAX_BIN_LENGTH)
        {
            len = DISPLAY_MAX_BIN_LENGTH;
        }
        
        HexDump((u8*)(*buf), len);

        if(*length > DISPLAY_MAX_BIN_LENGTH)
        {
            printf("... ... shenglue\r\n"); 
        }
    }

    if (g_onOffDebugGetBinBuf)
    {
        printf("+++++++++++++++++++++\r\n");  
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
