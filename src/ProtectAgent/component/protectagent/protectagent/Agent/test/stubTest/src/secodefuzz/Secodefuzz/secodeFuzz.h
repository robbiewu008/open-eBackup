/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao    w00296180   v2.4.X 2021-1224
Creat at 2017-01-19

*/
#ifndef __MUTATOR_PUBLIC_H__
#define __MUTATOR_PUBLIC_H__

#ifndef __KERNEL__
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h> // sqrt cos log


#ifndef _WIN32
#include <sys/time.h> // struct itimeral. setitimer()
#include <sys/wait.h>
#include <unistd.h>  
#include <sys/resource.h>
#include <pthread.h>

#endif

#define __TIME_ENABLE__
#ifdef  __TIME_ENABLE__
#include <time.h>  // time
#endif

#else
#include <linux/module.h>
#include <linux/slab.h> /* kmalloc() */
#endif

// ====================== 宏开关 ======================
#define LIB_DEFINE                              // 系统库宏定义
#define HAS_IO                                              // 文件读写
#define HAS_HOOK
#define HAS_TRACE_PC
#define HAS_LEAK_CHECK
#define HAS_SIGNAL
// #define HAS_KCOV

// 多线程支持仅仅为了发现多线程访问引起的问题
// 不能为了提高测试速度使用它，提高测试速度请使用多进程
// 大多数情况测试不要打开它
// #define SUPPORT_M_THREAD

// vs编译器要裁剪的
#ifdef _MSC_VER

#ifndef __clang__
#undef HAS_HOOK
#undef HAS_TRACE_PC
#undef HAS_LEAK_CHECK
#undef HAS_SIGNAL
#endif
#undef HAS_KCOV
#endif

// 编进内核态，大多裁剪了
#ifdef __KERNEL__
#undef LIB_DEFINE
#undef HAS_IO
#undef HAS_HOOK
#undef HAS_TRACE_PC
#undef HAS_LEAK_CHECK
#undef HAS_SIGNAL
#undef HAS_KCOV
#endif

/******************************************

定义一些规格

******************************************/
#ifdef __KERNEL__
#define MAX_PARA_NUM                30                          // 定义一个测试例支持的最大参数数量
#define DEFAULT_CORPOS_NUM            10                             // 定义一个测试例支持的最大样本数量
#define DEFAULT_MAX_OUTPUT_SIZE   100000           // 定义默认的maxOutputSize
#else
#define MAX_PARA_NUM                256                         // 定义一个测试例支持的最大参数数量
#define DEFAULT_CORPOS_NUM            5000               // 定义一个测试例支持的默认最大样本数量
#define DEFAULT_MAX_OUTPUT_SIZE   10000000        // 定义默认的maxOutputSize
#endif

 //#define little_mem            1                                      // 内存非常非常吃紧的时候使用这个宏
#ifdef little_mem
#define MAX_PARA_NUM                10                          // 定义一个测试例支持的最大参数数量
#define DEFAULT_CORPOS_NUM             1                            // 定义一个测试例支持的最大样本数量
#define DEFAULT_MAX_OUTPUT_SIZE    1000             // 定义默认的maxOutputSize

#undef LIB_DEFINE
#undef HAS_IO
#undef HAS_HOOK
#undef HAS_TRACE_PC
#undef HAS_LEAK_CHECK
#undef HAS_SIGNAL
#undef HAS_KCOV
#endif

#ifdef HAS_KCOV
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef HAS_SIGNAL
#include <signal.h>
#include "errno.h"
#endif

// 针对没有malloc系统的，一般别打开(使用时可能还要具体调整其他宏)
// #define malloc_self  1

#ifdef __cplusplus
extern "C" {
#endif

/******************************************

整数类型声明

******************************************/
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
#ifdef __x86_64__       // 这是什么鬼，估计到产品线得改
typedef unsigned long long u64;
#else
typedef uint64_t u64;
#endif /*  ^sizeof(...)  */

// s8s16s32s64
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

/******************************************

枚举声明

******************************************/

// 变异算法枚举，按照使用频率排列貌似可以增加运行效率:)
enum EnumMutated {
    ENUM_DATAELEMENT_BIT_FLIPPER = 0,
    ENUM_DATAELEMENT_BIT_FILL,
    ENUM_DATAELEMENT_BIT_ZERO,
    ENUM_DATAELEMENT_CHANGE_ASCII_INTEGER,
    ENUM_DATAELEMENT_MBIT_FLIPPER,
    ENUM_DATAELEMENT_DUPLICATE,
    ENUM_DATAELEMENT_LENGTH_EDGE_CASE,
    ENUM_DATAELEMENT_LENGTH_RANDOM,
    ENUM_DATAELEMENT_LENGTH_GAUSS,
    ENUM_DATAELEMENT_LENGTH_REPEAT_PART,
    ENUM_DATAELEMENT_REDUCE,
    ENUM_DATAELEMENT_BYTE_RANDOM,
    ENUM_DATAELEMENT_ONE_BYTE_INSERT,
    ENUM_DATAELEMENT_SWAP_TWO_PART,
    ENUM_DATAELEMENT_STRING_STATIC,
    ENUM_DATAELEMENT_COPY_PART_OF,
    ENUM_DATAELEMENT_INSERT_PART_OF,
    ENUM_DATAELEMENT_AFL,
    ENUM_DATAELEMENT_MAGIC,
    ENUM_DATAELEMENT_MAGIC_CHANGE,

    ENUM_NUMBER_EDGE_CASE,
    ENUM_NUMBER_EDGE_RANGE,
    ENUM_NUMBER_RANDOM,
    ENUM_NUMBER_VARIANCE,
    ENUM_NUMBER_SMALL_RANGE,
    ENUM_NUMBER_POWER_RANDOM,
    ENUM_NUMBER_MAGIC,
    ENUM_NUMBER_ENUM_M,
    ENUM_NUMBER_RANGE_M,

    ENUM_STRING_ASCII_RANDOM,
    ENUM_STRING_LENGTH_ASCII_RANDOM,
    ENUM_STRING_CASE_LOWER,
    ENUM_STRING_CASE_RANDOM,
    ENUM_STRING_CASE_UPPER,
    ENUM_STRING_LENGTH_EDGE_CASE,
    ENUM_STRING_LENGTH_RANDOM,
    ENUM_STRING_LENGTH_GAUSS,
    ENUM_STRING_UTF8_BOM,
    ENUM_STRING_UTF8_BOM_LENGTH,
    ENUM_STRING_UTF8_BOM_STATIC,
    ENUM_STRING_STATIC,
    ENUM_STRING_MAGIC,
    ENUM_STRING_ENUM_M,

    ENUM_BLOB_CHANGE_BINARY_INTEGER,
    ENUM_BLOB_CHANGE_FROM_NULL,
    ENUM_BLOB_CHANGE_RANDOM,
    ENUM_BLOB_CHANGE_SPECIAL,
    ENUM_BLOB_CHANGE_TO_NULL,
    ENUM_BLOB_EXPAND_ALL_RANDOM,
    ENUM_BLOB_EXPAND_SINGLE_INCREMENTING,
    ENUM_BLOB_EXPAND_SINGLE_RANDOM,
    ENUM_BLOB_EXPAND_ZERO,
    ENUM_BLOB_MAGIC,
    ENUM_BLOB_ENUM_M,

    ENUM_CUSTOM_NUMBER,
    ENUM_CUSTOM_STRING,
    ENUM_CUSTOM_BLOB,

    ENUM_MIIP4,
    ENUM_MIPV6,
    ENUM_MMAC,
    ENUM_MFLOAT16,
    ENUM_MFLOAT32,
    ENUM_MFLOAT64,
    ENUM_MDOUBLE,
    ENUM_MSELF,
    ENUM_CUSTOMMUTATOR,
    ENUM_CROSS,

    //  enum_MutatedAFL,
    enum_MutatedMAX = 100,
};

// 用户自定义数据类型，目前看来是必须要重新编译lib代码的，而且要修改多处代码，咋能简单点呢?
enum EnumType {
    ENUM_NUMBER_U        = 0,     //  无符号数字
    ENUM_NUMBER_S,                  //  有符号数字
    ENUM_NUMBER_ENUM,         //   枚举数字，只在指定值范围内变异
    ENUM_NUMBER_RANGE,        //   范围数字，只在指定值范围内变异
    ENUM_STRING ,                      //  字符串
    ENUM_STRING_NUM,                //  字符串数字
    ENUM_STRING_ENUM,            //  枚举字符串，只在指定值范围内变异
    ENUM_BLOB,                          //  buffer 可变长内存块           不可变长内存块是否要?
    ENUM_BLOB_ENUM,               //   枚举内存块，只在指定值范围内变异
    ENUM_FIXBLOB,                     //    不可变长内存块
    ENUM_AFL,
    ENUM_IPV4,
    ENUM_IPV6,
    ENUM_MAC,
    ENUM_FLOAT16,
    ENUM_FLOAT32,
    ENUM_FLOAT64,
    ENUM_DOUBLE,
    ENUM_TSELF,
    ENUM_MAX = 20,
};

enum EnumWhether {
    ENUM_NO = 0,
    ENUM_YES = 1,
};

/******************************************

结构体声明

******************************************/
#define MAX_Name_Len  32
#define MAX_mutater_Len  32


typedef struct  {
    char        name[MAX_Name_Len];
    char        mutaterName[MAX_mutater_Len];
    int           type;
    int           inType;
    int           len;
    int           maxLen;
    char*       value;
}SPara;

typedef struct {
    SPara        para;
    int             isHasInitValue;             // 是否有初始值
    int             isNeedFree;
    int             isAddWholeRandom;        // 这个用来干啥
    int             mutatedFrequency;
    int             isAddWholeSequence;
    int             sequenceStartPos;

    char          numberValue[8];            // 专门用来保存数字类型初始值的，省的单独分配内存

    // s64--s32 修改看看
    s32           inLen;                         // 数据的长度，以bit为单位，string必须被8整除,blob也是
    char*        inBuf;
    int             pos;                             // 目前执行的测试用例

    // 输出的数据，只读
    int             count;                          // 一共多少测试例
    int             isNeedFreeOutBuf;              // 释放outBuf标志
    int             length;
    int             sign;                                // 0=无符号，有符号

    // 用户可以改变
    int             isMutatedClose[enum_MutatedMAX];        // 可以针对某个元素单独关闭变异算法，

    // 内部变量，请别动
    int             isMutatedSupport[enum_MutatedMAX];
    int             posStart[enum_MutatedMAX];
    int             num[enum_MutatedMAX];
    int             numbak[enum_MutatedMAX];

    int*           enumNumberTable;
    char**       enumStringTable;
    char**       enumBlobTable;
    int*           enumBloblTable;
    int             enumCount;

    int             min;
    int             max;

    int             arg;

    void*         init;        // SElementInit
}SElement;

typedef struct {
    int             first;
    int             type;
    int             inType;
    int             isHasInitValue;
    int             isNeedFree;
    char*        initValueBuffer;
    u64           initValue;

    int             len;        // number单位bit   other单位byte
    int             maxLen;
    // 用于结构体
    void*         structPtr;
    int             structStart;
    int             structLength;

    int*           enumNumberTable;
    char**       enumStringTable;
    char**       enumBlobTable;
    int*           enumBloblTable;
    int             enumCount;

    int             min;
    int             max;

    int             arg;

    SElement*  element;
}SElementInit;

/************************************************************************

0.一些辅助功能接口

************************************************************************/
// 0.设置是否在crash时打印crash样本在屏幕上,默认打印，1为打印，0为不打印
extern  void DT_Set_If_Show_crash(int isShowAll);

// 1.设置是否在测试例结束时打印样本在屏幕上,默认不打印，1为打印，0为不打印
extern  void DT_Set_If_Show_Corpus(int isShowAll);

// 2.设置打印测试例执行报告的路径，如果为空则不打印测试例报告(报告名后边自动补充时间后缀)
extern  void DT_Set_Report_Path(char* path);

// 2.1.设置打印测试例执行报告的文件名称，如果为空则不打印测试例报告(文件原先有内容则新测试追加到文件末尾)
//与上一个函数二选一，谁后设置谁生效
extern  void DT_Set_Report_Fix_PathName(char* pathName);

// 3.设置是否输出覆盖率sancov文件，在clang编译器下有效，需要同时打开环境变量export ASAN_OPTIONS=coverage=1
// 默认不输出，1为输出，0为不输出
extern  void DT_Set_Is_Dump_Coverage(int isDumpCoverage);

// 4.设置最大输出尺寸，单位byte, 默认值为Default_maxOutputSize
extern  void DT_Set_MaxOutputSize(int imaxOutputSize);

// 5.得到secodefuzz的版本信息
extern  const char *DT_Get_Version(void);

// 6.设置单个测试例执行一次超过多长时间报bug,如果为0，则不检测超时bug
extern  void DT_Set_TimeOut_Second(int second);

// 7.将二进制样本转换成文本字符串形式并打印,参数为二进制文件路径
extern  void DT_Printf_Bin_To_Corpus(char *path);

// 8.将二进制样本转换成文本字符串形式并写到文件里,参数为二进制文件路径
extern  void DT_Write_Bin_To_Corpus(char *path);

// 9.将样本字符串转换为二进制文件，并写到文件里，,参数为样本文件路径
extern  void DT_Write_Corpus_To_Bin(char *path);

// 10.设置是否开始或者停止监控覆盖反馈，一般不需要调用
extern  void DT_Enable_TracePC(int isEnable);

// 11.主动打印当前样本和写入文件，利用函数返回值判断错误的时候使用
extern  void DT_Show_Cur_Corpus(void);

// 12.控制debug开关函数，默认关闭，1为开启，0为关闭
extern  void DT_Enable_Log(int isEnable);

// 13.得到变异数据的长度
extern  int DT_GET_MutatedValueLen(SElementInit *init);

// 13.0设置变异数据变异频率百分比，100为%100,1为1%,-1为不变异，0为默认全局变异
// 即使为%100,因为有些变异算法的原因，也有少量情况可能没有变异
extern  void DT_SET_MutatedFrequency(SElementInit *init, int frequency);

// 13.1得到某个参数是否被变异
extern  int DT_GET_IsBeMutated(SElementInit *init);

// 14.关闭所有变异算法，默认全部开启，1为开启，0为关闭
extern  void DT_Enable_AllMutater(int isEnable);

// 15.关闭某个变异算法，默认开启，1为开启，0为关闭
extern  void DT_Enable_OneMutater(enum EnumMutated  mutatedNum, int isEnable);

// 16.设置字符串变异是否最后添加\0 (如果长度非0),默认为添加
extern  void DT_Set_String_Has_Terminal(int isHasTerminal);

// 16.1设置字符串变异是否有Null
extern  void DT_Set_String_Has_Null(int isHasNull);

// 17.设置第一次运行到代码块，打印块pc指针到屏幕，默认关闭
extern  void DT_Set_Is_Print_New_PC(int isPrintPC);

// 18.设置单个测试例运行时间，如果设置的运行次数没到而运行时间到了，提前结束
extern  void DT_Set_Running_Time_Second(int second);

// 18.1为了与测试例最低运行要求配合，当产生新样本时，
// 1.运行次数重新计算，2，运行时间重新计算，3，前两者都重新计算,0和默认都是不做事情
extern  void DT_Set_Reset_On_New_Corpus(int value);

// 19.打开样本进化过程对循环语句的支持，因为会一定程序影响测试效率，默认开启
extern  void DT_Enable_Support_Loop(int isEnable);

// 20.使能每运行一次测试用例检测一次内存泄露，会严重减慢测试速度，默认开启，检测一千次
// 最好是在已经发现内存泄露的情况下，再打开这个功能找到引起内存泄露的测试例
// 单测试用例之前调用生效，只生效一个测试用例
extern  void DT_Enable_Leak_Check(int isEnable, int isDebug);

// 21.被测代码在运行时，调用下边函数，会使当时的变异样本被保存下来，用于以后变异
extern  void DT_Set_HasNewFeature(int weight);

// 22.设置是否删除不匹配的样本文件
// 如果设置删除1，则程序删除样本后可以继续执行，
// 如果设置不删除0，则程序遇到不匹配的样本直接退出
// 默认不删除0
extern  void DT_Set_IsDeleteMismatchCopusFile(int isDelete);

// 23.设置最大样本数量，默认是MAX_CORPOS_NUM，最小是1，最大10000,设置0回复默认值
extern  void DT_Set_MCorpusNm(int num);

// 23.1设置平滑样本数量，达到平滑样本数量，低权重样本直接丢弃，默认3000
extern  void DT_Set_SMCorpusNm(int num);

// 24.支持多线程时，用来线程退出前删除工具分配的内存，避免内存泄露
extern  void DT_Clear_ThreadMemory(void);

// 25.设置一个权重值影响多少变异次数,默认1000
extern  void DT_Set_MutatorCountFromWeight(int value);

// 26,27.28用于同步变异
extern  int DT_Get_RandomSeed(void);
extern  void DT_Set_RandomSeed(int value);
extern  int DT_Get_RandomVlaue(void);

// 29.设置是否注册asan报告回调，默认使能，在报告没去重的场景需要关闭
extern  void DT_Enable_AsanReport(int isEnable);

// 30.存放二进制样本路径，仅一个测试用例有效，过后清除
extern  void DT_SetBinCorpusDirIn(char* dir, int maxLen);
extern  void DT_SetBinCorpusDirOut(char* dir);

// 31.使能fork模式，DT循环在子进程中运行,默认关闭
extern  void DT_SetEnableFork(int isEnable);

// 32.设置外部字典文件路径，单测试用例有效
extern  void DT_SetDictionaryPath(char *path);

// 33.设置样本产生不同产生方式的权重
//1,代码块覆盖(10),2,跳转覆盖(5),3,循环覆盖(5),4,分支覆盖(1)，设置为0则这种方式不对样本产生有影响
// 当有定时器的时候，2要调整为0
extern  void DT_SetPathModeWeight(int mode, int weight);

// 由使用者自定义，实现外部变异算法
// size_t DT_CustomMutator(uint8_t* data, size_t size, size_t max_size, unsigned int seed);
// size_t DT_CustomCrossOver(const uint8_t *data, size_t size1,const uint8_t *data2, size_t size2, uint8_t *out,size_t max_size, unsigned int Seed);

// 34.注册回调函数，每发现crash时工具调用
extern  void DT_AddCrashCallBack(void (*fun)(void));

// 35.注册回调函数，每个测试用例运行完成工具调用
extern  void DT_AddTestCaseCallBack(void (*fun)(void));

// 36.设置监控OutOfMemory，MallocLimitMb默认2048M,一次分配内存最大值,RssLimitMb默认2048M,物理内存rss使用最大值
extern  void DT_SetCheckOutOfMemory(int mallocLimitMb, int rssLimitMb);
// 默认没有开启检测内存耗尽功能
extern  void DT_SetStopOutOfMemory(void);

// 37.获取上个测试用例是否运行成功
extern  int DT_GetIsPass(void);

// 38.设置获取不到trace-pc信息，用例直接退出循环，默认为0，不退出，设置为1，退出
extern  void DT_SetNoTracePCStop(int stop);

/************************************************************************

1.封装测试例循环的声明

DT_FUZZ_START(0, 30000000,"test_example" ,0)
{
    printf("\r%d" ,fuzzSeed + fuzzi);

    s32 number= *(s32 *)DT_SetGetS32(&g_Element[0],0x123456);
    char *string = DT_SetGetString(&g_Element[1], 10, 10000,"zhangpeng" );
    char *buf = DT_SetGetBlob(&g_Element[2], 6, 20000,"12345" );
    int  buf_len=DT_GET_MutatedValueLen(&g_Element[2]);

    fun_example(number,string, buf , buf_len);
}
DT_FUZZ_END()

说明:
被测函数的上下文须放在DT_FUZZ_START和DT_FUZZ_END循环体里
DT_FUZZ_START(seed, count, testCaseName, isReproduce)  
seed            为随机变异使用的seed值，一般设置为0，工具会随机个seed出来  
count           为测试次数  
testCaseName    为样本文件路径和名称  
isReproduce     是否为复现模式,0为测试模式；1为复现模式，只根据指定样本运行1次  

注:
1.DT_SetGet函数调用不要在if语句里，这样会导致调用跳变
2.DT_SetGet函数初始值如果是内存指针，要在循环外部初始化，
避免循环下次执行时，作用域失效导致初始值指针变化
3.测试用例要与样本逻辑对应，如果测试用例有修改，请删除样本再进行测试
4.变异出的内存，工具会自己释放，如果被测程序要释放内存，
请将变异后的内存copy出来再调用被测函数
5.工具生成的样本是一种测试资源，再次运行测试用例工具能够读取样本继续测试，
如果测试用例逻辑没有修改，多次测试请不要删除样本

************************************************************************/
#ifdef SUPPORT_M_THREAD 
extern  __thread int fuzzSeed;
extern  __thread int fuzzi;
extern  __thread int fuzzStart;
extern  __thread int fuzzEnd;
extern  __thread SElementInit g_Element[];
extern  __thread int fuzzPid;
extern  __thread int fuzzStatus;
extern  __thread int g_isHasNewFeature;

#else
extern  int fuzzSeed;
extern  int fuzzi;
extern  int fuzzStart;
extern  int fuzzEnd;
extern  SElementInit g_Element[];
extern  int fuzzPid;
extern  int fuzzStatus;
extern  int g_isHasNewFeature;

#endif

extern  void DTStart(int seed, int count, char* testCaseName, int isReproduce);
extern  void DTForStart(void);
extern  void DTForEnd(void);
extern  void DTEnd(void);
extern  int DTStop(void);
extern  int   DTFork(void);
extern  void DTWait(char*tempTestCaseName);
extern  void DTExit(int no);

extern  int RunningTimeIsOver(void);

#define DT_FUZZ_START(seed, count, testCaseName, isReproduce)  \
    { \
        fuzzPid = DTFork(); \
        char *tempTestCaseName = testCaseName;\
        if (fuzzPid == 0) \
        { \
            DTStart(seed, count, testCaseName, isReproduce); \
            for (fuzzi = fuzzStart; fuzzi < fuzzEnd; fuzzi++) \
            { \
                DTForStart();

// 没有清理内存的地方，好尴尬，一定要清理内存的话，就自己调用fork等函数吧
#define DT_FUZZ_END() \
                DTForEnd(); \
                if (RunningTimeIsOver() || DTStop()) \
                { \
                    break; \
                } \
            } \
            DTEnd(); \
            DTExit(DT_GetIsPass()); \
        } \
        else \
        {\
            DTWait(tempTestCaseName);\
        }\
    }


#define DT_FUZZ_END_EX() \
                DTForEnd(); \
                if (RunningTimeIsOver() || DTStop()) \
                { \
                    break; \
                } \
            } \
            DTEnd(); \
        } \
        else \
        {\
            DTWait(tempTestCaseName);\
        }\
    }


// 对外部提供的接口，第一个单词大写，然后用下划线与后边隔开，其他每个单词首字母大写

/******************************************

2.接口一DT_SetGet系列接口

注:

1)
获取返回内存得长度举例如下
char *buf = DT_SetGetBlob(&g_Element[2], 6, 20000,"12345" );
int  buf_len=DT_GET_MutatedValueLen(&g_Element[2]);

2)
内存类参数获取，初始样本的内存和初始长度必须匹配
正确
char *string = DT_SetGetString(&g_Element[1], 10, 10000,"zhangpeng" );
错误
char *string = DT_SetGetString(&g_Element[1], 64, 10000,"zhangpeng" );

3)
第一个参数为固定为 &g_Element[]，下标从0开始，依次递增，不允许跳变 

4)
同一个下标只能有一个setget函数调用，下列写法错误
if (a == 1)
{
    DT_SetGetS32(&g_Element[0])
}
else
{
    DT_SetGetString(&g_Element[0])
}

******************************************/
// init 元素结构体指针,使用&g_Element[n],每个测试例n从0开始，必须连续
// initValue 初始值或指向初始值的指针
// 返回值,指向变异值的指针,
// 相应得类型只返回相应大小得内存，比如s32返回得指针为4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetS32
char *DT_SetGetS64(SElementInit *init, s64 initValue);
char *DT_SetGetS32(SElementInit *init, s32 initValue);
char *DT_SetGetS16(SElementInit *init, s16 initValue);
char *DT_SetGetS8(SElementInit *init, s8 initValue);
char *DT_SetGetU64(SElementInit *init, u64 initValue);
char *DT_SetGetU32(SElementInit *init, u32 initValue);
char *DT_SetGetU16(SElementInit *init, u16 initValue);
char *DT_SetGetU8(SElementInit *init, u8 initValue);
char *DT_SetGetFloat(SElementInit *init, float initValue);
char *DT_SetGetDouble(SElementInit *init, double initValue);
char *DT_SetGetFLoat16(SElementInit *init, char* initValue);
char *DT_SetGetFLoat32(SElementInit *init, char* initValue);
char *DT_SetGetFLoat64(SElementInit *init, char* initValue);
char *DT_SetGetDouble64(SElementInit *init, char* initValue);

// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! initValue               = 0x1234
// ! int eunmTable[]    ={2,4,6,1234,0x1234,0x897654,0x456789}
// ! eunmCount           =7
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberEnum
char *DT_SetGetNumberEnum(SElementInit *init, s32 initValue, int* eunmTable, int eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! initValue               = 0xff
// ! int eunmTable[]     ={2,4,6,1234,0x1234};
// ! eunmCount           =5
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberEnum_EX
char *DT_SetGetNumberEnum_EX(SElementInit *init, s32 initValue, int* eunmTable, int eunmCount);
// min最小值max最大值,变异会包含边界值
// 例如
// ! initValue        =0x1234
// ! min               =0x1200
// ! max              =0x1300
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberRange
char *DT_SetGetNumberRange(SElementInit *init, s32 initValue, int min, int  max);
// min最小值max最大值,变异不会包含边界值
// 例如
// ! initValue         =0xff
// ! min                =0x1200
// ! max               =0x1300
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberRange_EX
char *DT_SetGetNumberRange_EX(SElementInit *init, s32 initValue, int min, int  max);
// length         初始值的内存长度,strlen(initValue)+1
// maxLength  设置变异的最大长度，如果为0，则为默认最大长度，默认为Default_maxOutputSize
// 例如
// ! length                   =6
// ! maxLength            =20
// ! initValue                ="11111"
char *DT_SetGetString(SElementInit *init, int length, int maxLength, char* initValue);
//本算法初始值为字符串数字，变异出来的值为字符串数字
// length        初始值的内存长度,strlen(initValue)+1
// maxLength   设置变异的最大长度，这里被忽略
// 例如
// ! length                   =6
// ! maxLength            =0
// ! initValue                ="11111"
char *DT_SetGetStringNum(SElementInit *init, int length, int maxLength, char* initValue);
// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="1.1.1"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum(
    SElementInit *init, int length, int maxLength, char* initValue, char* eunmTableS[], int eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="11111"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum_EX(
    SElementInit *init, int length, int maxLength, char* initValue, char* eunmTableS[], int  eunmCount);
// length               初始值(initValue)的内存长度
// maxLength        设置变异的最大长度，如果为0，则为默认最大长度，默认为Default_maxOutputSize
// 例如
// ! length                   =15
// ! maxLength            =40
// ! initValue                ="12345678900000"
char *DT_SetGetBlob(SElementInit *init, int length, int maxLength, char* initValue);
// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! length                       =14
// ! maxLength                =20
// ! initValue                    ="6666666666666"
// ! char* eunmTableB[]  ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableL[]       ={7,7,5,14};
// ! eunmCount               =4
char *DT_SetGetBlobEnum(SElementInit *init, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! length                        =14
// ! maxLength                 =20
// ! initValue                     ="6666666666677"
// ! char* eunmTableB[]   ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableB[]        ={7,7,5,14};
// ! eunmCount                =4
char *DT_SetGetBlobEnum_EX(SElementInit *init, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// length初始值的长度，变异后的所有值长度与这个长度相同
// maxLengt这个参数被忽略
char *DT_SetGetFixBlob(SElementInit *init, int length, int maxLength, char*  initValue);
// 例如
// ! u8 temp_ipv4[4]    ={192,168,0,1};
// ! initValue                =(char *)temp_ipv4)
char *DT_SetGetIpv4(SElementInit *init, char* initValue);
// 例如
// ! u8 temp_ipv6[16]  ={0x20,0x02,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x01};
// ! initValue               =(char*)temp_ipv6
char *DT_SetGetIpv6(SElementInit *init, char* initValue);
// 例如
// ! u8 temp_mac[6]     ={0x28,0x6e,0xd4,0x89,0x26,0xa8};
// ! initValue                =(char*)temp_mac
char *DT_SetGetMac(SElementInit *init, char* initValue);

// 自定义类型举例
char *DT_SetGetSelf(SElementInit *init, int length, int maxLength, char* initValue, int arg);

/******************************************/
// 专为2.0定义

// 内核态不支持
#ifndef __KERNEL__
#define secodepits
#endif

// 使用开源xml库，打开这个宏
//#define USE_libxm2lib

#ifdef secodepits

// 对外开源版本将这个宏下所有代码删除
//#define Open_source 

enum Enum_Pits_Type {
    ENUM_BUF_CUT        = 0,        // 中间截取
    ENUM_BLOCK_DELETE,           // 整块删除          
    ENUM_BLOCK_COPY,               // 整块复制
    ENUM_BLOCK_CHANGE,          // 相邻整块交换  
    ENUM_ELEMENT_DELETE,       // 元素删除          
    ENUM_ELEMENT_COPY,          // 元素复制
    ENUM_ELEMENT_CHANGE,     // 相邻元素交换      
    ENUM_RELATION_SIZE,         // relation size相关变异
};

/*
用于DT_Pits_Set_DebugOnOff
g_onOffDebugMutatorElement
g_onOffDebugParseAssociated
g_onOffDebugGetBinBuf
g_onOffDebugParseBin
g_onOffDebugParseDataModel
g_onOffDebugParseStateModel//Open_source
g_onOffDebugParseTestModel//Open_source
g_onOffDebugParseXml//Open_source
g_onOffDebugPublisher//Open_source
g_onOffDebugDoRelation
g_onOffDebugDoFixup
g_onOffDebugDoTransformer
g_onOffDebugMutatorPits
all
null
*/

// 定义输出buf的最大长度
#define OUT_BUF_MAX_LENGTH  10000000
extern int g_tempElementCount;

// 1.关闭所有pits变异算法，默认全部关闭，1为开启，0为关闭
extern  void DT_Pits_Enable_AllMutater(int isEnable);

// 2.关闭某个pits变异算法，默认关闭，1为开启，0为关闭
extern  void DT_Pits_Enable_OneMutater(enum Enum_Pits_Type  mutatedNum, int isEnable);

// 3.设置变异频率，默认100,代表100分之1，越小频率越高
// 可能需要针对测试套元素的多少进行调整吧
extern  void DT_Pits_Set_Mutator_Frequency(int frequency);

// 4.设置算法选择频率，默认20，代表10分之1，越小频率越高,不能小于20
extern  void DT_Pits_Set_Algorithm_Frequency(int frequency);

#ifndef Open_source
// 5.解析模型
extern int DT_Pits_ParsePits(char* docname);

// 执行状态模型
extern void DT_Pits_DoState(void);
#endif

// 6.解析数据模型
extern int DT_Pits_ParseDataModel(char* docname, char* dataModelName, int isfirst);

// 执行action
extern void DT_Pits_DoAction(int id);

// 7.释放解析模型使用到的内存
extern void DT_Pits_ParseFree(void);

// 释放运行一次测试用例用到的中间内存
extern void DT_Pits_OneRunFree(void);

// 8.设置样本文件路径
extern void DT_Pits_SetBinFile(char* binName);

// 9.设置样本文件内存指针和长度
extern void DT_Pits_SetBinBuf(char* binBuf, int binBufLength);

// 10.设置单个元素变异的最大长度,默认4000
extern void DT_Pits_Set_MaxOutput(int value);

// 11.设置debug开关
extern void DT_Pits_Set_DebugOnOff(char* debug);

// 12.设置是否进行大小端交换，默认不交换
extern void DT_Pits_Set_BigOrLittleSwap(int swap);

// 13.设置config,优先级高于config文件,只对一个测试用例有效，DT_Pits_ParseFree清除
extern void DT_Pits_Set_Config( char* configKey,char* configValue);

// 得到变异后的数据和长度
extern void DT_Pits_GetMutatorBuf(char** buf, int *len);

/************************************************************************

V3系列接口，以后用于取代同类型接口，屏蔽g_Element,隐藏结构声明，
使h文件与so脱耦合

************************************************************************/

// 13.得到变异数据的长度
extern  int DT_GET_MutatedValueLenV3(int id);

// 13.0设置变异数据变异频率百分比，100为%100,1为1%,-1为不变异，0为默认全局变异
// 即使为%100,因为有些变异算法的原因，也有少量情况可能没有变异
extern  void DT_SET_MutatedFrequencyV3(int id, int frequency);

// 13.1得到某个参数是否被变异
extern  int DT_GET_IsBeMutatedV3(int id);

// 15.关闭某个变异算法，默认开启，1为开启，0为关闭
extern  void DT_Enable_OneMutaterV3(int  mutatedId, int isEnable);

// 对外部提供的接口，第一个单词大写，然后用下划线与后边隔开，其他每个单词首字母大写

/******************************************

2.接口一DT_SetGet系列接口

1)
获取返回内存得长度举例如下
char *buf = DT_SetGetBlobV3(2, 6, 20000,"12345" );
int  buf_len=DT_GET_MutatedValueLenV3(2);

2)
内存类参数获取，初始样本的内存和初始长度必须匹配
正确
char *string = DT_SetGetStringV3(1, 10, 10000,"zhangpeng" );
错误
char *string = DT_SetGetStringV3(1, 64, 10000,"zhangpeng" );

3)
id使用从0开始，依次递增，不允许跳变 

4)
同一个id只能有一个setget函数调用，下列写法错误
if (a == 1)
{
    DT_SetGetS32V3(0)
}
else
{
    DT_SetGetStringV3(0)
}


******************************************/
// 每个测试例id从0开始，必须连续
// initValue 初始值或指向初始值的指针
// 返回值,指向变异值的指针
// 相应得类型只返回相应大小得内存，比如s32返回得指针为4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetS32V3
char *DT_SetGetS64V3(int id, s64 initValue);
char *DT_SetGetS32V3(int id, s32 initValue);
char *DT_SetGetS16V3(int id, s16 initValue);
char *DT_SetGetS8V3(int id, s8 initValue);
char *DT_SetGetU64V3(int id, u64 initValue);
char *DT_SetGetU32V3(int id, u32 initValue);
char *DT_SetGetU16V3(int id, u16 initValue);
char *DT_SetGetU8V3(int id, u8 initValue);
char *DT_SetGetFloatV3(int id, float initValue);
char *DT_SetGetDoubleV3(int id, double initValue);
char *DT_SetGetFLoat16V3(int id, char* initValue);
char *DT_SetGetFLoat32V3(int id, char* initValue);
char *DT_SetGetFLoat64V3(int id, char* initValue);
char *DT_SetGetDouble64V3(int id, char* initValue);

// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! initValue               = 0x1234
// ! int eunmTable[]    ={2,4,6,1234,0x1234,0x897654,0x456789}
// ! eunmCount           =7
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberEnumV3
char *DT_SetGetNumberEnumV3(int id, s32 initValue, int* eunmTable, int eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! initValue               = 0xff
// ! int eunmTable[]     ={2,4,6,1234,0x1234};
// ! eunmCount           =5
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberEnum_EXV3
char *DT_SetGetNumberEnum_EXV3(int id, s32 initValue, int* eunmTable, int eunmCount);
// min最小值max最大值,变异会包含边界值
// 例如
// ! initValue        =0x1234
// ! min               =0x1200
// ! max              =0x1300
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberRangeV3
char *DT_SetGetNumberRangeV3(int id, s32 initValue, int min, int  max);
// min最小值max最大值,变异不会包含边界值
// 例如
// ! initValue         =0xff
// ! min                =0x1200
// ! max               =0x1300
// 返回值,指向变异值的指针,4个字节
// 获取返回值举例:s32 number= *(s32 *)DT_SetGetNumberRange_EXV3
char *DT_SetGetNumberRange_EXV3(int id, s32 initValue, int min, int  max);
// length         初始值的内存长度,strlen(initValue)+1
// maxLength  设置变异的最大长度，如果为0，则为默认最大长度，默认为Default_maxOutputSize
// 例如
// ! length                   =6
// ! maxLength            =20
// ! initValue                ="11111"
char *DT_SetGetStringV3(int id, int length, int maxLength, char* initValue);
//本算法初始值为字符串数字，变异出来的值为字符串数字
// length        初始值的内存长度,strlen(initValue)+1
// maxLength   设置变异的最大长度，这里被忽略
// 例如
// ! length                   =6
// ! maxLength            =0
// ! initValue                ="11111"
char *DT_SetGetStringNumV3(int id, int length, int maxLength, char* initValue);
// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="1.1.1"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnumV3(
    int id, int length, int maxLength, char* initValue, char* eunmTableS[], int eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="11111"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum_EXV3(
    int id, int length, int maxLength, char* initValue, char* eunmTableS[], int  eunmCount);
// length               初始值(initValue)的内存长度
// maxLength        设置变异的最大长度，如果为0，则为默认最大长度，默认为Default_maxOutputSize
// 例如
// ! length                   =15
// ! maxLength            =40
// ! initValue                ="12345678900000"
char *DT_SetGetBlobV3(int id, int length, int maxLength, char* initValue);
// 变异后的每个值都在枚举表里，不会变异出其他的
// 例如
// ! length                       =14
// ! maxLength                =20
// ! initValue                    ="6666666666666"
// ! char* eunmTableB[]  ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableL[]       ={7,7,5,14};
// ! eunmCount               =4
char *DT_SetGetBlobEnumV3(int id, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// 变异后的每个值都在枚举表之外
// 例如
// ! length                        =14
// ! maxLength                 =20
// ! initValue                     ="6666666666677"
// ! char* eunmTableB[]   ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableB[]        ={7,7,5,14};
// ! eunmCount                =4
char *DT_SetGetBlobEnum_EXV3(int id, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// length初始值的长度，变异后的所有值长度与这个长度相同
// maxLengt这个参数被忽略
char *DT_SetGetFixBlobV3(int id, int length, int maxLength, char*  initValue);
// 例如
// ! u8 temp_ipv4[4]    ={192,168,0,1};
// ! initValue                =(char *)temp_ipv4)
char *DT_SetGetIpv4V3(int id, char* initValue);
// 例如
// ! u8 temp_ipv6[16]  ={0x20,0x02,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x01};
// ! initValue               =(char*)temp_ipv6
char *DT_SetGetIpv6V3(int id, char* initValue);
// 例如
// ! u8 temp_mac[6]     ={0x28,0x6e,0xd4,0x89,0x26,0xa8};
// ! initValue                =(char*)temp_mac
char *DT_SetGetMacV3(int id, char* initValue);

// 自定义类型举例
char *DT_SetGetSelfV3(int id, int length, int maxLength, char* initValue, int arg);
#endif
#ifdef __cplusplus
}
#endif
#endif
