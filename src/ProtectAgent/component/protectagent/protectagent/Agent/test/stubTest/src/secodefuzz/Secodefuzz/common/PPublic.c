/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180
wangchengyun 	wwx412654

本文件函数仅仅为对外接口的简单二次封装
为了用户调用方便
禁止体现核心逻辑

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_M_THREAD 
__thread int fuzzSeed;
__thread int fuzzi = 0;
__thread int fuzzStart;
__thread int fuzzEnd;
__thread SElementInit g_Element[MAX_PARA_NUM] = {{0}};
__thread int fuzzPid;
__thread int fuzzStatus = 0;
__thread int g_isHasNewFeature = 0;

#else
int fuzzSeed;
int fuzzi = 0;
int fuzzStart;
int fuzzEnd;
SElementInit g_Element[MAX_PARA_NUM] = {{0}};
int fuzzPid;
int fuzzStatus = 0;
int g_isHasNewFeature = 0; 

#endif

// 清理变异环境，更换样本的时候被调用
void ClearFuzzEnvironment (void)
{
    int i = 0;
    
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        if (g_Element[i].isNeedFree == ENUM_YES)
        {
            HwFree(g_Element[i].initValueBuffer);
            g_Element[i].initValueBuffer = NULL;
            g_Element[i].isNeedFree = ENUM_NO;
        }
        
        if (g_Element[i].element)
        {
            FreeMutatedValue(g_Element[i].element);
            FreeElement(g_Element[i].element);
            g_Element[i].element = NULL;
        }
    }
    HwMemset(g_Element, 0, sizeof(SElementInit) * g_globalThead.corpusModule.corpusM->paraNum);
    g_globalThead.wholeRandomNum = 0;
}

void DTStart(int seed, int count, char* testcase_name, int is_reproduce)
{
    fuzzSeed = seed;
    InitCommon(); 
    CorpusSetPath (testcase_name);
    ReportSetRunningTestCaseName(testcase_name);
    g_globalThead.temp_is_reproduce = is_reproduce;
    g_globalThead.runTime = ReportGetTime();
    g_globalThead.testcaseName = testcase_name;
    g_globalThead.isfoundfailed = 0;
    g_globalThead.isPass = 1;
    g_globalThead.gostop = 0;
    if ((is_reproduce) && (is_reproduce != (int)0xffffffff))
    {
        fuzzStart = 0;
        fuzzEnd = 1;
    }
    else
    {
        fuzzStart = 0;
        fuzzEnd = count;
    }

    RunningTimeGetStart();
}

void DTForStart(void)
{
    int is_reproduce = g_globalThead.temp_is_reproduce;
    InitSeed(fuzzSeed, fuzzi);
    CorpusStart(is_reproduce);
    TimeOutset();

    // 每次赋值
    if ((is_reproduce) && (is_reproduce != (int)0xffffffff))
    {
        g_globalThead.isNeedRecordCrash = 0;
    }
    else
    {
        g_globalThead.isNeedRecordCrash = 1;
    }

    g_globalThead.asanReportName[0] = 0;

    g_globalThead.isRunningTestCode = 1;
    g_global.mallocs = 0;
    g_global.frees = 0;

    if (g_global.enableDoLeakCheck)
    {
        LlvmEnableLeakCheck(1000);
    }
}

void DTForEnd(void)
{
    g_globalThead.isRunningTestCode = 0;
    
    TimeOutClean();
    LlvmDo8BitCounters();
    //这行代码必须与上边紧紧相连，防止其他代码引入到被测代码路径计算
    int isHasNewFeature = CorpusEnd();
    g_isHasNewFeature = isHasNewFeature;
    LlvmDoLeakCheck();

    // 次数重启
    if ((isHasNewFeature == 1) && ((g_global.isResetOnNewCorpus == 1) || (g_global.isResetOnNewCorpus == 3)))
    {
        fuzzi = fuzzStart;
    }

    // 运行时间重启
    if ((isHasNewFeature == 1) && ((g_global.isResetOnNewCorpus == 2) || (g_global.isResetOnNewCorpus == 3)))
    {
        RunningTimeGetStart();
    }
    
    // 测试例退出，清理测试例名字
    //ReportSetRunningTestCaseName(0);
}

void DTEnd(void)
{
    g_globalThead.runTime = ReportGetTime() - g_globalThead.runTime;
    LlvmDumpCoverage();
    if (g_globalThead.temp_is_reproduce == 0)
    {
        //运行次数完成或者时间到了，同样执行成功,并且没有发现问题
        if(((fuzzi == fuzzEnd) || (g_globalThead.isRunningtimeComplete = 1)) && (g_globalThead.isfoundfailed == 0))
        {
            ReportWriteSucceedTestCase(
                g_globalThead.testcaseName, fuzzSeed, fuzzi, g_globalThead.runTime, CorpusGetReadCorpusNum(), CorpusGetNewCorpusNum());
        }
        else
        {
            ReportWriteFailedTestCase1();
        }
    }
    ReportSetRunningTestCaseName(NULL);
    CorpusShowAll();
    ClearFuzzEnvironment ();

    // 结束了就清了这个样本，要不exit会有打印
    CleanCorpus();

    g_global.binCorpusDirOut = NULL;
    g_global.binCorpusDirIn = NULL;
    g_global.binMaxLen = 0;

    //调用用户注册的回调函数
    CallBackRunTestCase();
}

int DTStop(void)
{
    return g_globalThead.gostop;
}

int DTFork(void)
{
    int pid;
    
    if (g_global.isEnableFork == 0)
    {
        return 0;
    }    

    //fork模式，主进程需要设置isPass，因为主进程不运行DTStart
    g_globalThead.isPass = 1;

    pid = HwFork ();
    return pid;
}

void DTWait(char*tempTestCaseName)
{

    if (g_global.isEnableFork == 0)
    {
        return;
    } 
    
    fuzzStatus = HwWait(); 
    
    if (fuzzStatus != 0) 
    {
        hw_printf("!!!!(fork)test %s interrupt,status is %d\r\n", tempTestCaseName, HwWEXITSTATUS(fuzzStatus));
        g_globalThead.isPass = 0;
    }
    else
    {
        hw_printf("!!!!(fork)test %s process complete \r\n", tempTestCaseName);
    }

}

void DTExit(int no)
{
    if (g_global.isEnableFork == 0)
    {
        return;
    } 
    int error = 0;

    if (no == 0)
    {
        error = EXIT_CODE_OTHER;
        hw_printf("\r\nChild proess error has occur !!!\r\n");
    }
    
    HwExit(error);
}

static int FuzzCreateElement(SElementInit *pInit)
{
    SElement *p = NULL;
    switch (pInit->type)
    {
        case ENUM_STRING:
            p = CreatElementString(pInit->isHasInitValue, pInit->initValueBuffer, pInit->len, pInit->maxLen);
            break;

        case ENUM_STRING_NUM:
            p = CreatElementStringNum(pInit->isHasInitValue, pInit->initValueBuffer, pInit->len, pInit->maxLen);
            break;

        case ENUM_STRING_ENUM:
            p = CreatElementStringEnum(pInit->isHasInitValue, 
                pInit->initValueBuffer, pInit->len, pInit->maxLen, pInit->enumStringTable, pInit->enumCount);
            break;
            
        case ENUM_BLOB:
            p = CreatElementBlob(pInit->isHasInitValue, pInit->initValueBuffer, pInit->len, pInit->maxLen);
            break;

        case ENUM_BLOB_ENUM:
            p = CreatElementBlobEnum(pInit->isHasInitValue, 
                pInit->initValueBuffer, pInit->len, pInit->maxLen, pInit->enumBlobTable, pInit->enumBloblTable, pInit->enumCount);
            break;

        case ENUM_FIXBLOB:
            p = CreatElementFixBlob(pInit->isHasInitValue, pInit->initValueBuffer, pInit->len, pInit->maxLen);
            break;

        case ENUM_IPV4:
            p = CreatElementIpv4(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_IPV6:
            p = CreatElementIpv6(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_MAC:
            p = CreatElementMac(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_FLOAT16:
            p = CreatElementFloat16(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_FLOAT32:
            p = CreatElementFloat32(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_FLOAT64:
            p = CreatElementFloat64(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_DOUBLE:
            p = CreatElementDouble64(pInit->isHasInitValue, pInit->initValueBuffer);
            break;

        case ENUM_TSELF:
            p = CreatElementSelf(pInit->isHasInitValue, pInit->initValueBuffer, pInit->len, pInit->maxLen, pInit->arg);
            break;
            
        case ENUM_AFL:
            break;
            
        case ENUM_NUMBER_U:
            if (pInit->len <= 8)
            {
                p = CreatElementU8(pInit->isHasInitValue, (u8)pInit->initValue);
            }
            else if (pInit->len <= 16)
            {
                p = CreatElementU16(pInit->isHasInitValue, (u16)pInit->initValue);
            }
            else if (pInit->len <= 32)
            {
                p = CreatElementU32(pInit->isHasInitValue, (u32)pInit->initValue);
            }
            else if (pInit->len <= 64)
            {
                p = CreatElementU64(pInit->isHasInitValue, (u64)pInit->initValue);
            }
            break;
        case ENUM_NUMBER_S:
            if (pInit->len <= 8)
            {
                p = CreatElementS8(pInit->isHasInitValue, (s8)pInit->initValue);
            }
            else if (pInit->len <= 16)
            {
                p = CreatElementS16(pInit->isHasInitValue, (s16)pInit->initValue);
            }
            else if (pInit->len <= 32)
            {
                p = CreatElementS32(pInit->isHasInitValue, (s32)pInit->initValue);
            }
            else if (pInit->len <= 64)
            {
                p = CreatElementS64(pInit->isHasInitValue, (s64)pInit->initValue);
            }
            break;
        case ENUM_NUMBER_ENUM:
            p = CreatElementNumberEnum(pInit->isHasInitValue, (s32)pInit->initValue, 
                pInit->enumNumberTable, pInit->enumCount);
            break;
        case ENUM_NUMBER_RANGE:
            p = CreatElementNumberRange(pInit->isHasInitValue, (s32)pInit->initValue, pInit->min, pInit->max);
            break;
        default:
            hw_printf("do not find type of the element\n");
            ASSERT(1);
            return -1;  
    }
    pInit->element = p;
    p->para.inType = pInit->inType; // 新增
    return 0;
}

char*  DTGetFuzzValueEx(SElementInit *pInit)
{
    int first = 0;  // 第一次调迭代不变异控制
    char *src = NULL;

    ASSERT_NULL(pInit);

    if (pInit[0].first != 1)
    {
        pInit[0].first = 1;
        first = 1;
        
        if (pInit[0].len > 0)
        {
            pInit[0].isHasInitValue = ENUM_YES;         // 调用此接口都有初始值,谁说的
        }
        else
        {
            pInit[0].isHasInitValue = ENUM_NO;
        }
        FuzzCreateElement(&pInit[0]);
        AddWholeRandom(pInit[0].element);
    }
        
    // free
    if (pInit[0].element)
    {
        FreeMutatedValue(pInit[0].element);
    }

    // 如果在执行样本本身，则不变异     
    if ((first) || (g_globalThead.isNeedMutator == 0))
    {
        src = pInit[0].element->inBuf;
        pInit[0].element->para.value = src;
        pInit[0].element->para.len = pInit[0].element->inLen / 8;
        pInit[0].element->pos = POS_ORIGINAL;
        HwMemcpy(pInit[0].element->para.mutaterName, "default value", Instrlen("default value") +1);
    }
    else
    {
        src = GetMutatedValueRandom(pInit[0].element);
    }
    return src;
}

char *DT_SetGetS64(SElementInit *init, s64 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBERS64;
        init->len           = 64;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetS32(SElementInit *init, s32 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBERS32;
        init->len           = 32;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetS16(SElementInit *init, s16 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBERS16;
        init->len           = 16;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetS8(SElementInit *init, s8 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBERS8;
        init->len           = 8;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetU64(SElementInit *init, u64 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBERU64;
        init->len           = 64;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetU32(SElementInit *init, u32 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBERU32;
        init->len           = 32;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetU16(SElementInit *init, u16 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBERU16;
        init->len           = 16;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetU8(SElementInit *init, u8 initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBERU8;
        init->len           = 8;
        init->initValue     = initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetNumberEnum(SElementInit *init, s32 initValue, int* eunmTable, int  eunmCount)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_ENUM;
        init->inType       = ENUM_IN_NUMBER_ENUM;
        init->len           = 32;
        init->initValue     = initValue;

        init->enumNumberTable     = eunmTable;
        init->enumCount    = eunmCount;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetNumberEnum_EX(SElementInit *init, s32 initValue, int* eunmTable, int  eunmCount)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBER_ENUM_EX;
        init->len           = 32;
        init->initValue     = eunmTable[RAND_32()%eunmCount];

        init->enumNumberTable     = eunmTable;
        init->enumCount    = eunmCount;
    }
    
    char* aaa = DTGetFuzzValueEx(init);

    int i;
    // 初值一定不要在枚举里
    // 判断如果变异出来的数据在枚举里，则换为初值
    for (i = 0; i < eunmCount; i++)
    {
        if (*((s32 *)aaa) == eunmTable[i])
        {
            *((s32 *)aaa) = initValue;
        }
    }
    
    return aaa;
}

char *DT_SetGetNumberRange(SElementInit *init, s32 initValue, int min, int  max)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_RANGE;
        init->inType       = ENUM_IN_NUMBER_RANGE;
        init->len           = 32;
        init->initValue     = initValue;

        init->min   = min;
        init->max   = max;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetNumberRange_EX(SElementInit *init, s32 initValue, int min, int  max)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_S;
        init->inType       = ENUM_IN_NUMBER_RANGE_EX;
        init->len           = 32;
        init->initValue     = RAND_RANGE(min, max);
    }

    char* aaa = DTGetFuzzValueEx(init);

    // 初值一定不要在枚举里
    // 判断如果变异出来的数据在范围里，则换为初值
    if (((*((s32 *)aaa)) >= min) && ((*((s32 *)aaa)) <= max))
    {
        *((s32 *)aaa) = initValue;
    }
    
    return aaa;
}


char *DT_SetGetFloat(SElementInit *init, float initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBER_FLOAT;
        init->len           = 32;
        init->initValue     = *(u32 *)&initValue;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetDouble(SElementInit *init, double initValue)
{
    if (init->first != 1)
    {
        init->type           = ENUM_NUMBER_U;
        init->inType       = ENUM_IN_NUMBER_DOUBLE;
        init->len             = 64;
        init->initValue     = *(u64 *)&initValue;
    }

    return DTGetFuzzValueEx(init);
}

void DTInitValueBuffer(SElementInit *init, int length, char* initValue)
{
    if (length > 0)
    {
        init->isNeedFree = ENUM_YES;
        init->initValueBuffer = HwMalloc(length);
        HwMemcpy(init->initValueBuffer, initValue, length);
    }
}

char *DT_SetGetString(SElementInit *init, int length, int maxLength, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_STRING;
        init->inType     = ENUM_IN_STRING;
        init->len           = length;       // 字符串长度，strlen(str) + 1
        init->maxLen    = maxLength;
        DTInitValueBuffer(init, length, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetStringNum(SElementInit *init, int length, int maxLength, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_STRING_NUM;
        init->inType     = ENUM_IN_STRING_NUM;
        init->len           = length;       // 字符串长度，strlen(str) + 1
        init->maxLen    = maxLength;
        DTInitValueBuffer(init, length, initValue);
    }

    return DTGetFuzzValueEx(init);
}


char *DT_SetGetStringEnum(
    SElementInit *init, int length, int maxLength, char*  initValue, char* eunmTableS[], int eunmCount)
{
    if (init->first != 1)
    {
        init->type          = ENUM_STRING_ENUM;
        init->inType       = ENUM_IN_STRING_ENUM;
        init->len           = length;       // 字符串长度，strlen(str) + 1
        init->maxLen    = maxLength;
        DTInitValueBuffer(init, length, initValue);

        init->enumStringTable     = eunmTableS;
        init->enumCount    = eunmCount;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetStringEnum_EX(
    SElementInit *init, int length, int maxLength, char*  initValue, char* eunmTableS[], int eunmCount)
{
    if (init->first != 1)
    {
        init->type          = ENUM_STRING;
        init->inType       = ENUM_IN_STRING_ENUM_EX;
        init->len           = length;       // 字符串长度，strlen(str) + 1
        init->maxLen    = maxLength;

        int pos = RAND_32() % eunmCount;
        DTInitValueBuffer(init, Instrlen(eunmTableS[pos]) + 1, eunmTableS[pos]);
        init->len = Instrlen(eunmTableS[pos]) + 1;

        init->enumStringTable     = eunmTableS;
        init->enumCount    = eunmCount;
    }

    DTGetFuzzValueEx(init);

    int i;
    // 初值一定不要在枚举里
    // 判断如果变异出来的数据在枚举里，则换为初值
    for (i = 0; i < eunmCount; i++)
    {
        if ((Instrlen(eunmTableS[i]) + 1) == (u32)init->element->para.len)
        {
            if (HwMemCmp(eunmTableS[i], init->element->para.value, init->element->para.len) == 0)
            {
                if (init->element->isNeedFreeOutBuf == ENUM_YES)
                {
                    if (init->element->para.value != NULL)
                    {
                        HwFree(init->element->para.value);
                        init->element->para.value = NULL;
                        init->element->isNeedFreeOutBuf = ENUM_NO;
                    }
                }
                init->element->para.value = HwMalloc(length);
                HwMemcpy(init->element->para.value, initValue, length);
                init->element->para.len = length;
                init->element->isNeedFreeOutBuf = ENUM_YES;
            }
        }
    }

    return init->element->para.value;
}
        
char *DT_SetGetBlob(SElementInit *init, int length, int maxLength, char* initValue)
{
    if (init->first != 1)
    {
        init->type          = ENUM_BLOB;
        init->inType       = ENUM_IN_BLOB;
        init->len           = length;       
        init->maxLen    = maxLength;
        DTInitValueBuffer(init, length, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetBlobEnum(SElementInit *init, int length, int maxLength, 
    char*  initValue, char* eunmTableB[], int eunmTableL[], int eunmCount)
{
    if (init->first != 1)
    {
        init->type          = ENUM_BLOB_ENUM;
        init->inType       = ENUM_IN_BLOB_ENUM;
        init->len           = length;       
        init->maxLen    = maxLength;
        DTInitValueBuffer(init, length, initValue);

        init->enumBlobTable       = eunmTableB;
        init->enumBloblTable     = eunmTableL;
        init->enumCount              = eunmCount;
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetBlobEnum_EX(SElementInit *init, int length, int maxLength, 
    char*  initValue, char* eunmTableB[], int eunmTableL[], int eunmCount)
{
    if (init->first != 1)
    {
        init->type         = ENUM_BLOB;
        init->inType     = ENUM_IN_BLOB_ENUM_EX;
        init->len           = length;       // 字符串长度，strlen(str) + 1
        init->maxLen    = maxLength;

        int pos = RAND_32() % eunmCount;
        DTInitValueBuffer(init, eunmTableL[pos], eunmTableB[pos]);
        init->len = eunmTableL[pos];

        init->enumBlobTable       = eunmTableB;
        init->enumBloblTable     = eunmTableL;
        init->enumCount              = eunmCount;
    }

    DTGetFuzzValueEx(init);

    int i;
    // 初值一定不要在枚举里
    // 判断如果变异出来的数据在枚举里，则换为初值
    for (i = 0; i < eunmCount; i++)
    {
        if (eunmTableL[i] == init->element->para.len)
        {
            if (HwMemCmp(eunmTableB[i], init->element->para.value, init->element->para.len) == 0)
            {
                if (init->element->isNeedFreeOutBuf == ENUM_YES)
                {
                    if (init->element->para.value != NULL)
                    {
                        HwFree(init->element->para.value);
                        init->element->para.value = NULL;
                        init->element->isNeedFreeOutBuf = ENUM_NO;
                    }
                }
                init->element->para.value = HwMalloc(length);
                HwMemcpy(init->element->para.value, initValue, length);
                init->element->para.len = length;
                init->element->isNeedFreeOutBuf = ENUM_YES;
            }
        }
    }

    return init->element->para.value;
}
        
char *DT_SetGetFixBlob(SElementInit *init, int length, int maxLength, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_FIXBLOB;
        init->inType     = ENUM_IN_FIXBLOB;
        init->len           = length;       
        init->maxLen    = length; //  忽略最大长度参数，使用len
        DTInitValueBuffer(init, length, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetIpv4(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_IPV4;
        init->inType     = ENUM_IN_IPV4;
        init->len           = 4;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 4, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetIpv6(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_IPV6;
        init->inType     = ENUM_IN_IPV6;
        init->len           = 16;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 16, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetMac(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_MAC;
        init->inType     = ENUM_IN_MAC;
        init->len           = 6;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 6, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetFLoat16(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_FLOAT16;
        init->inType     = ENUM_IN_FLOAT16;
        init->len           = 2;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 2, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetFLoat32(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_FLOAT32;
        init->inType     = ENUM_IN_FLOAT32;
        init->len           = 4;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 4, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetFLoat64(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_FLOAT64;
        init->inType     = ENUM_IN_FLOAT64;
        init->len           = 8;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 8, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetDouble64(SElementInit *init, char* initValue)
{
    if (init->first != 1)
    {
        init->type         = ENUM_DOUBLE;
        init->inType     = ENUM_IN_DOUBLE;
        init->len           = 8;
        init->maxLen    = 0;
        DTInitValueBuffer(init, 8, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetSelf(SElementInit *init, int length, int maxLength, char* initValue, int arg)
{
    if (init->first != 1)
    {
        init->type         = ENUM_TSELF;
        init->inType     = ENUM_IN_SELF;
        init->len           = length;
        init->maxLen    = maxLength;
        init->arg           = arg;
        DTInitValueBuffer(init, length, initValue);
    }

    return DTGetFuzzValueEx(init);
}

char *DT_SetGetS64V3(int id, s64 initValue)
{
    return DT_SetGetS64(&g_Element[id], initValue);
}

char *DT_SetGetS32V3(int id, s32 initValue)
{
    return DT_SetGetS32(&g_Element[id], initValue);
}

char *DT_SetGetS16V3(int id, s16 initValue)
{
    return DT_SetGetS16(&g_Element[id], initValue);
}

char *DT_SetGetS8V3(int id, s8 initValue)
{
    return DT_SetGetS8(&g_Element[id], initValue);
}

char *DT_SetGetU64V3(int id, u64 initValue)
{
    return DT_SetGetU64(&g_Element[id], initValue);
}

char *DT_SetGetU32V3(int id, u32 initValue)
{
    return DT_SetGetU32(&g_Element[id], initValue);
}

char *DT_SetGetU16V3(int id, u16 initValue)
{
    return DT_SetGetU16(&g_Element[id], initValue);
}

char *DT_SetGetU8V3(int id, u8 initValue)
{
    return DT_SetGetU8(&g_Element[id], initValue);
}

char *DT_SetGetNumberEnumV3(int id, s32 initValue, int* eunmTable, int  eunmCount)
{
    return DT_SetGetNumberEnum(&g_Element[id], initValue, eunmTable, eunmCount);
}

char *DT_SetGetNumberEnum_EXV3(int id, s32 initValue, int* eunmTable, int  eunmCount)
{
    return DT_SetGetNumberEnum_EX(&g_Element[id], initValue, eunmTable, eunmCount);
}

char *DT_SetGetNumberRangeV3(int id, s32 initValue, int min, int max)
{
    return DT_SetGetNumberRange(&g_Element[id], initValue, min, max);
}

char *DT_SetGetNumberRange_EXV3(int id, s32 initValue, int min, int  max)
{
    return DT_SetGetNumberRange_EX(&g_Element[id], initValue, min, max);
}

char *DT_SetGetFloatV3(int id, float initValue)
{
    return DT_SetGetFloat(&g_Element[id], initValue);
}

char *DT_SetGetDoubleV3(int id, double initValue)
{
    return DT_SetGetDouble(&g_Element[id], initValue);
}

char *DT_SetGetStringV3(int id, int length, int maxLength, char* initValue)
{
    return DT_SetGetString(&g_Element[id], length, maxLength, initValue);
}

char *DT_SetGetStringNumV3(int id, int length, int maxLength, char* initValue)
{
    return DT_SetGetStringNum(&g_Element[id], length, maxLength, initValue);
}

char *DT_SetGetStringEnumV3(
    int id, int length, int maxLength, char* initValue, char* eunmTableS[], int eunmCount)
{
    return DT_SetGetStringEnum(&g_Element[id], length, maxLength, initValue, eunmTableS, eunmCount);
}

char *DT_SetGetStringEnum_EXV3(
    int id, int length, int maxLength, char*  initValue, char* eunmTableS[], int eunmCount)
{
    return DT_SetGetStringEnum_EX(&g_Element[id], length, maxLength, initValue, eunmTableS, eunmCount);
}
        
char *DT_SetGetBlobV3(int id, int length, int maxLength, char* initValue)
{
    return DT_SetGetBlob(&g_Element[id], length, maxLength, initValue);
}

char *DT_SetGetBlobEnumV3(int id, int length, int maxLength, 
    char*  initValue, char* eunmTableB[], int eunmTableL[], int eunmCount)
{
    return DT_SetGetBlobEnum(&g_Element[id], length, maxLength, initValue, eunmTableB, eunmTableL, eunmCount);
}

char *DT_SetGetBlobEnum_EXV3(int id, int length, int maxLength, 
    char*  initValue, char* eunmTableB[], int eunmTableL[], int eunmCount)
{
    return DT_SetGetBlobEnum_EX(&g_Element[id], length, maxLength, initValue, eunmTableB, eunmTableL, eunmCount);
}
        
char *DT_SetGetFixBlobV3(int id, int length, int maxLength, char* initValue)
{
    return DT_SetGetFixBlob(&g_Element[id], length, maxLength, initValue);
}

char *DT_SetGetIpv4V3(int id, char* initValue)
{
    return DT_SetGetIpv4(&g_Element[id], initValue);
}

char *DT_SetGetIpv6V3(int id, char* initValue)
{
    return DT_SetGetIpv6(&g_Element[id], initValue);
}

char *DT_SetGetMacV3(int id, char* initValue)
{
    return DT_SetGetMac(&g_Element[id], initValue);
}

char *DT_SetGetFLoat16V3(int id, char* initValue)
{
    return DT_SetGetFLoat16(&g_Element[id], initValue);
}

char *DT_SetGetFLoat32V3(int id, char* initValue)
{
    return DT_SetGetFLoat32(&g_Element[id], initValue);
}

char *DT_SetGetFLoat64V3(int id, char* initValue)
{
    return DT_SetGetFLoat64(&g_Element[id], initValue);
}

char *DT_SetGetDouble64V3(int id, char* initValue)
{
    return DT_SetGetDouble64(&g_Element[id], initValue);
}

char *DT_SetGetSelfV3(int id, int length, int maxLength, char* initValue, int arg)
{
    return DT_SetGetSelf(&g_Element[id], length, maxLength, initValue, arg);
}


void DT_Set_If_Show_crash(int isShowAll)
{
    CorpusSetIfShowCrash(isShowAll);
}

void DT_Set_If_Show_Corpus(int isShowAll)
{
    CorpusSetIfShow(isShowAll);
}

void DT_Set_Report_Path(char* path)
{
    ReportSetPath(path);
}

void DT_Set_Report_Fix_PathName(char* pathName)
{
    ReportSetFixPathName(pathName);
}

void DT_Set_Is_Dump_Coverage(int isDumpCoverage)
{
    LlvmSetIsDumpCoverage(isDumpCoverage);
}

void DT_Set_Is_Print_New_PC(int isPrintPC)
{
    LlvmSetIsPrintNewPC(isPrintPC);
}

const char *DT_Get_Version(void)
{
    return GetVersion();
}

void DT_Printf_Bin_To_Corpus(char *path)
{
    CorpusBinPrintf(path);
}

void DT_Write_Bin_To_Corpus(char *path)
{
    CorpusBinWrite(path);
}

void DT_Write_Corpus_To_Bin(char *path)
{
    CorpusCorpusWrite(path);
}

void DT_Enable_TracePC(int isEnable)
{
    if (isEnable == 0)
    {
        CorpusStartFeature();
    }
    else
    {
        CorpusEndFeature();
    }
}

void DT_Show_Cur_Corpus(void)
{
    // 我们当做发现了问题
    g_globalThead.isfoundfailed = 1;
    CorpusShowCur();
}

void DT_Enable_Log(int isEnable)
{
    if (isEnable == 0)
    {
        CloseLog();
    }
    else
    {
        OpenLog();
    }
}

int DT_GET_MutatedValueLen(SElementInit *init)
{
    return GetMutatedValueLen(init->element);
}

void DT_SET_MutatedFrequency(SElementInit *init, int frequency)
{
    return SetMutatedFrequency(init->element, frequency);
}

int DT_GET_IsBeMutated(SElementInit *init)
{
    return GetIsBeMutated(init->element);
}

int DT_GET_MutatedValueLenV3(int id)
{
    return DT_GET_MutatedValueLen(&g_Element[id]);
}

void DT_SET_MutatedFrequencyV3(int id, int frequency)
{
    return DT_SET_MutatedFrequency(&g_Element[id], frequency);
}

int DT_GET_IsBeMutatedV3(int id)
{
    return DT_GET_IsBeMutated(&g_Element[id]);
}

void DT_Enable_AllMutater(int isEnable)
{
    if (isEnable == 0)
    {
        SetCloseAllMutater();
    }
    else
    {
        SetOpenAllMutater();
    }
}

void DT_Enable_OneMutater(enum EnumMutated  mutatedNum, int isEnable)
{
    if (isEnable == 0)
    {
        SetCloseOneMutater(mutatedNum);
    }
    else
    {
        SetOpenOneMutater(mutatedNum);
    }
}

void DT_Enable_OneMutaterV3(int id, int isEnable)
{
    DT_Enable_OneMutater((enum EnumMutated)id, isEnable);
}

void DT_Enable_Support_Loop(int isEnable)
{
    // 已经默认支持了，不允许关闭了
    return;
}

void DT_Enable_Leak_Check(int isEnable, int isDebug)
{
    g_global.mallocDebug = isDebug;
    
    if (isEnable == 0)
    {
        g_global.enableDoLeakCheck = 0;
    }
    else
    {
        g_global.enableDoLeakCheck = 1;
    }
}

void DT_Set_HasNewFeature(int weight)
{
    LlvmTracePcSetHasNewFeature(weight);
}

void DT_Set_IsDeleteMismatchCopusFile(int isDelete)
{
    g_global.isDelMismatchCopusFile = isDelete;
}

void DT_Set_MCorpusNm(int num)
{
    if(num > MAX_CORPUSP_NUM)
    {
        num = MAX_CORPUSP_NUM;
    }

    if(num == 0)
    {
        num = DEFAULT_CORPOS_NUM;
    }
    
    if(num < 1)
    {
        num = 1;
    }
    g_global.maxCorpusnum = num;
}

void DT_Clear_ThreadMemory(void)
{
    ClearMemcory();
}

void DT_Set_MutatorCountFromWeight(int value)
{
    g_global.mutatorCountFromWeight = value;
}

int DT_Get_RandomSeed(void)
{
    return g_globalThead.randomSeed;
}

void DT_Set_RandomSeed(int value)
{
        HwSrand(value);
}

int DT_Get_RandomVlaue(void)
{
        return HwRand();
}


void DT_Set_Reset_On_New_Corpus(int value)
{
    g_global.isResetOnNewCorpus = value;
}

void DT_Set_Running_Time_Second(int second)
{
    RunningSetTimeSecond(second);
}

void DT_Set_TimeOut_Second(int second)
{
    TimeOutSetSecond(second);
}

void DT_Set_MaxOutputSize(int imaxOutputSize)
{
    SetMaxOutputSize(imaxOutputSize);
}

void DT_Set_String_Has_Terminal(int isHasTerminal)
{
    SetStringHasTerminal(isHasTerminal);
}

void DT_Set_String_Has_Null(int isHasNull)
{
    g_global.stringHasNull = isHasNull;
}

void DT_Enable_AsanReport(int isEnable)
{
    g_global.isEnableAsanReport = isEnable;
}

void DT_SetBinCorpusDirIn(char* dir, int maxLen)
{
    g_global.binCorpusDirIn = dir;
    g_global.binMaxLen = maxLen;
}

void DT_SetBinCorpusDirOut(char* dir)
{
    g_global.binCorpusDirOut = dir;
}

void DT_SetEnableFork(int isEnable)
{
    g_global.isEnableFork = isEnable;
}

void DT_SetDictionaryPath(char *path)
{
    g_global.dictionaryPath = path;
}

void DT_SetPathModeWeight(int mode, int weight)
{
    if (mode == 1)
    {
        g_global.blockModeWeight = weight;
    }

    if (mode == 2)
    {
        g_global.edgeModeWeight = weight;
    }

    if (mode == 3)
    {
        g_global.loopModeWeight = weight;
    }

    if (mode == 4)
    {
        g_global.hashModeWeight = weight;
    }

}

void DT_AddCrashCallBack(void (*fun)(void))
{
    CallBackAddCrash(fun);
}

void DT_AddTestCaseCallBack(void (*fun)(void))
{
    CallBackAddTestCase(fun);
}

void DT_SetCheckOutOfMemory(int mallocLimitMb, int rssLimitMb)
{
    MemoryStart(mallocLimitMb, rssLimitMb);
}

void DT_SetStopOutOfMemory(void)
{
    MemoryStop();
}

int DT_GetIsPass(void)
{
    return g_globalThead.isPass;
}

void DT_SetNoTracePCStop(int stop)
{
    g_global.stop = stop;
}


// 下边为封装后的创建元素结构体的函数，节省用户代码行数
SElement* CreatElementS64(int isHasInitValue, s64 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(s64 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_S, pElement->numberValue, 64);
    return pElement;
}

SElement* CreatElementS32(int isHasInitValue, s32 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(s32 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_S, pElement->numberValue, 32);
    return pElement;
}

SElement* CreatElementS16(int isHasInitValue, s16 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(s16 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_S, pElement->numberValue, 16);
    return pElement;
}

SElement* CreatElementS8(int isHasInitValue, s8 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(s8 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_S, pElement->numberValue, 8);
    return pElement;
}

SElement* CreatElementU64(int isHasInitValue, u64 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u64 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 64);
    return pElement;
}

SElement* CreatElementU32(int isHasInitValue, u32 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u32 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 32);
    return pElement;
}

SElement* CreatElementU16(int isHasInitValue, u16 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u16 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 16);
    return pElement;
}

SElement* CreatElementU8(int isHasInitValue, u8 initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u8 *)pElement->numberValue = initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 8);
    return pElement;
}

SElement* CreatElementNumberEnum(int isHasInitValue, u32 initValue, int* eunmTable, int  eunmCount)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u32 *)pElement->numberValue = initValue;

    pElement->enumCount = eunmCount;
    pElement->enumNumberTable= eunmTable;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_ENUM, pElement->numberValue, 32);
    return pElement;
}

SElement* CreatElementNumberRange(int isHasInitValue, u32 initValue , int min, int  max)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u32 *)pElement->numberValue = initValue;

    pElement->min = min;
    pElement->max= max;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_RANGE, pElement->numberValue, 32);
    return pElement;
}

SElement* CreatElementFloat(int isHasInitValue, float initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u32 *)pElement->numberValue = *(u32 *)&initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 32);
    return pElement;
}

SElement* CreatElementDouble(int isHasInitValue, double initValue)
{
    SElement *pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    *(u64 *)pElement->numberValue = *(u64 *)&initValue;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_NUMBER_U, pElement->numberValue, 64);
    return pElement;
}

SElement *CreatElementString(int isHasInitValue, char* initValue, int len, int maxLen)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    pElement->para.maxLen = maxLen;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_STRING, initValue, len << 3);
    return pElement;
}

SElement *CreatElementStringNum(int isHasInitValue, char* initValue, int len, int maxLen)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    pElement->para.maxLen = maxLen;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_STRING_NUM, initValue, len << 3);
    return pElement;
}


SElement *CreatElementStringEnum(
    int isHasInitValue, char* initValue, int len, int maxLen, char* eunmTableS[], int  eunmCount)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));

    pElement->para.maxLen = maxLen;

    pElement->enumCount = eunmCount;
    pElement->enumStringTable = eunmTableS;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_STRING_ENUM, initValue, len << 3);
    return pElement;
}

SElement *CreatElementBlob(int isHasInitValue, char* initValue, int len, int maxLen)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = maxLen;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_BLOB, initValue, len << 3);
    return pElement;
}

SElement *CreatElementBlobEnum(
    int isHasInitValue, char* initValue, int len, int maxLen, char* eunmTableB[], int eunmTableL[], int  eunmCount)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = maxLen;

    pElement->enumCount = eunmCount;
    pElement->enumBlobTable = eunmTableB;
    pElement->enumBloblTable = eunmTableL;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_BLOB_ENUM, initValue, len << 3);
    return pElement;
}

SElement *CreatElementFixBlob(int isHasInitValue, char* initValue, int len , int maxLen)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = len;// 忽略最大长度参数，使用len

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_FIXBLOB, initValue, len << 3);
    return pElement;
}

SElement *CreatElementIpv4(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_IPV4, initValue, 4 << 3);
    return pElement;
}

SElement *CreatElementIpv6(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_IPV6, initValue, 16 << 3);
    return pElement;
}

SElement *CreatElementMac(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_MAC, initValue, 6 << 3);
    return pElement;
}

SElement *CreatElementFloat16(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_FLOAT16, initValue, 2 << 3);
    return pElement;
}

SElement *CreatElementFloat32(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_FLOAT32, initValue, 4 << 3);
    return pElement;
}

SElement *CreatElementFloat64(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_FLOAT64, initValue, 8 << 3);
    return pElement;
}

SElement *CreatElementDouble64(int isHasInitValue, char* initValue)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = 0;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_DOUBLE, initValue, 8 << 3);
    return pElement;
}

SElement *CreatElementSelf(int isHasInitValue, char* initValue, int len, int maxLen, int arg)
{
    SElement *pElement = NULL;

    pElement = (SElement *)HwMalloc(sizeof(SElement));
    HwMemset(pElement, 0, sizeof(SElement));
    pElement->para.maxLen = maxLen;

    pElement->arg = arg;

    CreatElementEx(pElement, ENUM_YES, isHasInitValue, ENUM_TSELF, initValue, len << 3);
    return pElement;
}

#ifdef __cplusplus
}
#endif

