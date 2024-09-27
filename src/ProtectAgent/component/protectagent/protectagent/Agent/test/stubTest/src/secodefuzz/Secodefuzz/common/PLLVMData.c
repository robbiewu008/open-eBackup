/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ��ά���������ڴ湳�ӵ�����������

�����֧��llvm����ģ�����ݶ�Ϊ�գ���ȡΪ�ջ���0

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// Ŀǰ�����ڴ�hook���ܰ󶨣��Ժ���ܻ��޸�
#ifdef HAS_HOOK

int *tempPc =0; 

static void GetValueString(char* inTxt, char* outTxt)
{
    // ��ȡ�ԵȺ�Ϊ��ʼ��\r����\nΪ��β���ַ���
    // hw_sscanf(inTxt,"%*[^=]=%[^\r^\n]",outTxt);
    int length = strlen(inTxt);
    int start = 0;
    int k = 0;
    int i = 0;

    for (i = 0; i < length; i++)
    {
        if ((start == 0) && (inTxt[i] == '='))
        {
            start = 1;
            continue;
        }

        if (start == 0)
        {
            continue;
        }

        if ((inTxt[i] == '\r') || (inTxt[i] == '\n'))
        {
            break;
        }
        
        outTxt[k] = inTxt[i];
        k++;
    }
    outTxt[k] = 0;
}

static void ParseOneLine1(char* txt)
{
    char temp_txt1[1100];
    
    // ȥ��ǰ�ߵĿո�
    while (*txt == ' ')
    {
        txt++;
    }

    // ע�Ͳ�ȥ������������ʾ��д����
    if (txt[0] == '#')
    {
        return;
    }

    if (txt[0] == 'l')
    {
        GetValueString(txt, temp_txt1);
        InDelSpace(temp_txt1);
        int number = Inatol(temp_txt1);

        LlvmDataNumberAddValue(tempPc++, number, number);
    }

    if ((txt[0] == '@') || (txt[0] == '$'))
    {
        GetValueString(txt, temp_txt1);
        char *value = HwMalloc(256); // Ŀǰ���֧��256�����������
        HwMemset(value, 0, 256);
        int len = InParseStringToBin(temp_txt1, value);
        
        LlvmDataMemAddValue(tempPc++, value, value, len, len);
        HwFree(value);
    }
}

void ReadDictionary(char *path)
{
    // û��������
    if(path == NULL)
    {
        return;
    }

    char* data = NULL;
    int len;
    int i = 0;
    int j = 0;
    char temp_txt[1100];
    
    ReadFromFile(&data, &len, path);
    if (len == 0)
    {
        if (data != NULL)
        {
            HwFree(data);
            data = NULL;
        }
        return;
    }

    for (i = 0; i < len; i++)
    {
        temp_txt[j++] = data[i];

        // ����\n����ʼ����һ��
        if (data[i] == '\n')
        {
            temp_txt[j] = 0;
            ParseOneLine1(temp_txt);
            j = 0;
        }
    }
    // ���һ��
    temp_txt[j] = 0;
    ParseOneLine1(temp_txt);

    if (data != NULL)
    {
        HwFree(data);
        data = NULL;
    }

    return; 
}

static void InitLlvmDataMalloc(void)
{
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        g_globalThead.hookModule.llvmNumberTableU64 = (SLlvmNumberTableU64 *)HwMalloc(sizeof(SLlvmNumberTableU64));
        g_globalThead.hookModule.llvmMemTable = (SLlvmMemTable *)HwMalloc(sizeof(SLlvmMemTable));

        HwMemset(g_globalThead.hookModule.llvmNumberTableU64, 0, sizeof(SLlvmNumberTableU64));
        HwMemset(g_globalThead.hookModule.llvmMemTable, 0, sizeof(SLlvmMemTable));
        g_globalThead.hookModule.llvmDataIsHasInit = 1;
    }

}

void LlvmDataNumberAddValue(void *callerPc, u64 s1, u64  s2)
{   
    size_t idx;

    if (g_globalThead.hookModule.llvmDataIsHasInit ==0)
    {
        return;
    }

    // ̫С���������������������㷨��
    if ((s1 < 256) && (s2 < 256))
    {
        return;
    }

    // �ų�һЩ�ڴ�Ƚ���䷵��ֵ
    if ((s2 == 0) && (((u32)s1 <= 0xffffffff) && ((u32)s1 >= 0xffffff00)))
    {
        return;
    }

    //һ��pc����256����������
    idx = ((size_t)callerPc ^ ((s1 + s2) % 256)) % SIZE_LLVM_NUMBER_TABLE_U64;
    g_globalThead.hookModule.llvmNumberTableU64->A[idx] = s1;
    g_globalThead.hookModule.llvmNumberTableU64->B[idx] = s2;

    if (g_globalThead.hookModule.llvmNumberTableU64->has_value[idx] == 0)
    {
        g_globalThead.hookModule.llvmNumberTableU64->has_value[idx] = 1;
        g_globalThead.hookModule.llvmNumberTableU64->has_value_table[g_globalThead.hookModule.llvmNumberTableU64->has_value_total] = idx;
        g_globalThead.hookModule.llvmNumberTableU64->has_value_total++;
    }
}

int  LlvmDataNumberGetCount(void)
{
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        return 0;
    }
    
    return g_globalThead.hookModule.llvmNumberTableU64->has_value_total;
}

// ���������ȡһ��
u64 LlvmDataNumberGetValue(void)
{
    size_t idx;
    
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        return 0;
    }
    
    if (g_globalThead.hookModule.llvmNumberTableU64->has_value_total == 0)
    {
        return 0;
    }

    idx = RAND_32() % g_globalThead.hookModule.llvmNumberTableU64->has_value_total;
    idx = g_globalThead.hookModule.llvmNumberTableU64->has_value_table [idx];
    int isEven = RAND_BOOL();
    u64 temp;
    
    if (isEven)
    {
        temp= g_globalThead.hookModule.llvmNumberTableU64->A[idx];
    }
    else
    {
        temp= g_globalThead.hookModule.llvmNumberTableU64->B[idx];
    }

    // С��256����һ��ֵ
    if (temp < 256)
    {
        if (isEven)
        {
            temp= g_globalThead.hookModule.llvmNumberTableU64->B[idx];
        }
        else
        {
            temp= g_globalThead.hookModule.llvmNumberTableU64->A[idx];
        }
    }
    
    return temp;
}

void LlvmDataMemAddValue(void *callerPc, const char* s1, const char*  s2, size_t n1, size_t n2)
{   
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        return;
    }

    size_t len1 = MIN(n1, SIZE_LLVM_MEM_DATA);
    size_t len2 = MIN(n2, SIZE_LLVM_MEM_DATA);

    size_t hash = 0;

    char *phash = (char *)(&hash);
    size_t i;
    size_t j = 0;

    // ������hash������һ������ζ����hook������
    for (i = 0; i < len1; i++) 
    {
        phash[j++] ^= s1[i];

        if (j == 4)
        {
            j = 0;
        }
    }

    for (i = 0; i < len2; i++) 
    {
        phash[j++] ^= s2[i];

        if (j == 4)
        {
            j = 0;
        }
    }

    //1024 һ���ڴ�Ƚ�λ�����Ӱ��1024��hookֵ,��СӰ����
    size_t idx = (((size_t)callerPc << 2) ^ (hash % 1024)) % SIZE_LLVM_MEM_TABLE;

    for (i = 0; i < len1; i++) 
    {
        g_globalThead.hookModule.llvmMemTable->A[idx][i] = s1[i];
    }
    g_globalThead.hookModule.llvmMemTable->A[idx][i] = 0; // ����/0

    for (i = 0; i < len2; i++) 
    {
        g_globalThead.hookModule.llvmMemTable->B[idx][i] = s2[i];
    }
    g_globalThead.hookModule.llvmMemTable->B[idx][i] = 0; // ����/0

    g_globalThead.hookModule.llvmMemTable->len1[idx] = len1;
    g_globalThead.hookModule.llvmMemTable->len2[idx] = len2;

    if (g_globalThead.hookModule.llvmMemTable->has_value[idx]  == 0)
    {
        g_globalThead.hookModule.llvmMemTable->has_value[idx] = 1;
        g_globalThead.hookModule.llvmMemTable->has_value_table[g_globalThead.hookModule.llvmMemTable->has_value_total] = idx;
        g_globalThead.hookModule.llvmMemTable->has_value_total++;
    }
}


void LlvmDataMemAddValueEx(void *callerPc, const char* s1, const char*  s2)
{
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        return;
    }

    size_t len1 = 0;
    if (s1 != NULL)
    {
        // ��0����,ֻ�����0����
        for (; s1[len1]; len1++) 
        {
        }
    }

    size_t len2 = 0;
    if (s2 != NULL)
    {
        for (; s2[len2]; len2++) 
        {
        }
    }

    if ((len1 <= 1) && (len2 <=1))
    {
        return;
    }

    LlvmDataMemAddValue(callerPc, s1, s2, len1, len2);
}

int  LlvmDataMemGetCount(void)
{
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        return 0;
    }
    
    return g_globalThead.hookModule.llvmMemTable->has_value_total;
}

// ���������ȡһ��
char* LlvmDataMemGetValue(int *len)
{
    if (g_globalThead.hookModule.llvmDataIsHasInit == 0)
    {
        *len = 0;
        return 0;
    }

    if (g_globalThead.hookModule.llvmMemTable->has_value_total == 0)
    {
        *len = 0;
        return 0;
    }

    size_t idx  = RAND_32() % g_globalThead.hookModule.llvmMemTable->has_value_total;
    idx = g_globalThead.hookModule.llvmMemTable->has_value_table[idx];
    int isEven = RAND_32() % 2;
    int isHasZero = RAND_32() % 3;// %33�Ŀ���������/0,��Ϊ�ַ��������ʱ��lenû������/0�ĳ���

    if (isEven)
    {
        *len = g_globalThead.hookModule.llvmMemTable->len1[idx];
    }
    else
    {
        *len = g_globalThead.hookModule.llvmMemTable->len2[idx];
    }

    if (isHasZero == 1)
    {
        *len = *len + 1;
    }

    if (isEven)
    {
        return g_globalThead.hookModule.llvmMemTable->A[idx];
    }
    else
    {
        return g_globalThead.hookModule.llvmMemTable->B[idx];
    }
}

void CleanLlvmData(void)
{
    g_globalThead.hookModule.llvmDataIsHasInit = 0;
    if (g_globalThead.hookModule.llvmNumberTableU64)
    {
        HwFree(g_globalThead.hookModule.llvmNumberTableU64);
        g_globalThead.hookModule.llvmNumberTableU64 = NULL;
    }
    if (g_globalThead.hookModule.llvmMemTable)
    {
        HwFree(g_globalThead.hookModule.llvmMemTable);
        g_globalThead.hookModule.llvmMemTable = NULL;
    }
}

void InitLlvmData(void)
{
    InitLlvmDataMalloc();

    //��ͬ���������Ƿ����ڴ�hook? ��Ҫ˼��
    HwMemset(g_globalThead.hookModule.llvmNumberTableU64, 0, sizeof(SLlvmNumberTableU64));
    HwMemset(g_globalThead.hookModule.llvmMemTable, 0, sizeof(SLlvmMemTable));

    ReadDictionary(g_global.dictionaryPath);

    // һ��������������һ��
    g_global.dictionaryPath = NULL;
}

#else

void LlvmDataNumberAddValue(void *callerPc, u64 s1, u64  s2)
{
}

u64 LlvmDataNumberGetValue(void)
{
    return 0;
}

int  LlvmDataNumberGetCount(void)
{
    return 0;
}

void LlvmDataMemAddValue(void *callerPc, const char* s1, const char*  s2, size_t n1, size_t n2)
{
}

void LlvmDataMemAddValueEx(void *callerPc, const char* s1, const char*  s2)
{
}

char* LlvmDataMemGetValue(int *len)
{
    return NULL;
}

int  LlvmDataMemGetCount(void)
{
    return 0;
}

void InitLlvmData(void)
{
}

void CleanLlvmData(void)
{

}

#endif

#ifdef __cplusplus
}
#endif

