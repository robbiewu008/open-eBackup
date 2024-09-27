/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块提供样本管理

IO模块存在，则可以实现多样本变异，如果不支持，则无法从外部读取多样本，
WriteToFile
WriteToFileFail
ReadFromFile

llvm模块存在，则可以实现样本遗传算法，下三个函数只要实现即可，无论用什么方法
LlvmTracePcIsHasNewFeature
LlvmTracePcStartFeature
LlvmTracePcEndFeature


IO与llvm模块只要有一个存在即可实现部分功能，
如果都不存在，则本模块仅下个函数提供打印目前样本功能
CorpusShowCur


本模块对外部提供接口，用户可见，但也被宏封装，尽量不直接使用

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitCorpusMalloc(void)
{
    if (g_globalThead.corpusModule.corpusIsHasInit ==0)
    {
        g_globalThead.corpusModule.corpusIsHasInit = 1;

        g_globalThead.corpusModule.txt = HwMalloc(MAX_ONE_LINE);
        g_globalThead.corpusModule.txt1 = HwMalloc(MAX_ONE_LINE);

        g_globalThead.corpusModule.corpusM = (S_corpus_m *)HwMalloc(sizeof(S_corpus_m));
        HwMemset(g_globalThead.corpusModule.corpusM, 0, sizeof(S_corpus_m));

        g_globalThead.corpusModule.priorityCorpus = (int *)HwMalloc(MAX_Corpus_Priority_Num*sizeof(int));
        HwMemset(g_globalThead.corpusModule.priorityCorpus, 0, (MAX_Corpus_Priority_Num*sizeof(int)));
        g_globalThead.corpusModule.priorityCorpusNum = 0;

        g_globalThead.corpusModule.midPriorityCorpus = (int *)HwMalloc(MAX_Corpus_Priority_Num*sizeof(int));
        HwMemset(g_globalThead.corpusModule.midPriorityCorpus, 0, (MAX_Corpus_Priority_Num*sizeof(int)));
        g_globalThead.corpusModule.midPriorityCorpusNum = 0;
    }
}

static void InitElementFormCorpus(SPara* para, SElementInit* Element, int i)
{
    if (para->inType == ENUM_IN_NUMBERU8)
    {
        DT_SetGetU8(Element, *(u8 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERU16)
    {
        DT_SetGetU16(Element, *(u16 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERU32)
    {
        DT_SetGetU32(Element, *(u32 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERU64)
    {
        DT_SetGetU64(Element, *(u64 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBER_FLOAT)
    {
        DT_SetGetFloat(Element, *(float *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBER_DOUBLE)
    {
        DT_SetGetDouble(Element, *(double *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERS8)
    {
        DT_SetGetS8(Element, *(s8 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERS16)
    {
        DT_SetGetS16(Element, *(s16 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERS32)
    {
        DT_SetGetS32(Element, *(s32 *)para->value);
    }
    else if (para->inType == ENUM_IN_NUMBERS64)
    {
        DT_SetGetS64(Element, *(s64 *)para->value);
    }

    else if (para->inType == ENUM_IN_NUMBER_ENUM)
    {
        DT_SetGetNumberEnum(Element, *(s32 *)para->value, 
            g_globalThead.corpusModule.corpusM->enumNumberTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_NUMBER_ENUM_EX)
    {
        DT_SetGetNumberEnum_EX(Element, *(s32 *)para->value, 
            g_globalThead.corpusModule.corpusM->enumNumberTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_NUMBER_RANGE)
    {
        DT_SetGetNumberRange(Element, *(s32 *)para->value, 
            g_globalThead.corpusModule.corpusM->min[i], g_globalThead.corpusModule.corpusM->max[i]);
    }
    else if (para->inType == ENUM_IN_NUMBER_RANGE_EX)
    {
        DT_SetGetNumberRange_EX(Element, *(s32 *)para->value, 
            g_globalThead.corpusModule.corpusM->min[i], g_globalThead.corpusModule.corpusM->max[i]);
    }
    else if (para->inType == ENUM_IN_STRING)
    {
        if (para->len > 0)
        {
            para->value[para->len - 1] = 0;
        }
        
        DT_SetGetString(Element, para->len, para->maxLen, para->value);
    }
    else if (para->inType == ENUM_IN_STRING_NUM)
    {
        if (para->len > 0)
        {
            para->value[para->len - 1] = 0;
        }
        
        DT_SetGetStringNum(Element, para->len, para->maxLen, para->value);
    }
    else if (para->inType == ENUM_IN_STRING_ENUM)
    {
        if (para->len > 0)
        {
            para->value[para->len - 1] = 0;
        }

        DT_SetGetStringEnum(Element, para->len, para->maxLen, para->value, 
            g_globalThead.corpusModule.corpusM->enumStringTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_STRING_ENUM_EX)
    {
        if (para->len > 0)
        {
            para->value[para->len - 1] = 0;
        }

        DT_SetGetStringEnum_EX(Element, para->len, para->maxLen, para->value, 
            g_globalThead.corpusModule.corpusM->enumStringTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_BLOB)
    {
        DT_SetGetBlob(Element, para->len, para->maxLen, para->value);
    }
    else if (para->inType == ENUM_IN_BLOB_ENUM)
    {
        DT_SetGetBlobEnum(Element, para->len, para->maxLen, para->value, 
            g_globalThead.corpusModule.corpusM->enumBlobTable[i], g_globalThead.corpusModule.corpusM->enumBloblTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_BLOB_ENUM_EX)
    {
        DT_SetGetBlobEnum_EX(Element, para->len, para->maxLen, para->value, 
            g_globalThead.corpusModule.corpusM->enumBlobTable[i], g_globalThead.corpusModule.corpusM->enumBloblTable[i], g_globalThead.corpusModule.corpusM->enumCount[i]);
    }
    else if (para->inType == ENUM_IN_FIXBLOB)
    {
        DT_SetGetFixBlob(Element, para->len, para->maxLen, para->value);
    }
    else if (para->inType == ENUM_IN_IPV4)
    {
        DT_SetGetIpv4(Element, para->value);
    }
    else if (para->inType == ENUM_IN_IPV6)
    {
        DT_SetGetIpv6(Element, para->value);
    }
    else if (para->inType == ENUM_IN_MAC)
    {
        DT_SetGetMac(Element, para->value);
    }
    else if (para->inType == ENUM_IN_FLOAT16)
    {
        DT_SetGetFLoat16(Element, para->value);
    }
    else if (para->inType == ENUM_IN_FLOAT32)
    {
        DT_SetGetFLoat32(Element, para->value);
    }
    else if (para->inType == ENUM_IN_FLOAT64)
    {
        DT_SetGetFLoat64(Element, para->value);
    }
    else if (para->inType == ENUM_IN_DOUBLE)
    {
        DT_SetGetDouble64(Element, para->value);
    }
    else if (para->inType == ENUM_IN_SELF)
    {
        DT_SetGetSelf(Element, para->len, para->maxLen, para->value, g_globalThead.corpusModule.corpusM->arg[i]);
    }
}

//样本读取
static void CorpusRead(void)
{
    ReadAllCorpus(g_globalThead.corpusModule.corpusM->corpusPath);

    g_globalThead.corpusModule.corpusM->corpusIONumber = g_globalThead.corpusModule.corpusM->corpusNum;
    g_globalThead.corpusModule.corpusM->corpusReadNumber = g_globalThead.corpusModule.corpusM->corpusIONumber;

    if (g_globalThead.corpusModule.corpusM->corpusReadNumber)
    {
        hw_printf("\t Read corpus number is %d from corpus file!!!!\n", g_globalThead.corpusModule.corpusM->corpusReadNumber);
    }

    if (g_global.binCorpusDirIn)
    {
        ReadAllBinCorpus(g_global.binCorpusDirIn);
        g_global.binCorpusDirIn = NULL;

        if (g_globalThead.corpusModule.corpusM->corpusBinNum)
        {
            hw_printf("\t Read corpus number is %d from bin file!!!!\n", g_globalThead.corpusModule.corpusM->corpusBinNum);
        }
    }

}

//样本复现过程
static void CorpusReproduce(int isReproduce)
{
    // 如果is_reproduce非0，并且小于等于样本数量
    // 使用第is_reproduce个样本复现
    // 不变异，不记录样本
    if (isReproduce <= g_globalThead.corpusModule.corpusM->corpusNum)
    {
        int i;
        ClearFuzzEnvironment ();
        
        for(i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
        {
            InitElementFormCorpus(&g_globalThead.corpusModule.corpusM->corpus[isReproduce - 1]->para[i], &g_Element[i], i);
        }

        g_globalThead.randomSeed = g_globalThead.corpusModule.corpusM->corpus[isReproduce - 1]->randomSeed;
        g_globalThead.corpusModule.currentCorpusPos = isReproduce - 1;
        g_globalThead.isNeedMutator = 0;
        g_globalThead.isNeedRecordCorpus = 0;
        g_globalThead.isNeedForceRecordCorpus = 0;
        g_globalThead.isNeedWriteCorpus = 0;
        return;
    }
    else
    {
        //输入的数字大于样本数量
        hw_printf("\t the corpus number is %d < input number is %d,exit!!!!\n", g_globalThead.corpusModule.corpusM->corpusNum, isReproduce);
        HwExit(EXIT_CODE);
    }
}

//第一次运行
static void CorpusRunFirst(void)
{
    ClearFuzzEnvironment ();

    g_globalThead.randomSeed = 0;   // 运行代码中的不变异
    g_globalThead.corpusModule.currentCorpusPos = 0;
    g_globalThead.isNeedMutator = 0;
    g_globalThead.isNeedRecordCorpus = 1;
    g_globalThead.isNeedForceRecordCorpus = 1;
    g_globalThead.isNeedWriteCorpus = 0;
    CorpusStartFeature();
}

//早期执行文件样本
static void CorpusRunFileCorpus(void)
{
    int i;
    int j = g_globalThead.corpusModule.corpusM->runCount - 1;
    ClearFuzzEnvironment ();
    
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        InitElementFormCorpus(&g_globalThead.corpusModule.corpusM->corpus[j]->para[i], &g_Element[i], i);
    }

    g_globalThead.randomSeed = g_globalThead.corpusModule.corpusM->corpus[j]->randomSeed;
    g_globalThead.corpusModule.currentCorpusPos = g_globalThead.corpusModule.corpusM->runCount - 1;
    g_globalThead.isNeedMutator = 0;
    g_globalThead.isNeedRecordCorpus = 0;
    g_globalThead.isNeedForceRecordCorpus = 0;
    g_globalThead.isNeedWriteCorpus = 0;

    // 不记录样本，但是需要记录覆盖信息，一面以后同样的样本被记录
    CorpusStartFeature();
    return;
}

//早期执行bin 样本
static void CorpusRunBinCorpus(void)
{
    int i;
    int j = g_globalThead.corpusModule.corpusM->runCount - (1 + g_globalThead.corpusModule.corpusM->corpusReadNumber);
    ClearFuzzEnvironment ();
    
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        InitElementFormCorpus(&g_globalThead.corpusModule.corpusM->corpusBin[j]->para[i], &g_Element[i], i);
    }

    g_globalThead.randomSeed = g_globalThead.corpusModule.corpusM->corpusBin[j]->randomSeed;
    g_globalThead.corpusModule.currentCorpusPos = 0;  // 先这样，就记录给样本0吧
    g_globalThead.isNeedMutator = 0;
    g_globalThead.isNeedRecordCorpus = 1;
    g_globalThead.isNeedForceRecordCorpus = 0;
    g_globalThead.isNeedWriteCorpus = 1;

    // 不记录样本，但是需要记录覆盖信息，一面以后同样的样本被记录
    CorpusStartFeature();
    return;
}

//交换样本
static void CorpusSwitchCorpus(int isReproduce)
{
     int pos1;
    int pos2;
    int pos;
    int isCross;
    int paraPos;
    int i;
    ClearFuzzEnvironment ();

    pos1 = CorpusSelect();
    pos2 = RAND_RANGE(0, g_globalThead.corpusModule.corpusM->corpusNum - 1);
    pos = 0;

    // 20%100的测试例使用交叉参数，即其中一个参数来自另外样本
    // 遗传算法之杂交
    isCross = (RAND_32() % 5 == 0);
    paraPos = RAND_RANGE(0, g_globalThead.corpusModule.corpusM->paraNum - 1);

    i = 0;

    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        if (isCross && (i == paraPos))
        {
            pos = pos1;
        }
        else
        {
            pos = pos2;
        }

        InitElementFormCorpus(&g_globalThead.corpusModule.corpusM->corpus[pos]->para[i], &g_Element[i], i);
    }

    g_globalThead.randomSeed = g_globalThead.corpusModule.corpusM->corpus[pos]->randomSeed;
    g_globalThead.corpusModule.corpusM->corpus[pos]->mutatorCount -= SWITCH_CORPUS_COUNT;
    g_globalThead.corpusModule.currentCorpusPos = pos;
    
    if (isReproduce == (int)0xffffffff)
    {
        g_globalThead.isNeedRecordCorpus = 0;
        g_globalThead.isNeedMutator = 1;
        g_globalThead.isNeedForceRecordCorpus = 0;
        g_globalThead.isNeedWriteCorpus = 0;
        return;
    }
    
    g_globalThead.isNeedMutator = 1;
    g_globalThead.isNeedRecordCorpus = 1;
    g_globalThead.isNeedForceRecordCorpus = 0;
    g_globalThead.isNeedWriteCorpus = 1;
    CorpusStartFeature();
    return;
}

//同样本累积变异
static void CorpusAgainMutator(int isReproduce)
{
     int i;

    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        g_globalThead.corpusModule.tempCorpus.para[i] = g_Element[i].element->para;

        if (g_globalThead.corpusModule.tempCorpus.para[i].len > 0)
        {
            g_globalThead.corpusModule.tempCorpus.para[i].value = HwMalloc(g_globalThead.corpusModule.tempCorpus.para[i].len);
            HwMemcpy(g_globalThead.corpusModule.tempCorpus.para[i].value, g_Element[i].element->para.value, g_globalThead.corpusModule.tempCorpus.para[i].len);
        }
        else
        {
            g_globalThead.corpusModule.tempCorpus.para[i].value = NULL;
        }
    }

    ClearFuzzEnvironment ();
    
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        InitElementFormCorpus(&g_globalThead.corpusModule.tempCorpus.para[i], &g_Element[i], i);

        if(g_globalThead.corpusModule.tempCorpus.para[i].value != NULL)
        {
            HwFree(g_globalThead.corpusModule.tempCorpus.para[i].value);
            g_globalThead.corpusModule.tempCorpus.para[i].value = NULL;
        }
    }
    
    if (isReproduce == (int)0xffffffff)
    {
        g_globalThead.isNeedRecordCorpus = 0;
        g_globalThead.isNeedMutator = 1;
        g_globalThead.isNeedForceRecordCorpus = 0;
        g_globalThead.isNeedWriteCorpus = 0;
        return;
    }
    
    g_globalThead.isNeedRecordCorpus = 1;
    g_globalThead.isNeedMutator = 1;
    g_globalThead.isNeedForceRecordCorpus = 0;
    g_globalThead.isNeedWriteCorpus = 1;
    CorpusStartFeature();
    return;

}

//(isReproduce == (int)0xffffffff) 不进化样本，只用现有样本变异模式
void CorpusStart(int isReproduce)
{
    int temp;

    //获取随机数，作为联合变异的seed
    g_globalThead.randomSeed = RAND_32();

    // 第一次执行，读取样本
    if (g_globalThead.corpusModule.corpusM->runCount == 0)
    {
        CorpusRead();
    }

    // 样本复现测试，只在这里
    if ((isReproduce > 0) && (isReproduce != (int)0xffffffff))
    {
        CorpusReproduce(isReproduce);
        return;
    }

    //第一次运行，运行代码中的测试用例
    // 不变异，强制记录新样本，但不写入文件
    // g_isRunTestCodeInit控制有样本后是否运行代码中的初始值
    if ((isReproduce == 0) 
        && (g_globalThead.corpusModule.corpusM->runCount == 0))
    {
        CorpusRunFirst();
        return;
    }
    
    // 最早n次运行使用样本文件中的样本运行，
    // 不变异，不进行新样本记录
    if ((isReproduce == 0)
        && (g_globalThead.corpusModule.corpusM->corpusReadNumber > 0) // 因为代码中的样本一定在最后一个，所以一定有一个样本最少
        && (g_globalThead.corpusModule.corpusM->runCount < (g_globalThead.corpusModule.corpusM->corpusReadNumber + 1)))
    {
        CorpusRunFileCorpus();
        return;
    }

    // 接下来，如果有bin样本，则运行bin样本
    if ((isReproduce == 0)
        && (g_globalThead.corpusModule.corpusM->corpusBinNum > 0) // 因为代码中的样本一定在最后一个，所以一定有一个样本最少
        && (g_globalThead.corpusModule.corpusM->runCount < (g_globalThead.corpusModule.corpusM->corpusReadNumber + 1 + g_globalThead.corpusModule.corpusM->corpusBinNum)))
    {
        CorpusRunBinCorpus();
        return;
    }

    temp = g_globalThead.corpusModule.corpusM->runCount % SWITCH_CORPUS_COUNT;

    // 如果样本数量大于0，
    // 每约100次换个样本进行变异，
    if ((g_globalThead.corpusModule.corpusM->corpusNum > 0) && (temp == 0))
    {
        CorpusSwitchCorpus(isReproduce);
        return;
    }

    // 如果样本数量大于0，
    // 前百分之50运行样本一次变异，之后运行基于变异值得再次变异
    if ((g_globalThead.corpusModule.corpusM->corpusNum > 0)
        && (temp > (SWITCH_CORPUS_COUNT / 2)) 
        && (((temp - (SWITCH_CORPUS_COUNT / 2)) % ((SWITCH_CORPUS_COUNT / 2) / SWITCH_SUB_COUNT)) == 0))
    {
        CorpusAgainMutator(isReproduce);
        return;
    }

    if (isReproduce == (int)0xffffffff)
    {
        g_globalThead.isNeedRecordCorpus = 0;
        g_globalThead.isNeedMutator = 1;
        g_globalThead.isNeedForceRecordCorpus = 0;
        g_globalThead.isNeedWriteCorpus = 0;
        return;
    }

    g_globalThead.isNeedRecordCorpus = 1;
    g_globalThead.isNeedMutator = 1;
    g_globalThead.isNeedForceRecordCorpus = 0;
    g_globalThead.isNeedWriteCorpus = 1;
    CorpusStartFeature();
    return;
}

void CorpusStartFeature(void)
{
    KcovStartFeature();
    LlvmTracePcStartFeature();
}

void CorpusEndFeature(void)
{
    KcovEndFeature();
    LlvmTracePcEndFeature();
}

static int CorpusIsHasNewFeature(void)
{
    int temp = KcovIsHasNewFeature();
    int temp1 = LlvmTracePcIsHasNewFeature();
    int isHasNewFeature = temp || temp1;

    return isHasNewFeature;
}

int CorpusCheck(void)
{
    int isnotmismatch = 0;
    int i = 0;

    if (g_globalThead.corpusModule.corpusM->paraNum != g_globalThead.wholeRandomNum)
    {
        //连数量都不一样，还说啥呢
        hw_printf("\t the corpus paraNum is %d < code paraNum is %d,!!!!\n", g_globalThead.corpusModule.corpusM->paraNum, g_globalThead.wholeRandomNum);
        isnotmismatch = 1;
    }
    else
    {
        for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
        {
            //只比较第一个样本
            SPara* tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[0]->para[i];

            if (g_Element[i].element->para.type != tempCorpus->type)
            {
                //类型不一样
                hw_printf("\t the corpus type is %d < code type is %d int para %d,!!!!\n", tempCorpus->type, g_Element[i].element->para.type, i);
                isnotmismatch = 1;
            }

            if (g_Element[i].element->para.inType != tempCorpus->inType)
                {
                //细类型不一样
                hw_printf("\t the corpus inType is %d < code inType is %d int para %d,!!!!\n",  tempCorpus->inType, g_Element[i].element->para.inType, i);
                isnotmismatch = 1;
            }

            if (g_Element[i].element->para.maxLen != tempCorpus->maxLen)
            {
                //最大长度不一样
                hw_printf("\t the corpus maxLen is %d < code maxLen is %d int para %d,!!!!\n", tempCorpus->maxLen, g_Element[i].element->para.maxLen, i);
                isnotmismatch = 1;
            }
        }
    }

    if (isnotmismatch ==1)
    {
        if (g_global.isDelMismatchCopusFile == 0)
        {
            //程序安静的退出吧
            hw_printf("\t please modify your testcode or delete corpus file,exit!!!!\n");
            HwExit(EXIT_CODE);
        }
        else
        {
            char* path = g_globalThead.corpusModule.corpusM->corpusPath;
            CleanCorpus();
            g_globalThead.corpusModule.corpusM->corpusPath = path;

            hw_printf("\t corpus is mismatch, delete %s file, continue running!!!!\n", path);

            // 删除原有样本文件
            RemoveFile(path);
        }
    }
    return 0;
}

int CorpusEnd()
{
    int isHasNewFeature;
    int i;
    
    CorpusEndFeature();
    isHasNewFeature = CorpusIsHasNewFeature();

    //校验样本是否与测试用例相符，比较复杂，谁让产品线有需求呢:)
    // 第一次执行，已经读取完样本了，代码样本也已经执行了，可以比较一下了
    if ((g_globalThead.corpusModule.corpusM->runCount == 0) && (g_globalThead.corpusModule.corpusM->corpusNum != 0))
    {
       CorpusCheck();
    }

    // 只有加入整体随机的变量才能加入遗传算法
    if (g_globalThead.corpusModule.corpusM->paraNum == 0)
    {
        g_globalThead.corpusModule.corpusM->paraNum = g_globalThead.wholeRandomNum;
    }

    if ((g_globalThead.corpusModule.corpusM->runCount == 0) && (g_globalThead.temp_is_reproduce == 0) && (isHasNewFeature == 0))
    {
        hw_printf("\t Not found new cover on first running, please check -fsanitize-coverage=trace-pc!!!!\n");

        //如果用户设置了循环退出，那就退出吧
        if (g_global.stop == 1)
        {
            g_globalThead.gostop = 1;
            g_globalThead.isfoundfailed = 1;
        }
    }

    // 没有参数的测视用例就不用进行样本进化了，没意义
    if (g_globalThead.corpusModule.corpusM->paraNum == 0)
    {
        g_globalThead.corpusModule.corpusM->runCount++;
        return 0;
    }

    // 记录参数其他数据
    for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
    {
        g_globalThead.corpusModule.corpusM->enumNumberTable[i] = g_Element[i].enumNumberTable;
        g_globalThead.corpusModule.corpusM->enumStringTable[i] = g_Element[i].enumStringTable;
        g_globalThead.corpusModule.corpusM->enumBlobTable[i] = g_Element[i].enumBlobTable;
        g_globalThead.corpusModule.corpusM->enumBloblTable[i] = g_Element[i].enumBloblTable;
        g_globalThead.corpusModule.corpusM->enumCount[i] = g_Element[i].enumCount;
        g_globalThead.corpusModule.corpusM->min[i] = g_Element[i].min;
        g_globalThead.corpusModule.corpusM->max[i] = g_Element[i].max;
        g_globalThead.corpusModule.corpusM->arg[i] = g_Element[i].arg;
    }

    //有新的样本产生，样本变异次数增加5倍因子
    if((isHasNewFeature) && (g_globalThead.isNeedMutator == 1))
    {
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.currentCorpusPos]->mutatorCount += g_global.mutatorCountFromWeight * NewCorpus_Weight;
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.currentCorpusPos]->newCorpusCount++;
        g_globalThead.corpusModule.corpusM->corpus[g_globalThead.corpusModule.currentCorpusPos]->weight +=NewCorpus_Weight;
    }

    // 样本进化到最大，先打印告警，需要修改max_corpus_num的值重新编译库以适应当前场景
    if (isHasNewFeature && (g_globalThead.corpusModule.corpusM->corpusNum >= g_global.maxCorpusnum))
    {
        // 只打印10次起到警示作用就行了
        if (g_globalThead.corpusModule.temppNum < 10)
        {
            g_globalThead.corpusModule.temppNum++;
            hw_printf("\t the corpus number is max = %d, please notice!!!!\n",g_globalThead.corpusModule.corpusM->corpusNum);
        }
    }

     // 达到平滑样本数量，只使用高权重样本
    if (isHasNewFeature
        && (g_globalThead.corpusModule.corpusM->corpusNum >= g_global.smoothCorpusnum) 
        && (GetWeight() < NewLoop_Weight))
    {
        g_globalThead.corpusModule.corpusM->runCount++;
        return 0;
    }
    
    // 如果产生新的分支，则记录样本  遗传算法之进化
    // 如果达到最大样本数量，开始样本淘汰
    if (((g_globalThead.isNeedForceRecordCorpus == 1) )
        ||((g_globalThead.isNeedRecordCorpus == 1) && isHasNewFeature ))
    {
        int copyCorpus = 0;

        //如果达到最大样本，寻找最差样本替换
        if (g_globalThead.corpusModule.corpusM->corpusNum >= g_global.maxCorpusnum)
        {
            //永远找到mutatorCount最小值淘汰,但不写到样本文件，
            copyCorpus = CorpusDiscard();
        }
        else
        {
            copyCorpus = g_globalThead.corpusModule.corpusM->corpusNum;
            g_globalThead.corpusModule.corpusM->corpusNum++;
        }

        unsigned int hash = ElementGetHash();
        

        if (g_globalThead.corpusModule.corpusM->corpus[copyCorpus] != NULL)
        {
            CleanOneCorpus(g_globalThead.corpusModule.corpusM->corpus[copyCorpus]);
            g_globalThead.corpusModule.corpusM->corpus[copyCorpus] = NULL;
        }
        
        if (g_globalThead.corpusModule.corpusM->corpus[copyCorpus] == NULL)
        {
            g_globalThead.corpusModule.corpusM->corpus[copyCorpus] = (S_corpus*)HwMalloc(sizeof(S_corpus));
            HwMemset(g_globalThead.corpusModule.corpusM->corpus[copyCorpus], 0, sizeof(S_corpus));
        }

        g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->hash = hash;
        
        for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
        {
            SPara* tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->para[i];
            
            HwMemcpy(tempCorpus->name, g_Element[i].element->para.name, MAX_Name_Len);
            HwMemcpy(tempCorpus->mutaterName, g_Element[i].element->para.mutaterName, MAX_mutater_Len);
            
            tempCorpus->type = g_Element[i].element->para.type;
            tempCorpus->inType = g_Element[i].element->para.inType;
            tempCorpus->maxLen = g_Element[i].element->para.maxLen;
            tempCorpus->len = g_Element[i].element->para.len;

            if (g_Element[i].element->para.len > 0)
            {
                tempCorpus->value = HwMalloc(g_Element[i].element->para.len);
                HwMemcpy(tempCorpus->value, g_Element[i].element->para.value, g_Element[i].element->para.len);
            }
            else
            {
                tempCorpus->value = NULL;
            }
        }
        
        g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->paraNum = g_globalThead.corpusModule.corpusM->paraNum;
        g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->randomSeed = g_globalThead.randomSeed;
        g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->weight = GetWeight();
        g_globalThead.corpusModule.corpusM->corpus[copyCorpus]->mutatorCount 
            = GetWeight() * g_global.mutatorCountFromWeight;

        // 权重高的样本记录起来供优先调度
        CorpusPriorityAdd(copyCorpus, GetWeight());

        // 记录到样本文件尾部
        // 只通过路径记录的样本，不写到文件里,跳转样本也被除去
        if ((g_globalThead.isNeedWriteCorpus == 1) 
            && (GetWeight() >=NewLoop_Weight ) 
            && (g_globalThead.corpusModule.corpusM->corpusIONumber< g_global.maxCorpusnum))
        {
            CorpusWrite(copyCorpus);
        }

        // 如果要是变异出来的，要输出到bin 文件夹
        if ((g_global.binCorpusDirOut) 
            && (g_globalThead.isNeedMutator == 1) 
            && (GetWeight() >=NewLoop_Weight ) 
            && (g_globalThead.corpusModule.corpusM->corpusIONumber< g_global.maxCorpusnum))
        {
            CorpusWriteBin(copyCorpus);
        }
        
        g_globalThead.corpusModule.corpusM->newCorpusNum++;
    }

    g_globalThead.corpusModule.corpusM->runCount++;

    if (isHasNewFeature)
    {
        return 1;
    }
    return 0;
}

int CorpusGetCorpusNum(void)
{
    return g_globalThead.corpusModule.corpusM->corpusNum;
}

int CorpusGetReadCorpusNum(void)
{
    return g_globalThead.corpusModule.corpusM->corpusReadNumber;
}


int CorpusGetNewCorpusNum(void)
{
    return g_globalThead.corpusModule.corpusM->newCorpusNum;
}

// 不同测试例之间，要调用初始化函数清内存
void InitCorpus(void)
{   
    int j; 
    int i;

    InitCorpusMalloc();
    
    for (j = 0; j < g_globalThead.corpusModule.corpusM->corpusNum; j++)
    {
        for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
        {
            SPara* tempCorpus = &g_globalThead.corpusModule.corpusM->corpus[j]->para[i];
                
            if(tempCorpus->value)
            {
                HwFree(tempCorpus->value);
            }
            tempCorpus->value = NULL;
            
        }

        if (g_globalThead.corpusModule.corpusM->corpus[j] != NULL)
        {
            HwFree(g_globalThead.corpusModule.corpusM->corpus[j]);
            g_globalThead.corpusModule.corpusM->corpus[j] = NULL;
        }
    }

    // 模块内部数据，全部清0
    HwMemset(g_globalThead.corpusModule.corpusM, 0, sizeof(S_corpus_m));
}

void CleanOneCorpus(S_corpus* corpus)
{
        int i;
        for (i = 0; i < g_globalThead.corpusModule.corpusM->paraNum; i++)
        {
            SPara* tempCorpus = &corpus->para[i];
                
            if (tempCorpus->value)
            {
                HwFree(tempCorpus->value);
            }
            tempCorpus->value = NULL;
        }

        if (corpus != NULL)
        {
            HwFree(corpus);
        }
}

void CleanCorpus(void)
{   
    if (g_globalThead.corpusModule.corpusIsHasInit == 1)
    {
        int j; 
        for (j = 0; j < g_globalThead.corpusModule.corpusM->corpusNum; j++)
        {
            CleanOneCorpus(g_globalThead.corpusModule.corpusM->corpus[j]);
            g_globalThead.corpusModule.corpusM->corpus[j] = NULL;
        }

        for (j = 0; j < g_globalThead.corpusModule.corpusM->corpusBinNum; j++)
        {
            CleanOneCorpus(g_globalThead.corpusModule.corpusM->corpusBin[j]);
            g_globalThead.corpusModule.corpusM->corpusBin[j] = NULL;
        }
    
        HwMemset(g_globalThead.corpusModule.corpusM, 0, sizeof(S_corpus_m));

        HwMemset(g_globalThead.corpusModule.priorityCorpus, 0, (MAX_Corpus_Priority_Num*sizeof(int)));
        g_globalThead.corpusModule.priorityCorpusNum = 0;

        HwMemset(g_globalThead.corpusModule.midPriorityCorpus, 0, (MAX_Corpus_Priority_Num*sizeof(int)));
        g_globalThead.corpusModule.midPriorityCorpusNum = 0;
    }   
}

void CleanCorpusMemory(void)
{   
    g_globalThead.corpusModule.corpusIsHasInit = 0;
    if (g_globalThead.corpusModule.corpusM)
    {
        HwFree(g_globalThead.corpusModule.corpusM);
        g_globalThead.corpusModule.corpusM = NULL;
    }

    //
    if (g_globalThead.corpusModule.txt)
    {
        HwFree(g_globalThead.corpusModule.txt);
        g_globalThead.corpusModule.txt = NULL;
    }
    if (g_globalThead.corpusModule.txt1)
    {
        HwFree(g_globalThead.corpusModule.txt1);
        g_globalThead.corpusModule.txt1 = NULL;
    }
}


#ifdef __cplusplus
}
#endif

