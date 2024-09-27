/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

fuzz�㷨���ȵĺ����ļ�
�Ա��ļ����޸ģ���Ҫ����

*/

#include "PCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

const char *g_LibInfo = "secodeFuzz version: v2.4.x (2021-12-24, 10:38)";

// �ڸ������㷨�������������ʱ�򣬱������ȿ������ߴ�
// ���ֵ��������̫С����ֻ֤���ܹ���ĺܴ�ߴ�ı����㷨�ſ����������
// ��ϵͳ���޲���̫�࣬�Ƽ�65535

// ȫ��Ψһ
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

//Ϊ�˶��̶߳���Ŭ��
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
���ñ�����֮ǰ���±߱������븳ֵ,����������׷����˵��ã�һ����Զ��
isNeedFree
isHasInitValue
type
isHasSigned     �������ͱ�ѡ
inLen
inBuf 
numberValue
�ðɣ����ǲ������˵�����:)
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

    // �ַ�����������ʼֵ�������ת��Ϊ���֣����ȡs64���͵����ֱ����㷨
    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        *(s64 *)pElement->numberValue = (s64)Inatol(pElement->inBuf);
    }

    // �õ�һ���ж��ٲ�������֧����Щ�����㷨
    for (i = 0; i < enum_MutatedMAX; i++)
    {
        // ���ڿ�˵���ñ����㷨û�б�ע��
        if (g_global.mutaterGroup[i] == NULL)
        {
            continue;
        }
        
        // �������㷨ȫ�ֿ����Ƿ񱻹ر�
        if (g_global.isMutatedClose[i] == ENUM_YES)
        {
            continue;
        }

        // �������㷨Ԫ�ؼ��𿪹��Ƿ񱻹رգ�������ڲ������ã���Ҫ�ú�����
        if (pElement->isMutatedClose[i] == ENUM_YES)
        {
            continue;
        }

        // ����Ԫ���Ƿ񱻸ñ����㷨֧��
        if (g_global.mutaterGroup[i]->getIsSupport(pElement) == ENUM_NO)
        {
            continue;
        }

        // ���øñ����㷨��֧�֣����ø��㷨һ�����ٲ�����
        pElement->isMutatedSupport[i] = ENUM_YES;
        pElement->num[i] = g_global.mutaterGroup[i]->getCount(pElement);

        if (g_global.isLogOpen)
        {
            hw_printf("[*] %s Get Count %d Mutator: %s\n", pElement->para.name, pElement->num[i], g_global.mutaterGroup[i]->name);
        }   
        
        pElement->numbak[i] = g_global.mutaterGroup[i]->getCount(pElement);

        // ����ÿ�������㷨��ʼ��������λ�ã�����߼���pos���뵽�Ǹ������㷨��
        pElement->posStart[i] = pElement->count;

        // ���ø�Ԫ��һ�����ٲ�����
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
            //��ӡһ�������¾�����
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

// ��ȡ��������ͳһ�ӿڣ���ȡ�����Լ�ǿ��ת�����Լ���Ҫ������
// Ŀǰ����Ϊ�ڲ��ṹ���������ṩ
static char* GetMutatedValue(SElement *pElement, int pos)
{
    int i;
    pElement->pos = pos;

    if (g_global.isLogOpen)
    {
        hw_printf("[*] %s Performing iteration [%d] \n", pElement->para.name, pos);
    }

    // ���ʹ��ԭʼλ�ã����ó�ʼֵ
    if (pos == (int)POS_ORIGINAL)
    {
        SetElementOriginalValue(pElement);
        HwMemcpy(pElement->para.mutaterName, "no mutator", Instrlen("no mutator") +1);
    }
    else
    {
        for (i = 0; i < enum_MutatedMAX; i++)
        {
            // ���ڿ�˵���ñ����㷨û�б�ע��
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

                    // ͳһ���������ڴ棬���ñ�������
                    len = pElement->para.len;
                    if ((pElement->para.len > 0) || (pElement->para.type == ENUM_FIXBLOB))
                    {
                        char* temp1;
                        if (pElement->para.len > pElement->para.maxLen)
                        {
                            pElement->para.len = pElement->para.maxLen;
                        }

                        // �̶����ȣ�����ĳ��ȿ��ܻ��б仯������֮
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

                    // ȥ������Ϊ0���ַ������죬��Ϊֻʣ\0
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

                    // �ַ���ͳһ���ƽ����������Ҳ���ɾ�����������
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

    // û�б����㷨����ԭʼֵ��ʵ�������Ǹ��ݴ���������Ӧ�õ�������
    if (pElement->count == 0)
    {
        tempPos = POS_ORIGINAL;
    }

    return GetMutatedValue(pElement, tempPos);
}

char* GetMutatedValueRandom(SElement *pElement)
{
    int pos;

    // �����Ϊ���ã���ʹ�����õ�Ƶ���ж��Ƿ�Ҫ����
    if ((pElement->mutatedFrequency > 0) && (!(RAND_RANGE(1, 100) <=  pElement->mutatedFrequency)))
    {
            pos = POS_ORIGINAL;
    }
    else if(pElement->mutatedFrequency == -1)
    {
            pos = POS_ORIGINAL;
    }
    // Ŀǰ����ƣ�ֻҪ�Ǽ�����������������һ�����ʲ��ܻ�ȡ����ֵ��������Ҫ�ı�
    else if ((pElement->mutatedFrequency == 0) && (pElement->isAddWholeRandom == ENUM_YES) && (GetIsMutated() != ENUM_YES))
    {
            pos = POS_ORIGINAL;
    }
    else
    {   
        // ���û�б���ֵ����ȡԭʼֵ��ok
        if (pElement->count == 0)
        {
            pos = POS_ORIGINAL;
        }
        else
        {
            pos = RAND_RANGE(0, pElement->count - 1);
        }  
    }

    // û���κα����㷨����ÿ��ʹ�ó�ʼֵ
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

// �ú�������free��ʱ�ڴ棬ÿ�λ�ȡ��������Ҫ����
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

// �±ߺ�������ȫ�ֱ����㷨���أ������ṩ
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

// �±ߺ�����������Ԫ�ر����㷨���أ������ṩ
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

// ��ģ���ʼ���������������ʼ����
// �ɵ�ĳ�������㷨��һ�ַ����ǣ�ֱ��ע�͵���ʼ�������ĵ���
void InitCommon(void)
{
    // ��ʼ�����б����㷨�ṹ��
    HwMemset((void *)g_global.mutaterGroup, 0, sizeof(g_global.mutaterGroup));

    // ��ʼ�������㷨�����ĳ�����㷨��ʼ��û�б����ã���ñ����㷨��������
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
    
    // ����Ϊ���Ʊ����㷨
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

    // ����û��Զ�������㷨,д������

    // �����ʷ���ģ��llvm��ʼ��
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

