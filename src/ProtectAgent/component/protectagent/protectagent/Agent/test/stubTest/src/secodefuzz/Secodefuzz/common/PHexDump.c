/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180
wangchengyun 	wwx412654


*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

const char hex_asc[] = "0123456789abcdef";
#define HexAscLo(x)   hex_asc[((x) & 0x0f)]
#define HexAscHi(x)   hex_asc[((x) & 0xf0) >> 4]

void HexDumpToBuffer1(const u8 *buf, size_t bufLen, unsigned int rowSize,
    char *lineBuf, size_t lineBufLen, int ascii)
{
    const u8 *ptr = buf;
    u8 ch;
    unsigned int j, lx = 0;
    unsigned int asciiColumn;
    // ÿ�й̶�16����32���ֽڣ����Ϲ���������
    if (rowSize != 16 && rowSize != 32)
    {
        rowSize = 16;
    }

    if (!bufLen)
    {
        goto nil;
    }
    
    if (bufLen > rowSize)       /*  limit to one line at a time  */
    {
        bufLen = rowSize;
    }
    
    switch (1) 
    {
        default:
            //  ת����ASCii ��ʽ
            for (j = 0; (j < bufLen) && (lx + 3) <= lineBufLen; j++) 
            {
                ch = ptr[j];
                lineBuf[lx++] = HexAscHi(ch);
                lineBuf[lx++] = HexAscLo(ch);
                lineBuf[lx++] = ' ';
            }
            if (j)
            {
                lx--;
            }

            asciiColumn = 3 * rowSize + 2;
            break;
    }
    
    if (!ascii)
    {
        goto nil;
    }
    
    //  ���϶�Ӧ��ASCII�ַ������������£�
    //  0009ab42: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
    //  0009ab42: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  
    while (lx < (lineBufLen - 1) && lx < (asciiColumn - 1))
    {
        lineBuf[lx++] = ' ';
    }
    
    for (j = 0; (j < bufLen) && (lx + 2) < lineBufLen; j++) 
    {
        ch = ptr[j];
        lineBuf[lx++] = ((ch) && InIsAscii(ch) && InIsPrint(ch)) ? ch : '.';
    }
    nil:
    // �ǵü��Ͻ������ţ��п��ܵ���printk --> kernel panic 
    lineBuf[lx++] = '\0';
}

void PrintHexDump1(unsigned int rowSize,
    const u8 *buf, size_t len, int ascii)
{
    const u8 *ptr = buf;
    
    unsigned int k, lineLen;
    unsigned int reMaining = len;
        /* ÿ��������������
           40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
           |                                          |  |                 |
           -----------------------------------------------
                    16*3 or 32*3                       2      32+1 
        */
    unsigned char lineBuf[32 * 3 + 2 + 32 + 1];

    if (rowSize != 16 && rowSize != 32)
    {
        rowSize = 16;
    }

    for (k = 0; k < len; k += rowSize) 
    {
        lineLen = MIN(reMaining, rowSize);
        reMaining -= rowSize;
        //  lineBuf ������Ҫ��ӡ���ַ���
        HexDumpToBuffer1(ptr + k, lineLen, rowSize,
        (char *)lineBuf, sizeof(lineBuf), ascii);
        hw_printf("%s\n", lineBuf);
    }
}

// һ����kernel�����У����ô˽ӿڣ���������Ĭ�ϵģ�����Ҫ�˽�̫��ϸ��
void HexDump(u8 *buf, u32 len)
{
    PrintHexDump1(16, buf, len, 1);
}

#ifdef __cplusplus
}
#endif