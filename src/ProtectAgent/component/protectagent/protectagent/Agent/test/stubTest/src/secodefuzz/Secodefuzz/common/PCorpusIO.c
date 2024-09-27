/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块提供样本输入输出管理

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_HOOK
__attribute__((weak)) int* __asan_get_report_pc(void);
#endif

static void GetValueString(char* inTxt, char* outTxt)
{
    // 获取以等号为开始，\r或者\n为结尾的字符串
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

void CorpusGetValueAndLen(char** buf,int * len,int pos)
{
    int tempLen;
    char *tempBuf;
    int i;

    tempLen = 0;
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        SPara* tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[pos]->para[i];
        tempLen += tempCorpus->len;
    }

    if (tempLen > 0)
    {
        tempBuf = HwMalloc(tempLen);
    }
    else
    {
        tempBuf = NULL;
    }

    tempLen = 0;
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        SPara* tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[pos]->para[i];
        HwMemcpy(tempBuf + tempLen,  tempCorpus->value, tempCorpus->len);
        tempLen += tempCorpus->len;
    }

    *buf = tempBuf;
    *len = tempLen;
}

unsigned int CorpusGetHash(int pos)
{
    char* buf;
    int len;

    CorpusGetValueAndLen(&buf,&len,pos);
    unsigned int hash = HwCheckSum32((unsigned char *)buf, len);
    HwFree(buf);
    
    return hash;
}

unsigned int ElementGetHash(void)
{
    int tempLen;
    char *tempBuf;
    unsigned int hash;
    int i;

    tempLen = 0;
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {   
        if (g_Element[i].element != NULL)
        {
            tempLen += g_Element[i].element->para.len;
        }
    }

    if (tempLen > 0)
    {
        tempBuf = HwMalloc(tempLen);
    }
    else
    {
        tempBuf = NULL;
    }
    
    tempLen = 0;
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {   
        if (g_Element[i].element != NULL)
        {
            HwMemcpy(tempBuf + tempLen,  g_Element[i].element->para.value, g_Element[i].element->para.len);
            tempLen += g_Element[i].element->para.len;
        }
    }
    
    hash = HwCheckSum32((unsigned char *)tempBuf, tempLen);
    HwFree(tempBuf);
    
    return hash;
}

static void ParseOneLine(char* txt)
{
    SPara* tempCorpus;

    // 只读取允许的最大样本
    if (g_globalThead.corpusModule.corpusM->corpusNum >= g_global.maxCorpusnum)
    {
        return;
    }
    
    // 去掉前边的空格
    while (*txt == ' ')
    {
        txt++;
    }

    // 注释不去管它，这里显示的写出来
    if (txt[0] == '#')
    {
        return;
    }

    // 以!为单个样本获取的结束符
    if (txt[0] == '!')
    {
        if ((g_globalThead.corpusModule.corpusM->paraNum == 0) && (g_globalThead.corpusModule.tempParaNum != 0))
        {
            g_globalThead.corpusModule.corpusM->paraNum = g_globalThead.corpusModule.tempParaNum;
        }
        g_globalThead.corpusModule.tempParaNum = 0;

        unsigned int hash = CorpusGetHash(g_globalThead.corpusModule.corpusM->corpusNum);
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->hash = hash;
        
        g_globalThead.corpusModule.corpusM->corpusNum++;

        // 如果读取的样本超过上限，打印告警，应该考虑删除样本或者增大上限
        if (g_globalThead.corpusModule.corpusM->corpusNum >= g_global.maxCorpusnum)
        {
            hw_printf("\t the corpus number is max by reading the corpus file!!!!\n");
        }
        return;
    }

    if (txt[0] == '-')
    {
        g_globalThead.corpusModule.tempParaNum++;
        return;
    }

    if ((txt[0] == 'n') && (g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum] == NULL))
    {
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum] = (S_corpus*)HwMalloc(sizeof(S_corpus));
        HwMemset(g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum], 0, sizeof(S_corpus));
    }

    tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->para[g_globalThead.corpusModule.tempParaNum];
    
    if (txt[0] == 'n')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        HwMemcpy(tempCorpus->name, g_globalThead.corpusModule.txt1, Instrlen(g_globalThead.corpusModule.txt1) + 1);
    }
    if (txt[0] == 't')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        tempCorpus->type = InGetTypeFromString(g_globalThead.corpusModule.txt1);
    }
    if (txt[0] == 'i')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        tempCorpus->inType = InGetInTypeFromString(g_globalThead.corpusModule.txt1);
    }
    if (txt[0] == 'l')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        tempCorpus->len = Inatol(g_globalThead.corpusModule.txt1);
    }
    if ((txt[0] == 'm') && (txt[1] == 'a'))
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        tempCorpus->maxLen = Inatol(g_globalThead.corpusModule.txt1);
    }
    if ((txt[0] == 'm') && (txt[1] == 'u'))
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        HwMemcpy(tempCorpus->mutaterName, g_globalThead.corpusModule.txt1, Instrlen(g_globalThead.corpusModule.txt1) + 1);
    }

    if (txt[0] == 'v')
    {
        if (txt[5] == 'N')
        {
            GetValueString(txt, g_globalThead.corpusModule.txt1);
            InDelSpace(g_globalThead.corpusModule.txt1);
            tempCorpus->value = HwMalloc(tempCorpus->len);
            s64 tempNumber = 0;

            if (txt[6] == 'A')
            {
                tempNumber = Inatol(g_globalThead.corpusModule.txt1);
            }
            if (txt[6] == 'X')
            {
                tempNumber = Inhtol(g_globalThead.corpusModule.txt1);
            }

            // 有无符号分别对待
            if (tempCorpus->type == ENUM_NUMBER_S)
            {
                if (tempCorpus->len == 1)
                {
                    *((s8 *)tempCorpus->value) = (s8)tempNumber;
                }
                else if (tempCorpus->len == 2)
                {
                    *((s16 *)tempCorpus->value) = (s16)tempNumber;
                }
                else if (tempCorpus->len == 4)
                {
                    *((s32*)tempCorpus->value) = (s32)tempNumber;
                }
                else if (tempCorpus->len == 8)
                {
                    *((s64*)tempCorpus->value) = (s64)tempNumber;
                }
            }
            else // if (tempCorpus->type == ENUM_NUMBER_U)
            {
                if (tempCorpus->len == 1)
                {
                    *((u8 *)tempCorpus->value) = (u8)tempNumber;
                }
                else if (tempCorpus->len == 2)
                {
                    *((u16 *)tempCorpus->value) = (u16)tempNumber;
                }
                else if (tempCorpus->len == 4)
                {
                    *((u32 *)tempCorpus->value) = (u32)tempNumber;
                }
                else if (tempCorpus->len == 8)
                {
                    *((u64 *)tempCorpus->value) = (u64)tempNumber;
                }
            }
        }
        else
        {
            GetValueString(txt, g_globalThead.corpusModule.txt1);
            if (tempCorpus->len > 0)
            {
                tempCorpus->value = HwMalloc(tempCorpus->len);
                InParseStringToBin(g_globalThead.corpusModule.txt1, tempCorpus->value);
            }
            else
            {
                tempCorpus->value = NULL;
            }
        }
    }

    if (txt[0] == 'w')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->weight = Inatol(g_globalThead.corpusModule.txt1);
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->mutatorCount 
            = g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->weight * g_global.mutatorCountFromWeight;

        // 权重高的样本记录起来供优先调度
        CorpusPriorityAdd(g_globalThead.corpusModule.corpusM->corpusNum, g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->weight);
    }

    if (txt[0] == 'r')
    {
        GetValueString(txt, g_globalThead.corpusModule.txt1);
        InDelSpace(g_globalThead.corpusModule.txt1);
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.corpusM->corpusNum]->randomSeed = Inatol(g_globalThead.corpusModule.txt1);
    }
}

// 从样本文件中读取样本到全局变量中
void ReadAllCorpus(char *path) 
{
    char* data = NULL;
    int len;
    int i = 0;
    int j = 0;
    
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
        g_globalThead.corpusModule.txt[j++] = data[i];

        // 遇到\n，开始解析一行
        if (data[i] == '\n')
        {
            g_globalThead.corpusModule.txt[j] = 0;
            ParseOneLine(g_globalThead.corpusModule.txt);
            j = 0;
        }
    }
    // 最后一行
    g_globalThead.corpusModule.txt[j] = 0;
    ParseOneLine(g_globalThead.corpusModule.txt);

    if (data != NULL)
    {
        HwFree(data);
        data = NULL;
    }
    return; 
}

void ReadAllBinCorpus(char *dirPath) 
{
    if (dirPath == NULL)
    {
        return;
    }
    
    int ret = OpenDir(dirPath);
    if (ret == ENUM_NO)
    {
        return;
    }

    char *binPath;
    char temp[1024];

    for (;;)
    {
        SPara* tempCorpus;
        
        binPath = ReadDir();

        if (binPath == NULL)
        {
            CloseDir();
            return;
        }

        if (binPath[0] == '.')
        {
            continue;
        }
        
        temp[0] = 0;
        hw_sprintf(temp, "%s/%s", dirPath, binPath);

        // 一次一个
        int len;
        char *data;
        ReadFromFile(&data, &len, temp);

        // 不能超过最大长度
        if (g_global.binMaxLen < len)
        {
            len = g_global.binMaxLen;
        }

        // 最开始应该做去重操作的

        g_globalThead.corpusModule.corpusM->corpusBin[g_globalThead.corpusModule.corpusM->corpusBinNum] = (S_corpus*)HwMalloc(sizeof(S_corpus));
        HwMemset(g_globalThead.corpusModule.corpusM->corpusBin[g_globalThead.corpusModule.corpusM->corpusBinNum], 0, sizeof(S_corpus));

        // 就支持一个参数
        tempCorpus = &g_globalThead.corpusModule.corpusM->corpusBin[g_globalThead.corpusModule.corpusM->corpusBinNum]->para[0];
        HwMemcpy(tempCorpus->name, "BinCorpus", Instrlen("BinCorpus") + 1);
        tempCorpus->type = InGetTypeFromString((char*)"Blob");
        tempCorpus->inType = InGetInTypeFromString((char*)"Blob");
        tempCorpus->len = len;
        tempCorpus->maxLen = g_global.binMaxLen;
        HwMemcpy(tempCorpus->mutaterName, "BinCorpus", Instrlen("BinCorpus") + 1);
        tempCorpus->value = data;

        g_globalThead.corpusModule.corpusM->corpusBinNum++;

        // 如果读取的样本超过上限，打印告警，应该考虑删除样本或者增大上限
        // 到上限直接退出，不玩了
        if (g_globalThead.corpusModule.corpusM->corpusBinNum >= g_global.maxCorpusnum)
        {
            hw_printf("\t the corpusBin number is max by reading the corpus file!!!!\n");
            return;
        }
    }
    
        return; 
}


// 得到一个参数的完整打印
static int GetParaPrint(SPara* tempCorpus, char *buf)
{
    int size = 0;

    size += hw_sprintf(buf + size, "name\t\t=%s\r\n", tempCorpus->name);
    size += hw_sprintf(buf + size, "type\t\t=%s\r\n", InGetStringFromType(tempCorpus->type));
    size += hw_sprintf(buf + size, "in_type\t\t=%s\r\n", InGetStringFromInType(tempCorpus->inType));
    size += hw_sprintf(buf + size, "mutater_name\t=%s\r\n", tempCorpus->mutaterName);
    size += hw_sprintf(buf + size, "len\t\t=%d\r\n", tempCorpus->len);
    size += hw_sprintf(buf + size, "max_len\t\t=%d\r\n", tempCorpus->maxLen);

    // 增加对number的数字打印
    if (tempCorpus->inType == ENUM_IN_NUMBERU8)
    {
        u8 tempValue = *((u8 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%u,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERU16)
    {
        u16 tempValue = *((u16 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%u,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERU32)
    {
        u32 tempValue = *((u32 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%u,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERU64)
    {
        u64 tempValue = *((u64 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%llu,0x%llx\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERS8)
    {
        s8 tempValue = *((s8 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%d,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERS16)
    {
        s16 tempValue = *((s16 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%d,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERS32)
    {
        s32 tempValue = *((s32 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%d,0x%x\r\n", tempValue, tempValue);
    }
    if (tempCorpus->inType == ENUM_IN_NUMBERS64)
    {
        s64 tempValue = *((s64 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%ld,0x%lx\r\n", tempValue, tempValue);
    }
    if ((tempCorpus->inType == ENUM_IN_NUMBER_FLOAT) ||(tempCorpus->inType == ENUM_IN_FLOAT32))
    {
#ifndef __KERNEL__
        float tempValue = *((float *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%e,0x%x\r\n", tempValue, (u32)tempValue);
#endif
    }
    if ((tempCorpus->inType == ENUM_IN_NUMBER_DOUBLE) 
        || (tempCorpus->inType == ENUM_IN_FLOAT64)
        || (tempCorpus->inType == ENUM_IN_DOUBLE))
    {
#ifndef __KERNEL__
        double tempValue = *((double *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%e,0x%lx\r\n", tempValue, (s64)tempValue);
#endif
    }
    if ((tempCorpus->inType == ENUM_IN_NUMBER_ENUM)
        || (tempCorpus->inType == ENUM_IN_NUMBER_ENUM_EX)
        || (tempCorpus->inType == ENUM_IN_NUMBER_RANGE)
        || (tempCorpus->inType == ENUM_IN_NUMBER_RANGE_EX))
    {
        s32 tempValue = *((s32 *)tempCorpus->value);
        size += hw_sprintf(buf + size, "#number_value\t=%d,0x%x\r\n", tempValue, tempValue);
    }

    g_globalThead.corpusModule.txt[0]=0;
    InParseBinToHexString(g_globalThead.corpusModule.txt, tempCorpus->value, tempCorpus->len);
    
    size += hw_sprintf(buf + size, "%s", g_globalThead.corpusModule.txt);

    g_globalThead.corpusModule.txt[0]=0;
    InParseBinToString(g_globalThead.corpusModule.txt, tempCorpus->value, tempCorpus->len);
    
    size += hw_sprintf(buf + size, "%s", g_globalThead.corpusModule.txt);

    return size;
}

// 得到一个样本的完整打印
static int GetCorpusPrint(int pos, char *buf)
{
    int size = 0;
    int i;

    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {   
        SPara* temp_corpus = &g_globalThead.corpusModule.corpusM->corpus[pos]->para[i];

        size += GetParaPrint(temp_corpus, buf + size);
        size += hw_sprintf(buf + size, "------------para=%d\r\n", i);
    }
    size += hw_sprintf(buf + size, "weight\t\t=%d\r\n", g_globalThead.corpusModule.corpusM->corpus[pos]->weight);
    size += hw_sprintf(buf + size, "randomseed\t=%d\r\n", g_globalThead.corpusModule.corpusM->corpus[pos]->randomSeed);
    size += hw_sprintf(buf + size, "!!!!!!!!!!!!!!!!!!!!!!!!!!!above is corpus %d, time is %s\r\n", g_globalThead.corpusModule.corpusM->corpusIONumber++ + 1, HwGetDate());
    
    return size;
}

void CorpusBinPrintf(char *path)
{
    char* data = NULL;
    int len;

    InitCorpusMalloc();
    
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

    InParseBinToString(g_globalThead.corpusModule.txt, data, len);
    hw_printf("%s", g_globalThead.corpusModule.txt);

    if (data != NULL)
    {
        HwFree(data);
    }
    
        return; 
}

void CorpusBinWrite(char *path)
{
    char* data = NULL;
    int len;
    char tempPath[500];

    hw_sprintf(tempPath, "%s%s", path, ".txt");

    InitCorpusMalloc();
    
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

    InParseBinToString(g_globalThead.corpusModule.txt, data, len);

    WriteToFile(g_globalThead.corpusModule.txt, Instrlen(g_globalThead.corpusModule.txt)+1, tempPath);

    if (data != NULL)
    {
        HwFree(data);
    }
        return; 
}

void CorpusCorpusWrite(char *path)
{
    char* data = NULL;
    int len;
    char tempPath[500];

    hw_sprintf(tempPath, "%s%s", path, ".bin");

    InitCorpusMalloc();
    
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

    GetValueString(data, g_globalThead.corpusModule.txt1);
    len = InParseStringToBin(g_globalThead.corpusModule.txt1, g_globalThead.corpusModule.txt);

    WriteToFile(g_globalThead.corpusModule.txt, len, tempPath);

    if (data != NULL)
    {
        HwFree(data);
    }
        return; 
}

int CorpusShowCur(void)
{
    int i;
    int isHasPara = 0;

    int size;
    char *txt;
    char crashName[MAX_FILE_PATH + 100];
    int crashNameSize;

#ifdef SUPPORT_M_THREAD 
    return 0; // 多线程暂时不支持这个，因为在信号处理程序没办法打印线程变量
#endif

    // 不在一个测试用例里运行两次这东东
    if (g_globalThead.isNeedRecordCrash == 2)
    {
        return g_globalThead.corpusModule.corpusM->paraNum;
    }

    // !!!
    if (g_globalThead.isNeedRecordCrash == 1)
    {
        g_globalThead.isNeedRecordCrash = 2;
    }

    // 没有变异数据就直接退出
    if (g_globalThead.corpusModule.corpusM->paraNum <= 0)
    {
        return g_globalThead.corpusModule.corpusM->paraNum;
    }

    g_globalThead.isPass = 0;

    crashName[0] = 0;

    txt = HwMalloc(MAX_ONE_LINE);

    size =hw_sprintf(txt, "\r\n*************************** crash corpus is \r\n");
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {   
        if (g_Element[i].element != NULL)
        {
            isHasPara = 1;
            SPara* temp_corpus = &g_Element[i].element->para;

            g_globalThead.corpusModule.txt1[0] = 0;
            GetParaPrint(temp_corpus, g_globalThead.corpusModule.txt1);

            size += hw_sprintf(txt + size, "%s", g_globalThead.corpusModule.txt1);
            size += hw_sprintf(txt + size, "------------para=%d\r\n", i);
        }
    }

    size += hw_sprintf(txt + size, "!!!!!!!!!!!!!!!!!!!!!!!!!!!above is crash corpus\r\n");

    if (g_global.isPrintfCrashCorpus)
    {
        hw_printf("%s", txt);
    }

    if ((g_globalThead.isNeedRecordCrash != 0) && (g_globalThead.isNeedRecordCorpus != 0) && isHasPara)
    {
        crashNameSize = hw_sprintf(crashName, "%s_crash", g_globalThead.corpusModule.corpusM->corpusPath);
#ifdef HAS_HOOK
                // 得到asan报告pc指针，这样一堆报告和一堆crash样本就能一一对应了
                int * pc = 0;
                if (__asan_get_report_pc)
                {
                    pc =__asan_get_report_pc();
                }
                if (pc)
                {
                    crashNameSize += hw_sprintf(crashName + crashNameSize, "_%p", pc);
                }
                else
                {
                    crashNameSize += hw_sprintf(crashName + crashNameSize, "_%d", g_global.pcNo);
                }
#endif
        crashNameSize += hw_sprintf(crashName + crashNameSize, "_crc%x", ElementGetHash());

        // 是否写入crash文件
        WriteToFile(txt, size, crashName);

        // 最后挣扎的时候，打印覆盖率文件:)
        LlvmDumpCoverage();
    }
    
    HwFree(txt);

    // 新移动过来，需要观察
    ReportWriteFailedTestCase1();


    //调用用户注册的回调函数
    CallBackRunCrash();

    return g_globalThead.corpusModule.corpusM->paraNum;
}

int CorpusAsanReportWrite(char * report)
{
    int reportNameSize;
    
    if (report == NULL)
    {
        return 1;
    }

#ifdef SUPPORT_M_THREAD 
        return 0; // 多线程暂时不支持这个，因为在信号处理程序没办法打印线程变量
#endif

    //如果已经写过，就写到文件尾
    if (g_globalThead.asanReportName[0] != 0)
    {
        int tempLen = strlen(report);

        if (g_globalThead.asanReportLen > tempLen)
        {
            hw_printf("\t Print asan report error, please notice!!!!\n");
            return 0;
        }
            
        WriteToFileFail(report + g_globalThead.asanReportLen, tempLen - g_globalThead.asanReportLen, g_globalThead.asanReportName);

        g_globalThead.asanReportLen = tempLen;
        return 0;
    }
    
    // 这个回调，可以记录asan报告
    reportNameSize = hw_sprintf(g_globalThead.asanReportName, "%s_crash", g_globalThead.corpusModule.corpusM->corpusPath);
#ifdef HAS_HOOK
    // 得到asan报告pc指针，这样一堆报告和一堆crash样本就能一一对应了
    int * pc = 0;
    if (__asan_get_report_pc)
    {
        pc =__asan_get_report_pc();
    }
    if (pc)
    {
        reportNameSize += hw_sprintf(g_globalThead.asanReportName + reportNameSize, "_%p", pc);
    }
    else 
    {
        g_global.pcNo++;
        reportNameSize += hw_sprintf(g_globalThead.asanReportName + reportNameSize, "_%d", g_global.pcNo);
    }
#endif

    reportNameSize += hw_sprintf(g_globalThead.asanReportName + reportNameSize, "_asan");

    int tempLen = strlen(report);
    if (g_globalThead.asanReportLen > tempLen)
    {
        hw_printf("\t Print asan report error, please notice!!!!\n");
        return 0;
    }

    // 是否写入crash文件
    WriteToFile(report + g_globalThead.asanReportLen, tempLen - g_globalThead.asanReportLen, g_globalThead.asanReportName);

    g_globalThead.asanReportLen = tempLen;
    return 0;
}


void CorpusShowAll()
{
    int pos; 
    char *txt;

    if (g_global.isNeedShowCorpus == 0)
    {
        return;
    }

    if (g_globalThead.corpusModule.corpusM->corpusNum <= 0)
    {
        return;
    }

    txt = HwMalloc(MAX_ONE_LINE);

    // 一个样本打印一次，节省内存
    for (pos = 0; pos < g_globalThead.corpusModule.corpusM->corpusNum; pos++)
    {
        txt[0] = 0;
        GetCorpusPrint(pos, txt);
        hw_printf("%s", txt);
    }
    
    HwFree(txt);
}

void CorpusWrite(int pos)
{
    int size;
    char *txt;
    txt = HwMalloc(MAX_ONE_LINE);
    txt[0] = 0;
    size = GetCorpusPrint(pos, txt);
    WriteToFileFail(txt, size, g_globalThead.corpusModule.corpusM->corpusPath);

    HwFree(txt);
}

void CorpusWriteBin(int pos)
{
    int len;
    char *buf;
    char outname[1024];
    unsigned int hash;

    CorpusGetValueAndLen(&buf,&len,pos);
    hash = HwCheckSum32((unsigned char *)buf, len);

    outname[0] = 0;
    hw_sprintf(outname, "%s/binCorpus_crc%x", g_global.binCorpusDirOut, hash);

    // 写文件
    WriteToFile(buf, len, outname);

    HwFree(buf);
}


void CorpusSetPath(char* path)
{
    g_globalThead.corpusModule.corpusM->corpusPath = path;
}

void CorpusSetIfShowCrash(int isShowAll)
{
    g_global.isPrintfCrashCorpus = isShowAll;
}

void CorpusSetIfShow(int isShowAll)
{
    g_global.isNeedShowCorpus = isShowAll;
}

#ifdef __cplusplus
}
#endif

