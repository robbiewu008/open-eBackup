/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180
wangchengyun 	wwx412654

模块之间公用的接口放在这里

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

 u64 g_edgeCaseTable[] =
{
    // 0
    // 0x0,     0x0,
    // 1
    0x0,        0x1,
    // 2
    0x2,        0x3,
    // 3
    0x4,        0x7,
    // 4
    0x8,        0xF,
    // 5
    0x10,       0x1F,
    // 6
    0x20,       0x3F,
    // 7
    0x40,       0x7F,
    // 8
    0x80,       0xFF,
    // 9
    0x100,      0x1FF,
    // 10
    0x200,      0x3FF,
    // 11
    0x400,      0x7FF,
    // 12
    0x800,      0xFFF,
    // 13
    0x1000,     0x1FFF,
    // 14
    0x2000,     0x3FFF,
    // 15
    0x4000,     0x7FFF,
    // 16
    0x8000,     0xFFFF,
    // 17
    0x10000,    0x1FFFF,
    // 18
    0x20000,    0x3FFFF,
    // 19
    0x40000,    0x7FFFF,
    // 20
    0x80000,    0xFFFFF,
    // 21
    0x100000,   0x1FFFFF,
    // 22
    0x200000,   0x3FFFFF,
    // 23
    0x400000,   0x7FFFFF,
    // 24
    0x800000,   0xFFFFFF,
    // 25
    0x1000000,  0x1FFFFFF,
    // 26
    0x2000000,  0x3FFFFFF,
    // 27
    0x4000000,  0x7FFFFFF,
    // 28
    0x8000000,  0xFFFFFFF,
    // 29
    0x10000000, 0x1FFFFFFF,
    // 30
    0x20000000, 0x3FFFFFFF,
    // 31
    0x40000000, 0x7FFFFFFF,
    // 32
    0x80000000, 0xFFFFFFFF,
    // 33
    0x100000000,        0x1FFFFFFFF,
    // 34
    0x200000000,        0x3FFFFFFFF,
    // 35
    0x400000000,        0x7FFFFFFFF,
    // 36
    0x800000000,        0xFFFFFFFFF,
    // 37
    0x1000000000,       0x1FFFFFFFFF,
    // 38
    0x2000000000,       0x3FFFFFFFFF,
    // 39
    0x4000000000,       0x7FFFFFFFFF,
    // 40
    0x8000000000,       0xFFFFFFFFFF,
    // 41
    0x10000000000,      0x1FFFFFFFFFF,
    // 42
    0x20000000000,      0x3FFFFFFFFFF,
    // 43
    0x40000000000,      0x7FFFFFFFFFF,
    // 44
    0x80000000000,      0xFFFFFFFFFFF,
    // 45
    0x100000000000,     0x1FFFFFFFFFFF,
    // 46
    0x200000000000,     0x3FFFFFFFFFFF,
    // 47
    0x400000000000,     0x7FFFFFFFFFFF,
    // 48
    0x800000000000,     0xFFFFFFFFFFFF,
    // 49
    0x1000000000000,    0x1FFFFFFFFFFFF,
    // 50
    0x2000000000000,    0x3FFFFFFFFFFFF,
    // 51
    0x4000000000000,    0x7FFFFFFFFFFFF,
    // 52
    0x8000000000000,    0xFFFFFFFFFFFFF,
    // 53
    0x10000000000000,   0x1FFFFFFFFFFFFF,
    // 54
    0x20000000000000,   0x3FFFFFFFFFFFFF,
    // 55
    0x40000000000000,   0x7FFFFFFFFFFFFF,
    // 56
    0x80000000000000,   0xFFFFFFFFFFFFFF,
    // 57
    0x100000000000000,  0x1FFFFFFFFFFFFFF,
    // 58
    0x200000000000000,  0x3FFFFFFFFFFFFFF,
    // 59
    0x400000000000000,  0x7FFFFFFFFFFFFFF,
    // 60
    0x800000000000000,  0xFFFFFFFFFFFFFFF,
    // 61
    0x100000000000000,  0x1FFFFFFFFFFFFFFF,
    // 62
    0x200000000000000,  0x3FFFFFFFFFFFFFFF,
    // 63
    0x400000000000000,  0x7FFFFFFFFFFFFFFF,
    // 64
    0x800000000000000,  0xFFFFFFFFFFFFFFFF,
};

const char *g_typeStr[] = {
    "UnsignedInteger", "SignedInteger", "EnumInteger", "RangeInteger", "String", "StringNum", "StringEnum", 
    "Blob", "BlobEnum", "FixBlob", "AFL", "IPV4", "IPV6", "MAC", "Float16", "Float32", "Float64", "Double", "self", "MAX"
};

const char *g_inTypeStr[] = {
    "no support", "u8", "u16", "u32", "u64", "float", "double", "S8", "S16", "S32", "S64", "EnumInteger",
    "EnumIntegerEX", "RangeInteger", "RangeIntegerEX", "String", "StringNum", "StringEnum", "StringEnumEX",
    "Blob", "BlobEnum", "BlobEnumEX", "FixBlob", "AFL", "IPV4", "IPV6", "MAC", "Float16", "Float32", "Float64", "Double", "self", "MAX"
};

// 得到数字的bit宽度
u32   InGetBitNumber(u32 n)
{
    u32 c;
    if (n == 0)
    {
        return 0;
    }

    c = 32;

    if (!(n & 0xffff0000))
    {
        c -= 16;
        n <<= 16;
    }
    if (!(n & 0xff000000))
    {
        c -= 8;
        n <<= 8;
    }
    if (!(n & 0xf0000000))
    {
        c -= 4;
        n <<= 4;
    }
    if (!(n & 0xc0000000))
    {
        c -= 2;
        n <<= 2;
    }
    if (!(n & 0x80000000))
    {
        c -= 1;
    }

    return c;
} 

const char *InGetStringFromType(int type)
{
    int index = sizeof(g_typeStr) / sizeof(g_typeStr[0]) - 1;

    if (type < index)
    {
        index = type;
    }
    return g_typeStr[index];
}

int InGetTypeFromString(char* typeName)
{
    int count = sizeof(g_typeStr) / sizeof(char *);
    int i;
    for (i = 0; i < count; i++)
    {
        if (HwStrCmp(g_typeStr[i], typeName) == 0)
        {
            break;
        }
    }
    return i;
}

const char *InGetStringFromInType(int type)
{
    int index = sizeof(g_inTypeStr) / sizeof(g_inTypeStr[0]) - 1;

    if (type < index)
    {
        index = type;
    }
    return g_inTypeStr[index];
}

int InGetInTypeFromString(char* typeName)
{
    int count = sizeof(g_inTypeStr) / sizeof(char *);
    int i;
    for (i = 0; i < count; i++)
    {
        if(HwStrCmp(g_inTypeStr[i], typeName) == 0)
        {
            break;
        }
    }
    return i;
}

// 得到是否是枚举或者范围类型
int InGetTypeIsEnumOrRange(int Type)
{
     if ((Type == ENUM_NUMBER_ENUM)
        || (Type == ENUM_NUMBER_RANGE)
        || (Type == ENUM_STRING_ENUM)
        || (Type == ENUM_BLOB_ENUM))
    {
        return ENUM_YES;
    }

    return ENUM_NO;

}

// 得到是否是枚举或者范围类型
int InGetTypeIsChangeLength(int Type)
{
     if ((Type == ENUM_STRING)
        || (Type == ENUM_BLOB)
        || (Type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;

}

int InGetParaNum(void)
{
    return g_globalThead.wholeRandomNum;
}


// 判断字符串能否转化为数字
int InStringIsNumber(SElement *pElement)
{
    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_YES;
    }
    
    if (pElement->para.type == ENUM_STRING)
    {
        if (pElement->isHasInitValue == ENUM_YES)
        {
            s64 temp_s64 = Inatol(pElement->inBuf);
            if ((temp_s64 != 0) && (temp_s64 != -1))
            {
                return ENUM_YES;
            }
        }
    }

    return ENUM_NO;
}

// 将inbuf copy给outbuf,如果有初始值的话，没有则置空
char *SetElementOriginalValue(SElement *pElement)
{
    if (pElement->inLen)
    {
        pElement->isNeedFreeOutBuf = ENUM_YES;
        pElement->para.len = (pElement->inLen) >> 3;
        pElement->para.value = HwMalloc((pElement->inLen) >> 3);
        HwMemcpy(pElement->para.value, pElement->inBuf, pElement->para.len);
    }
    else
    {
        pElement->isNeedFreeOutBuf = ENUM_NO;
        pElement->para.len = 0;
        pElement->para.value = NULL;
    }
    
    return pElement->para.value;
}

// 为outbuf分配内存，设置长度
char *SetElementInitoutBuf(SElement *pElement, int len)
{
    pElement->isNeedFreeOutBuf = ENUM_YES;
    pElement->para.len = len;

    if (len > 0)
    {
        pElement->para.value = HwMalloc(len);
    }
    else
    {
        pElement->para.value = NULL;
    }
    
    return pElement->para.value;
}

// 为outbuf分配内存，设置长度
char *SetElementInitoutBuf2(SElement *pElement, int len)
{
    if (g_globalThead.isHasInit == 0)
    {
        g_globalThead.isHasInit = 1;
        g_globalThead.valueBuf = HwMalloc(DEFAULT_MAX_OUTPUT_SIZE);
    }
    pElement->isNeedFreeOutBuf = ENUM_NO;
    pElement->para.len = len;
    pElement->para.value = g_globalThead.valueBuf;
    
    return pElement->para.value;
}

char *SetElementInitoutBufEx(SElement *pElement, int len)
{
    if (IS_USE_GLOBAL_MALLOC == 0)
    {
        return SetElementInitoutBuf(pElement, len);
    }
    else
    {
        return SetElementInitoutBuf2(pElement, len);
    }
}

char* MagicGetValue(SElement *pElement, char* data, int len, int type)
{
    int inLen;
    int start;

    ASSERT_NULL(pElement);

    // 找到要插入或者覆盖的起始位置
    inLen = (int)(pElement->inLen / 8);
    start = RAND_RANGE(0, inLen);

    if (type == ENUM_INSERT) // Insert
    {
        if ((len + inLen) > pElement->para.maxLen)
        {
            return SetElementOriginalValue(pElement);
        }

        SetElementInitoutBufEx(pElement, len + inLen);

        HwMemcpy(pElement->para.value, pElement->inBuf, inLen); 
        HwMemcpy(pElement->para.value + start, data, len); 
        HwMemcpy(pElement->para.value + start + len, pElement->inBuf + start, inLen - start); 
    }
    else if (type == ENUM_OVERWRITE) // Overwrite
    {
        if ((len + start) > pElement->para.maxLen)
        {
            return SetElementOriginalValue(pElement);
        }

        SetElementInitoutBufEx(pElement, MAX(len + start, inLen));

        HwMemcpy(pElement->para.value, pElement->inBuf, inLen); 
        HwMemcpy(pElement->para.value + start, data, len); 
    }
    else if (type == ENUM_CHANGE) // Change
    {
        SetElementInitoutBufEx(pElement, inLen);

        HwMemcpy(pElement->para.value, pElement->inBuf, inLen); 
        HwMemcpy(pElement->para.value + start, data, MIN(inLen - start, len)); 
    }
    else if (type == ENUM_REPLACE)   // replace
    {
        if (len > pElement->para.maxLen)
        {
            len = pElement->para.maxLen;
        }

        SetElementInitoutBufEx(pElement, len);

        HwMemcpy(pElement->para.value , data, len); 
    }
    return pElement->para.value;
}

// 得到buf 内所有byte中0的数量
int InGetBufZeroNumber(char* string, int len)
{
    int count = 0;
    int i;
    for (i = 0; i < len; i++)
    {
        if (string[i] == 0)
        {
            count ++;
        }
    }
    
    return count;
}

void InGetRegion(int length, int *outStart, int *outLength)
{
    int value;
    
    ASSERT_ZERO(length);
    ASSERT_NULL(outStart);
    ASSERT_NULL(outLength);
    
    value = RAND_RANGE(1, length);
    *outStart = RAND_RANGE(0, length - value);
    *outLength = value;
}

// 得到字符串里英文字母的数量
int InGetLetterNumber(char* string)
{
    int count = 0;
    int i;
    int k = Instrlen(string);
    for (i = 0; i < k; i++)
    {
        count = count + InIsLetter(string[i]);
    }
    
    return count;
}

int InToUpper(int c)
{
    return (c >= 'a' && c <= 'z') ? (c + 'A' - 'a') : c;
}

// tolower
int InToLower(int c)
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

int InIsLetter(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return 1;
    }

    if(c >= 'A' && c <= 'Z')
    {
        return 1;
    }

    return 0;
}

int InIsAscii(char c) 
{
    return (((c) & ~0x7f) == 0);
}

int InIsPrint(char c) 
{
    return ((31 < c) && (c < 127));
}
int InIsSpace(char c)
{
    if (c == '\t'|| c == '\n'|| c == ' ')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int InIsxDigit(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return 1;
    }

    if ((c >= 'a') && (c <= 'f'))
    {
        return 1;
    }

    if ((c >= 'A') && (c <= 'F'))
    {
        return 1;
    }

    return 0;
}

int InIsDigit(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return 1;
    }

    return 0;
}

u32 Instrlen(const char *s)
{
    int i; 
    for (i = 0; s[i]; i++)
    {        
    }
    return i; 
}

u64 Insqrt(u64 x) 
{
    u64 a, b; 
    
    if (x <= 0)
    {
        return 0;
    }
    
    a = (x >> 3) + 1; 
    
    while (1)
    { 
        b = ((a + 1) >> 1) + ((x / a) >> 1); 
        if (a - b < 2) 
        {
            return b - 1 + ((x - b * b + (b << 2)) / b >> 2);
        }
        a = b; 
    } 
} 

// 长整形转字符型
// num要转换的数值，*str字符串，radix转换的进制  
char *Inltoa(s64 value, char *string, int radix)
{
    char tmp[33];
    char *tp = tmp;
    s64 i;
    s64 vv;
    int signED;
    char *sp;

    if (radix > 36 || radix <= 1)
    {
        return 0;
    }

    signED = (radix == 10 && value < 0);
    if (signED)
    {
        vv = -value;
    }
    else
    {
        vv = (s64)value;
    }
    while (vv || tp == tmp)
    {
        i = vv % radix;
        vv = vv / radix;
        if (i < 10)
        {
            *tp++ = i + '0';
        }
        else
        {
            *tp++ = i + 'a' - 10;
        }
    }

    if (string == 0)
    {
        string = (char *)HwMalloc((tp - tmp) + signED + 1);
    }
    sp = string;

    if (signED)
    {
        *sp++ = '-';
    }

    while (tp > tmp)
    {
        *sp++ = *--tp;
    }
    *sp = 0;
    return string;
}

// 把一个数字字符串转换为一个整数。
s64 Inatol(char *string)
{
    s64 value = 0;
    s64 f = 1; 

    // 去掉前边的空格，最后增加代码，注意可能有bug
    while (*string == ' ')
    {
        string++;
    }   

    if (*string == '-')
    {
        string ++;
        f = -1;
    }

    // 逐个把字符串的字符转换为数字。
    while (*string >= '0' && *string <= '9')
    {
        value *= 10;
        value += *string - '0';
        string++;
    }

    value = f * value;

    // 错误检查：如杲由于遇到一个非数字字符而终止，把结果设置为0
    // 这个暂时注释掉，感觉也没啥用  ，取数字部分就好了
    // if(*string != '\0')
    //     value = 0;

    return value;
}

// 将十六进制的字符串转换成整数  
s64 Inhtol(char *s)  
{  
    s64 i;  
    s64 n = 0;  
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i)  
    {  
        if (InToLower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + InToLower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (InToLower(s[i]) - '0');  
        }  
    }  
    return n;  
}  

void InDelSpace(char *str)  
{  
    char *p = str;  
    char *t = p;  
    while (*p != '\0') 
    {  
        if (*p == ' ') 
        {  
            t++;  
            if (*t != ' ') 
            {  
                *p = *t;  
                *t = ' ';  
            }  
        }
        else  
        {  
            p++;  
            t = p;  
        }  
    }  
}  

int InParseStringToBin(char *str, char* buf) 
    {
    int temp = 0;
    size_t pos;

    size_t l = 0;  // We are parsing the range [l,r].
    size_t r = Instrlen(str) - 1;  // We are parsing the range [l,r].
    
    // Skip spaces from both sides.
    while (l < r && InIsSpace(str[l])) 
    {
        l++;
    }

    while (r > l && InIsSpace(str[r])) 
    {
        r--;
    }

    if (r - l < 2) 
    {
        return 0;
    }

    // Check the closing "
    if (str[r] != '"') 
    {
        return 0;
    }

    r--;

    // Find the opening "
    while (l < r && str[l] != '"') 
    {
        l++;
    }

    if (l >= r)
    {
        return 0;
    }

    l++;

    for (pos = l; pos <= r; pos++) 
    {
        uint8_t v = (uint8_t)str[pos];

        if (!InIsPrint(v) && !InIsSpace(v))
        {
            return 0;
        }
        
        if (v == '\\') 
        {
            // Handle '\\'
            if (pos + 1 <= r && (str[pos + 1] == '\\' || str[pos + 1] == '"')) 
            {
                buf[temp++] = str[pos + 1];
                pos++;
                continue;
            }
            // Handle '\xAB'
            if (pos + 3 <= r && str[pos + 1] == 'x'
            && InIsxDigit(str[pos + 2]) && InIsxDigit(str[pos + 3])) 
            {
                char Hex[] = "0xAA";
                Hex[2] = str[pos + 2];
                Hex[3] = str[pos + 3];
                buf[temp++] = (HwStrToL(Hex, NULL, 16));
                pos += 3;
                continue;
            }
            return 0;  // Invalid escape.
        } 
        else 
        {
            // Any other character.
            buf[temp++] = v;
        }
    }
  return temp;
}

int InParseBinToString(char *str, char* buf, int len) 
{
    int size;
    int k;

    size = hw_sprintf(str, "value\t\t=\"");

    for (k = 0; k < len; k++)
    {
        uint8_t byte = buf[k];
        if (byte == '\\')
        {
            size += hw_sprintf(str + size, "\\\\");
        }
        else if (byte == '"')
        {
            size += hw_sprintf(str + size, "\\\"");
        }
        else if (byte >= 32 && byte < 127)
        {
            size += hw_sprintf(str + size, "%c", byte);
        }
        else
        {
            size += hw_sprintf(str + size, "\\x%02x", byte);
        }
    }
    size += hw_sprintf(str + size, "\"\r\n");
    return size;
}

int InParseBinToHexString(char *str, char* buf, int len) 
{
    int size;
    int k;

    size = hw_sprintf(str, "hexvalue\t=\"");

    for (k = 0; k < len; k++)
    {
        uint8_t byte = buf[k];

        size += hw_sprintf(str + size, "\\x%02x", byte);

    }
    size += hw_sprintf(str + size, "\"\r\n");
    return size;
}

// 大小端转换
u8  InBswap8(u8 x) 
{ 
    return x; 
}

u16 InBswap16(u16 x) 
{ 
    u16 y;
    char* tempx = (char *)(&x);
    char* tempy = (char *)(&y);
    tempy[1] = tempx[0];
    tempy[0] = tempx[1];

    return y;
}
u32 InBswap32(u32 x) 
{ 
    u32 y;
    char* tempx = (char *)(&x);
    char* tempy = (char *)(&y);
    tempy[3] = tempx[0];
    tempy[2] = tempx[1];
    tempy[1] = tempx[2];
    tempy[0] = tempx[3];

    return y;
}
u64 InBswap64(u64 x)
{ 
    u64 y;
    char* tempx = (char *)(&x);
    char* tempy = (char *)(&y);
    tempy[7] = tempx[0];
    tempy[6] = tempx[1];
    tempy[5] = tempx[2];
    tempy[4] = tempx[3];
    tempy[3] = tempx[4];
    tempy[2] = tempx[5];
    tempy[1] = tempx[6];
    tempy[0] = tempx[7];
    return y;
}

#ifdef __cplusplus
}
#endif