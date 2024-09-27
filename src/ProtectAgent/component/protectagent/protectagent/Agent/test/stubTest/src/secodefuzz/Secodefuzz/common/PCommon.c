/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

fuzz算法调度的核心文件
对本文件的修改，需要谨慎

*/

#include "PCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

const char *g_LibInfo = "secodeFuzz version: v2.4.x (2021-12-24, 10:38)";

// 在各变异算法计算测试用例的时候，必须首先考虑最大尺寸
// 这个值不能设置太小，保证只有能够变的很大尺寸的变异算法才考虑这个因素
// 受系统所限不能太多，推荐65535

// 全局唯一
SGlobal g_global =
{
    .mutaterGroup = {0},
        
    .isMutatedClose = {ENUM_NO},
    .maxOutputSize = DEFAULT_MAX_OUTPUT_SIZE,
    .stringHasTerminal = 1,
    .stringHasNull = 0,
    .isPrintPc = 0,
    .isDumpCoverage = 0,
    .isResetOnNewCorpus = 0,
    .timeOutSecond = 0,
    .runningTimeSecond = 0,
    .isPrintfCrashCorpus = 1,
    .isLogOpen = 0,
    .maxCorpusnum = DEFAULT_CORPOS_NUM,
    .smoothCorpusnum = SMOOTH_CORPOS_NUM,
    .isNeedShowCorpus = 0,
    .isDelMismatchCopusFile = 0,
    .mutatorCountFromWeight = DEFAULT_MUTATOR_COUNT,
    .isEnableAsanReport = 1,
    .isEnableFork = 0,
    .loopModeWeight = NewLoop_Weight,
    .hashModeWeight = NewRoute_Weight,
    .blockModeWeight = NewPc_Weight,
    .edgeModeWeight = NewEdge_Weight,
    .stop = 0,

    .startCheckOutOfMemory = 0,
    .mallocLimitMb = 2048,
    .rssLimitMb = 2048,
    .mallocs = 0,
    .frees = 0,
    .mallocDebug = 0,
    .isSelfMalloc = 0,
    .enableDoLeakCheck = 1,
    .hasAddMallocCallBack = 0,
    
    .crashCallBack = NULL,
    .testCaseCallBack = NULL,

    .binCorpusDirIn = NULL,
    .binMaxLen = 0,
    .binCorpusDirOut = NULL,

    .reportPath = {0},

    .isSIGINT = 0,
    .isCrash = 0,

    .hasSignalInit = 0,
    .date = {0},
    .nameNo = 1,

    .modules_Start = {0},
    .modules_Stop = {0},
    .numModules = 0,
    .numGuards = 0,

    .pcNo = 10000
};

//为了多线程而做努力
#ifdef SUPPORT_M_THREAD 
__thread STGlobal g_globalThead =
#else
STGlobal g_globalThead =
#endif
{
    .temp_is_reproduce = 0,
    .seed = 0,
    .next = 1,
    .runningTestcaseName = {0},
    .testcaseName = NULL,
    
    .wholeRandomNum = 0,
    .runTime = 0,
    .runningTimeStartTime = 0,
    .isRunningtimeComplete = 0, 
    .gostop = 0,

    .isDoingLeak = 0,
    .doLeakCount = 0,

    .tracepcModule.isEnableFeature = 0,

    .tracepcModule.tracePcPcs = NULL,
    .tracepcModule.tracePcEdges = NULL,
    .tracepcModule.tracePcPcsTemp = NULL,
    .tracepcModule.tracePcIsHasMalloc = 0,
    .tracepcModule.tracePcPcTatol = 0,
    .tracepcModule.tracePcEdgeTatol = 0,

    .tracepcModule.pc2Sum = 0,

    .tracepcModule.bit8Loops = NULL, 
    .tracepcModule.bit8Hashs = NULL,
    .tracepcModule.bit8HashTatol = 0,
    .tracepcModule.bit8LoopTatol = 0,
    .tracepcModule.bit8IsHasMalloc = 0,

    .tracepcModule.bit8PerLoopCounters = NULL,         
    
    .tracepcModule.bit8LoopIdxHasBeenRecorded = NULL,                                         
    .tracepcModule.bit8LoopIdxTable = NULL,
    .tracepcModule.bit8LoopIdxTatol = 0,
    .tracepcModule.bit8LoopSum = 0,
    
    .tracepcModule.bit8PcIdxHasBeenRecorded = NULL,
    .tracepcModule.bit8PcIdxTable = NULL,
    .tracepcModule.bit8PcIdxTatol = 0,
    .tracepcModule.bit8PcsSum = 0,

    .tracepcModule.isHasNewFeature = 0,
    .tracepcModule.weight = 0,

    .hookModule.llvmNumberTableU64 = NULL,
    .hookModule.llvmMemTable = NULL,
    .hookModule.llvmDataIsHasInit = 0,

    .corpusModule.corpusM = NULL,
    .corpusModule.corpusIsHasInit = 0,
    .corpusModule.tempCorpus = {{{{0}}}},
    .corpusModule.priorityCorpus = NULL,
    .corpusModule.priorityCorpusNum = 0,
    .corpusModule.currentCorpusPos = 0,
    .corpusModule.txt = NULL,
    .corpusModule.txt1 = NULL,
    .corpusModule.temppNum = 0,
    .corpusModule.tempParaNum = 0,

    .isNeedRecordCrash = 0,
    .isNeedRecordCorpus = 0,
    .isNeedMutator = 1,
    .isfoundfailed = 0,
    .isPass = 1,
    .asanReportName = {0},
    .asanReportLen = 0,
    
    .valueBuf = NULL,
    .isHasInit = 0,
    .pcDescr = {0},
    .hashString = {0},

    .isRunningTestCode = 0,
};

const char *GetVersion()
{
    return g_LibInfo;
}

void SetMaxOutputSize(int imaxOutputSize)
{
    if (imaxOutputSize > DEFAULT_MAX_OUTPUT_SIZE)
    {
        imaxOutputSize = DEFAULT_MAX_OUTPUT_SIZE;
    }
    
    g_global.maxOutputSize = imaxOutputSize;
}

void SetStringHasTerminal(int isHasTerminal)
{
    g_global.stringHasTerminal = isHasTerminal;
}

int RegisterMutater(const struct MutaterGroup* pMutaterGroup, enum EnumMutated mutatedNum)
{
    g_global.mutaterGroup[mutatedNum] = pMutaterGroup;
    return 1;
}

/*
调用本函数之前，下边变量必须赋值,本函数给有追求的人调用，一般人远离
isNeedFree
isHasInitValue
type
isHasSigned     数字类型必选
inLen
inBuf 
numberValue
好吧，还是不给外人调用了:)
*/
static void GetElementName(SElement *pElement)
{
    switch (pElement->para.type) 
    {
        case ENUM_NUMBER_U: 
            hw_sprintf(pElement->para.name, "NumberU-%d", g_global.nameNo++);
            break;
        case ENUM_NUMBER_S: 
            hw_sprintf(pElement->para.name, "NumberS-%d", g_global.nameNo++);
            break;
        case ENUM_NUMBER_ENUM: 
            hw_sprintf(pElement->para.name, "NumberEnum-%d", g_global.nameNo++);
            break;
        case ENUM_NUMBER_RANGE: 
            hw_sprintf(pElement->para.name, "NumberRange-%d", g_global.nameNo++);
            break;
        case ENUM_STRING: 
            hw_sprintf(pElement->para.name, "String-%d", g_global.nameNo++);
            break;
        case ENUM_STRING_NUM: 
            hw_sprintf(pElement->para.name, "StringNum-%d", g_global.nameNo++);
            break;
        case ENUM_STRING_ENUM: 
            hw_sprintf(pElement->para.name, "StringEnum-%d", g_global.nameNo++);
            break;
        case ENUM_BLOB: 
            hw_sprintf(pElement->para.name, "Blob-%d", g_global.nameNo++);
            break;
        case ENUM_BLOB_ENUM: 
            hw_sprintf(pElement->para.name, "BlobEnum-%d", g_global.nameNo++);
            break;
        case ENUM_FIXBLOB: 
            hw_sprintf(pElement->para.name, "FixBlob-%d", g_global.nameNo++);
            break;
        case ENUM_AFL: 
            hw_sprintf(pElement->para.name, "AFL-%d", g_global.nameNo++);
            break;
        case ENUM_IPV4: 
            hw_sprintf(pElement->para.name, "Ipv4-%d", g_global.nameNo++);
            break;
        case ENUM_IPV6: 
            hw_sprintf(pElement->para.name, "Ipv6-%d", g_global.nameNo++);
            break;
        case ENUM_MAC: 
            hw_sprintf(pElement->para.name, "Mac-%d", g_global.nameNo++);
            break;
        case ENUM_FLOAT16: 
            hw_sprintf(pElement->para.name, "Float16-%d", g_global.nameNo++);
            break;
        case ENUM_FLOAT32: 
            hw_sprintf(pElement->para.name, "Float32-%d", g_global.nameNo++);
            break;
        case ENUM_FLOAT64: 
            hw_sprintf(pElement->para.name, "Float64-%d", g_global.nameNo++);
            break;
        case ENUM_DOUBLE: 
            hw_sprintf(pElement->para.name, "Double-%d", g_global.nameNo++);
            break;
        case ENUM_TSELF: 
            hw_sprintf(pElement->para.name, "self-%d", g_global.nameNo++);
            break;
        default:
            hw_sprintf(pElement->para.name, "default-%d", g_global.nameNo++);
            break;
    }

    return;
}

static void CreatElement(SElement *pElement)
{
    int i;

    ASSERT_NULL(pElement);
    ASSERT_NEGATIVE(pElement->inLen);
    ASSERT_NEGATIVE(pElement->para.maxLen);

    GetElementName(pElement);

    if (g_global.isLogOpen)
    {
        hw_printf("[*-*] Creat Element %s\n", pElement->para.name);
    }
    
    pElement->count = 0;
    pElement->pos = 0;

    // 字符串变量，初始值如果可以转化为数字，则获取s64类型的数字变异算法
    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        *(s64 *)pElement->numberValue = (s64)Inatol(pElement->inBuf);
    }

    // 得到一共有多少测试例，支持哪些变异算法
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        // 等于空说明该变异算法没有被注册
        if (g_global.mutaterGroup[i] == NULL)
        {
            continue;
        }
        
        // 看变异算法全局开关是否被关闭
        if (g_global.isMutatedClose[i] == ENUM_YES)
        {
            continue;
        }

        // 看变异算法元素级别开关是否被关闭，这个现在不起作用，需要好好想想
        if (pElement->isMutatedClose[i] == ENUM_YES)
        {
            continue;
        }

        // 看该元素是否被该变异算法支持
        if (g_global.mutaterGroup[i]->getIsSupport(pElement) == ENUM_NO)
        {
            continue;
        }

        // 设置该变异算法被支持，设置该算法一共多少测试例
        pElement->isMutatedSupport[i] = ENUM_YES;
        pElement->num[i] = g_global.mutaterGroup[i]->getCount(pElement);

        if (g_global.isLogOpen)
        {
            hw_printf("[*] %s Get Count %d Mutator: %s\n", pElement->para.name, pElement->num[i], g_global.mutaterGroup[i]->name);
        }   
        
        pElement->numbak[i] = g_global.mutaterGroup[i]->getCount(pElement);

        // 设置每个变异算法起始测视力的位置，共后边计算pos落入到那个变异算法里
        pElement->posStart[i] = pElement->count;

        // 设置该元素一共多少测试例
        pElement->count += pElement->num[i];
    }
}

void CreatElementEx(SElement *pElement, int isNeedFree, int isHasInitValue, int type, char* inBuf, int inLen)
{
    ASSERT_NULL(pElement);

    pElement->isNeedFree = isNeedFree;
    pElement->isHasInitValue = isHasInitValue;
    pElement->para.type = type;
    pElement->inBuf = inBuf;
    pElement->inLen = inLen;

    if (pElement->para.maxLen <= 0)
    {
        pElement->para.maxLen = g_global.maxOutputSize;
    }

    if (pElement->para.maxLen > g_global.maxOutputSize)
    {
        static int temp = 0;
        if (temp == 0)
        {   
            //打印一次提醒下就完了
            temp = 1;
            hw_printf("maxLen(%d) is larger than g_global.maxOutputSize(%d).\n", pElement->para.maxLen, g_global.maxOutputSize);
        }
        pElement->para.maxLen = g_global.maxOutputSize;
    }

    if (pElement->para.maxLen < (pElement->inLen / 8))
    {
        hw_printf("maxLen can not be less than inLen. %d, in len%d\n", pElement->para.maxLen, (pElement->inLen / 8));
        pElement->para.maxLen = g_global.maxOutputSize;
    }

    CreatElement(pElement);
}

void FreeElement(SElement *pElement)
{
    if (pElement != NULL)
    {
        if (pElement->isNeedFree)
        {
            if (g_global.isLogOpen)
            {
                hw_printf("[*-*] Free Element %s\n", pElement->para.name);
            }
            
            HwFree(pElement);
            pElement = NULL;
        }
    }
}

void AddWholeRandom(SElement *pElement)
{
    if (pElement->isAddWholeRandom == ENUM_NO)
    {
        pElement->isAddWholeRandom = ENUM_YES;
        g_globalThead.wholeRandomNum++;
    }
}

void LeaveWholeRandom(SElement *pElement)
{
    if (pElement->isAddWholeRandom == ENUM_YES)
    {
        pElement->isAddWholeRandom = ENUM_NO;
        g_globalThead.wholeRandomNum--;
    }
}

// 获取测试例的统一接口，获取后需自己强制转换成自己想要的类型
// 目前限制为内部结构，不对外提供
static char* GetMutatedValue(SElement *pElement, int pos)
{
    int i;
    pElement->pos = pos;

    if (g_global.isLogOpen)
    {
        hw_printf("[*] %s Performing iteration [%d] \n", pElement->para.name, pos);
    }

    // 如果使用原始位置，则用初始值
    if (pos == (int)POS_ORIGINAL)
    {
        SetElementOriginalValue(pElement);
        HwMemcpy(pElement->para.mutaterName, "no mutator", Instrlen("no mutator") +1);
    }
    else
    {
        for (i = 0; i < enum_MutatedMAX; i++)
        {
            // 等于空说明该变异算法没有被注册
            if (g_global.mutaterGroup[i] == NULL)
            {
                continue;
            }
            
            if (pElement->isMutatedSupport[i] == ENUM_NO)
            {
                continue;
            }
// wait bug2
            if ((pos >= pElement->posStart[i]) && (pos < (pElement->posStart[i] + pElement->num[i])))
            {
                if (g_global.mutaterGroup[i]->getValue)
                {
                    int len;
                    g_global.mutaterGroup[i]->getValue(pElement, pos - pElement->posStart[i]);
                    HwMemcpy(pElement->para.mutaterName, 
                        g_global.mutaterGroup[i]->name, Instrlen(g_global.mutaterGroup[i]->name) + 1);

                    // 统一重新申请内存，放置变异数据
                    len = pElement->para.len;
                    if ((pElement->para.len > 0) || (pElement->para.type == ENUM_FIXBLOB))
                    {
                        char* temp1;
                        if (pElement->para.len > pElement->para.maxLen)
                        {
                            pElement->para.len = pElement->para.maxLen;
                        }

                        // 固定长度，变异的长度可能会有变化，重置之
                        if (pElement->para.type == ENUM_FIXBLOB)
                        {
                            pElement->para.len = pElement->para.maxLen;
                        }

                        temp1 = HwMalloc(pElement->para.len);

                        if (pElement->para.type == ENUM_FIXBLOB)
                        {
                            HwMemset(temp1, pElement->para.len, 0);
                        }

                        HwMemcpy(temp1, pElement->para.value, MIN(len, pElement->para.len));

                        if (pElement->isNeedFreeOutBuf == ENUM_YES)
                        {
                            if (pElement->para.value != NULL)
                            {
                                HwFree(pElement->para.value);
                                pElement->para.value = NULL;
                                pElement->isNeedFreeOutBuf = ENUM_NO;
                            }
                        }
                        
                        pElement->para.value = temp1;
                        pElement->isNeedFreeOutBuf = ENUM_YES;
                    }

                    // 去掉长度为0的字符串变异，改为只剩\0
                    if ((g_global.stringHasNull == 0) && (pElement->para.len == 0) && (pElement->para.type == ENUM_STRING))
                    {
                        char* temp1;
                        pElement->para.len = 1;
                        temp1 = HwMalloc(pElement->para.len);

                        HwMemset(temp1, pElement->para.len, 0);

                        if (pElement->isNeedFreeOutBuf == ENUM_YES)
                        {
                            if (pElement->para.value != NULL)
                            {
                                HwFree(pElement->para.value);
                                pElement->para.value = NULL;
                                pElement->isNeedFreeOutBuf = ENUM_NO;
                            }
                        }
                        
                        pElement->para.value = temp1;
                        pElement->isNeedFreeOutBuf = ENUM_YES;
                    }

                    // 字符串统一复制结束符，这个也许会删除，看情况吧
                    if ((g_global.stringHasTerminal == 1) && (pElement->para.type == ENUM_STRING) && (pElement->para.len > 0))
                    {
                        pElement->para.value[pElement->para.len - 1] = 0;
                    }
                        
                    if (g_global.isLogOpen)
                    {
                        hw_printf("[*] Mutator: %s\n", g_global.mutaterGroup[i]->name);
                    }
                }
                break;
            }
        }
    }

    if (g_global.isLogOpen)
    {
        // int len = (pElement->outLen/8 + IS_ADD_ONE(pElement->outLen%8));
        u32 len = (u32)(pElement->para.len);
        hw_printf("[*] output size is %d\n", len);

        if (len <= DEBUG_MAX_OUTPUT_SIZE)
        {
            HexDump((u8 *)pElement->para.value, len);
        }
        else
        {
            HexDump((u8 *)pElement->para.value, DEBUG_MAX_OUTPUT_SIZE);
            hw_printf("[*] totol len is %d, only display %d ... ...\n", len, DEBUG_MAX_OUTPUT_SIZE);
        }
    }
    return pElement->para.value;
}

int GetMutatedValueLen(SElement *pElement)
{
    int length = pElement->para.len;
    return length;
}

void SetMutatedFrequency(SElement *pElement, int frequency)
{
    pElement->mutatedFrequency = frequency;
    return;
}


int GetIsBeMutated(SElement *pElement)
{
    if (pElement->pos == (int)POS_ORIGINAL)
    {
        return 0;
    }
    return 1;
}

char* GetMutatedValueSequence(SElement *pElement, int pos)
{
    int tempPos;

    if (pElement->isAddWholeSequence == ENUM_YES)
    {
        if ((pos >= pElement->sequenceStartPos) && (pos < (pElement->sequenceStartPos + pElement->count)))
        {
            tempPos = pos - pElement->sequenceStartPos;
        }
        else
        {
            tempPos = POS_ORIGINAL;
        }
    }
    else
    {
        tempPos = pos;
    }

    // 没有编译算法就用原始值，实际上这是个容错处理，正常不应该到这里来
    if (pElement->count == 0)
    {
        tempPos = POS_ORIGINAL;
    }

    return GetMutatedValue(pElement, tempPos);
}

char* GetMutatedValueRandom(SElement *pElement)
{
    int pos;

    // 如果人为设置，则使用设置的频率判断是否要变异
    if ((pElement->mutatedFrequency > 0) && (!(RAND_RANGE(1, 100) <=  pElement->mutatedFrequency)))
    {
            pos = POS_ORIGINAL;
    }
    else if(pElement->mutatedFrequency == -1)
    {
            pos = POS_ORIGINAL;
    }
    // 目前的设计，只要是加入整体随机，则就有一定概率才能获取变异值，可能需要改变
    else if ((pElement->mutatedFrequency == 0) && (pElement->isAddWholeRandom == ENUM_YES) && (GetIsMutated() != ENUM_YES))
    {
            pos = POS_ORIGINAL;
    }
    else
    {   
        // 如果没有变异值，则取原始值就ok
        if (pElement->count == 0)
        {
            pos = POS_ORIGINAL;
        }
        else
        {
            pos = RAND_RANGE(0, pElement->count - 1);
        }  
    }

    // 没有任何变异算法，就每次使用初始值
    if (pElement->count == 0)
    {
        pos = POS_ORIGINAL;
    }
    
    if (pos >= pElement->count)
    {
        hw_printf("\tCount of element is greater than pos!\n");
        ASSERT(1);
        return NULL;
    }
    
    return GetMutatedValue(pElement, pos);
}

// 该函数仅仅free临时内存，每次获取测试例后都要调用
void FreeMutatedValue(SElement *pElement)
{
    if (pElement->isNeedFreeOutBuf)
    {
        if (pElement->para.value != NULL)
        {
            HwFree(pElement->para.value);
            pElement->para.value = NULL;
            pElement->isNeedFreeOutBuf = ENUM_NO;
        }
    }
}

// 下边函数操作全局变异算法开关，对外提供
void    SetCloseAllMutater(void)
{
    int i;
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        g_global.isMutatedClose[i] = ENUM_YES;
    }
}

void    SetCloseOneMutater(enum EnumMutated  mutatedNum)
{
    g_global.isMutatedClose[mutatedNum] = ENUM_YES;
}

void    SetOpenAllMutater(void)
{
    int i;
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        g_global.isMutatedClose[i] = ENUM_NO;
    }
}
void    SetOpenOneMutater(enum EnumMutated  mutatedNum)
{
    g_global.isMutatedClose[mutatedNum] = ENUM_NO;
}

// 下边函数操作单独元素变异算法开关，对外提供
void    SetElementCloseAllMutater(SElement *pElement)
{
    int i;
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        pElement->isMutatedClose[i] = ENUM_YES;
    }
}

void    SetElementCloseOneMutater(SElement *pElement, enum EnumMutated  mutatedNum)
{
    pElement->isMutatedClose[mutatedNum] = ENUM_YES;
}

void    SetElementOpenAllMutater(SElement *pElement)
{
    int i;
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        pElement->isMutatedClose[i] = ENUM_NO;
    }
}

void    SetElementOpenOneMutater(SElement *pElement, enum EnumMutated  mutatedNum)
{
    pElement->isMutatedClose[mutatedNum] = ENUM_NO;
}

// 本模块初始化函数，必须在最开始调用
// 干掉某个变异算法的一种方法是，直接注释掉初始化函数的调用
void InitCommon(void)
{
    // 初始化所有变异算法结构体
    HwMemset((void *)g_global.mutaterGroup, 0, sizeof(g_global.mutaterGroup));

    // 初始化变异算法，如果某变异算法初始化没有被调用，则该变异算法不起作用
    InitBlobChangeBinaryInteger();
    InitBlobChangeFromNull();
    InitBlobChangeRandom();
    InitBlobChangeSpecial();
    InitBlobChangeToNull();
    InitBlobExpandAllRandom();
    InitBlobExpandSingleIncrementing();
    InitBlobExpandSingleRandom();
    InitBlobExpandZero();
    InitBlobMagic();
    InitBlobEnum();
    
    // 这三为定制变异算法
    InitCustomNumber();
    InitCustomBlob();
    InitCustomString();

    InitDataElementReduce();
    InitDataElementDuplicate();
    InitDataElementBitFlipper();
    InitDataElementBitFill();
    InitDataElementBitZero();
    InitDataElementChangeASCIIInteger();
    InitDataElementMBitFlipper();
    InitDataElementLengthEdgeCase();
    InitDataElementLengthRandom();
    InitDataElementLengthGauss();
    InitDataElementLengthRepeatPart();
    InitDataElementByteRandom();
    InitDataElementOneByteInsert();
    InitDataElementSwapTwoPart();

    InitDataElementStringStatic();
    InitDataElementCopyPartOf();
    InitDataElementInsertPartOf();
    InitDataElementAFL();
    InitDataElementMagic();
    InitDataElementMagicChange();
    
    InitNumberEdgeCase();
    InitNumberEdgeRange();
    InitNumberRandom();
    InitNumberVariance();
    InitNumberSmallRange();
    InitNumberPowerRandom();
    InitNumberMagic();
    InitNumberEnum();
    InitNumberRange();
    
    InitStringAsciiRandom();
    InitStringLengthAsciiRandom();
    InitStringCaseLower();
    InitStringCaseRandom();
    InitStringCaseUpper();
    InitStringLengthEdgeCase();
    InitStringLengthRandom();
    InitStringLengthGauss();
    InitStringUtf8Bom();
    InitStringUtf8BomLength();
    InitStringUtf8BomStatic();
    InitStringStatic();
    InitStringMagic();
    InitStringEnum();

    InitIpv4();
    InitIpv6();
    InitMac();
    InitFloat16();
    InitFloat32();
    InitFloat64();
    InitDouble();
    InitSelf();
    InitCustomMutator();
    InitCross();

    // 如果用户自定义变异算法,写在这里

    // 覆盖率反馈模块llvm初始化
    InitPcCounters();
    Init8BitCounters();
    InitKcov();
    InitLlvmData();
    InitCorpus();
    InitSignalCallback();
    InitTimeOut();
}

void ClearMemcory(void)
{
    // 8bit
    LlvmClean8BitCounters();
    
    // pccount
    LlvmCleanPcCounters();

    //corpus
    CleanCorpusMemory();

    // llvmdata
    CleanLlvmData();

    // internal
    g_globalThead.isHasInit = 0;
    if (g_globalThead.valueBuf)
    {
        HwFree(g_globalThead.valueBuf);
        g_globalThead.valueBuf = NULL;
    }
    
}

int GetWeight(void)
{
    int weight = 0;
    weight += LlvmTracePcGetWeight();
    weight += KcovGetWeight();
    return weight;
}

#ifdef __cplusplus
}
#endif

