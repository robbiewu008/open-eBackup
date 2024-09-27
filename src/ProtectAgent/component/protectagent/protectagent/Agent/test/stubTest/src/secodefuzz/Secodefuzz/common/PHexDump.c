/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
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
    // 每行固定16或者32个字节，符合国际审美观
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
            //  转换成ASCii 形式
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
    
    //  加上对应的ASCII字符串，区别如下：
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
    // 记得加上结束符号，有可能导致printk --> kernel panic 
    lineBuf[lx++] = '\0';
}

void PrintHexDump1(unsigned int rowSize,
    const u8 *buf, size_t len, int ascii)
{
    const u8 *ptr = buf;
    
    unsigned int k, lineLen;
    unsigned int reMaining = len;
        /* 每行数据类似下面
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
        //  lineBuf 返回需要打印的字符串
        HexDumpToBuffer1(ptr + k, lineLen, rowSize,
        (char *)lineBuf, sizeof(lineBuf), ascii);
        hw_printf("%s\n", lineBuf);
    }
}

// 一般在kernel代码中，调用此接口，许多参数用默认的，不需要了解太多细节
void HexDump(u8 *buf, u32 len)
{
    PrintHexDump1(16, buf, len, 1);
}

#ifdef __cplusplus
}
#endif