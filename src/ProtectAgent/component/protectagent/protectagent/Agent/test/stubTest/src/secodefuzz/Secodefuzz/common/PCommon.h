/*
版权所有 (c) 华为技术有限公司 2012-2018
作者:
wanghao 			w00296180
wangchengyun 	wwx412654


*/

#ifndef _COMMON_H
#define _COMMON_H

#include "../secodeFuzz.h"

#ifdef __cplusplus
extern "C" {
#endif

// 目前这东西仅用于打印，攒不用来做别的，切记切记
enum EnumInType {
    ENUM_IN_MIN 	= 0,	
    ENUM_IN_NUMBERU8,	
    ENUM_IN_NUMBERU16,
    ENUM_IN_NUMBERU32,
    ENUM_IN_NUMBERU64,
    ENUM_IN_NUMBER_FLOAT,
    ENUM_IN_NUMBER_DOUBLE,
    ENUM_IN_NUMBERS8,
    ENUM_IN_NUMBERS16,
    ENUM_IN_NUMBERS32,
    ENUM_IN_NUMBERS64,
    ENUM_IN_NUMBER_ENUM,	
    ENUM_IN_NUMBER_ENUM_EX,	
    ENUM_IN_NUMBER_RANGE,	
    ENUM_IN_NUMBER_RANGE_EX,	
    ENUM_IN_STRING,		
    ENUM_IN_STRING_NUM,	
    ENUM_IN_STRING_ENUM,	
    ENUM_IN_STRING_ENUM_EX,	
    ENUM_IN_BLOB,			
    ENUM_IN_BLOB_ENUM,	
    ENUM_IN_BLOB_ENUM_EX,	
    ENUM_IN_FIXBLOB,		
    ENUM_IN_AFL,
    ENUM_IN_IPV4,
    ENUM_IN_IPV6,
    ENUM_IN_MAC,
    ENUM_IN_FLOAT16,
    ENUM_IN_FLOAT32,
    ENUM_IN_FLOAT64,
    ENUM_IN_DOUBLE,
    ENUM_IN_SELF,
    ENUM_IN_MAX = 30,
};

enum EnumOperationType {
    ENUM_INSERT	= 0,
    ENUM_OVERWRITE = 1,
    ENUM_REPLACE	= 2,
    ENUM_CHANGE	= 3, // 替换一部分，长度不变
};

#define INTEGER_8		8


#define EXIT_CODE		             50
#define EXIT_CODE_LEAK		      51
#define EXIT_CODE_OTHER             52
#define EXIT_CODE_RssLimit          53
#define EXIT_CODE_MallocLimit     54


// Corpus

// 最大的样本数量不能超过10000，实际上超过了也没啥意义了
#define MAX_CORPUSP_NUM                        10000
#define MAX_Corpus_Priority_Num            (MAX_CORPUSP_NUM*4)
#define MAX_Corpus_Hash_Num                (MAX_CORPUSP_NUM*10)
#define NewPc_Weight                                30
#define NewEdge_Weight                            3 //暂时变小，hold住两个定时器
#define NewLoop_Weight                            20
#define NewRoute_Weight                          1
#define NewCorpus_Weight                        1
#define DEFAULT_MUTATOR_COUNT           1000

#define SMOOTH_CORPOS_NUM           (DEFAULT_CORPOS_NUM * 2 / 3)


// 变异出来的最大长度为MAX_valueBuf，一个内存最多4个字节表示
// 比如\x00  多分配1000字节表示名字等号啥的:)
// 定义样本文件一行最大的字数
#define MAX_ONE_LINE  ((DEFAULT_MAX_OUTPUT_SIZE*(4|0ULL)+1000)  * 2)
#define SWITCH_CORPUS_COUNT 100                 // 定义样本间切换的迭代次数间隔
#define SWITCH_SUB_COUNT        4                   // 定义一次样本使用中，使用变异值再次变异的迭加次数

// Peport
#define MAX_FILE_PATH 512                             // 定义最大的文件路径字符串长度

// Trace_PC
#define MAX_PC_NUM  (1<<21)                         // 定义保存最大的代码块pc指针数量
#define MAX_MODULE_NUM  4096                     // 定义最大模块数量
#define MAX_PC_DESCR_SIZE  1024                 // 定义最大描述pc打印字符串的尺寸

// LLVMData
#define SIZE_LLVM_MEM_TABLE  20480           // 内存hook保存的最大内存数据的数量
#define SIZE_LLVM_MEM_DATA  40                // 每个数据不应超过40字节，太大了貌似也没啥意义 ::大约1M内存占用
#define SIZE_LLVM_NUMBER_TABLE_U64  102400   // 整数hook保存的最大数字数据的数量

// Kcov
#define MAX_KERNEL_PC_NUM  (1<<21)

// Common
#define DEBUG_MAX_OUTPUT_SIZE  1000        // 打印输出单测试例最大字节数

#define MAX_COUNT                500                    // 一个变异算法 测试例的最大数量
#define STRING_NUMBER_LEN       33

#define MIN(_a, _b)     (((_a) > (_b)) ? (_b) : (_a))
#define MAX(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))

#define IS_ADD_ONE(x)   (((x) > (0)) ? (1) : (0))

#define POS_ORIGINAL    0xffffffff

// Internal
#define IS_USE_GLOBAL_MALLOC            1       // 设置为变异数据分配的内存是否使用一次性分配的大内存

// 单bit翻转
#define FLIP_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] ^= (128 >> ((bf) & 7)); \
    } while (0)

// 单bit清0
#define ZERO_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] &= ~(128 >> ((bf) & 7)); \
    } while (0)

// 单bit置1
#define FILL_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] |= (128 >> ((bf) & 7)); \
    } while (0)

#define ONE_IN(x)				((HwRand() % (x)) == 0)	// limit of RAND_MAX-1
#define RAND_16()				(HwRand() & 0xFFFF)
#define RAND_32()				(((0UL | HwRand()) << 1) | (HwRand() & 1))
// rand_64为什么要这样写
#define RAND_64()				(((0ULL | HwRand()) << 33) | ((0ULL | HwRand()) << 2) | (HwRand() & 0x3))
#define RAND_BOOL()				(HwRand() & 1)
#define RAND_BYTE()				(HwRand() & 0xff)

#define RAND_RANGE(min, max)	((min) + (HwRand() % ((max) - (min) + 1)))
#define RAND_RANGE64(min, max)	((min) + (RAND_64() % ((max) - (min) + 1)))

//====================== 宏开关 ======================
#define ENABLE_ASSERT			1		// enable assert

//  DEBUG 
#ifdef _WIN32
#define __s_func__				__FUNCTION__
#else
#define __s_func__				__func__
#endif 

#if ENABLE_ASSERT
#define ASSERT(value)			do{ \
        if (value) \
        { \
            hw_printf("\r\n!!!*******:%s[%s]%d \r\n", __FILE__, __s_func__, __LINE__); \
            hw_printf("assert!\r\n"); \
        } \
    }while (0)
    
#else
#define ASSERT(value)		
#endif
#define ASSERT_NULL(value)		ASSERT(NULL == (value))
#define ASSERT_ZERO(value)		ASSERT(0 == (value))
#define ASSERT_NEGATIVE(value)	ASSERT(0 > (value))

#ifndef __KERNEL__
// public
#define hw_printf(fmt, ...)				printf(fmt, ##__VA_ARGS__)
#else
#define hw_printf(fmt, ...)				printk(fmt, ##__VA_ARGS__)
#endif

#ifndef _WIN32
#define hw_sprintf(buf, fmt, ...)			sprintf(buf, fmt, ##__VA_ARGS__)
#else

#ifdef __clang__
#define hw_sprintf(buf, fmt, ...)			_snprintf(buf, 50000, fmt, ##__VA_ARGS__)
#else
#define hw_sprintf(buf, fmt, ...)			sprintf(buf, fmt, ##__VA_ARGS__)
#endif

#endif

// #ifndef _MSC_VER //为了核心网，先注释掉
#define hw_sscanf(buf, fmt, ...)			sscanf(buf, fmt, ##__VA_ARGS__)
// #else
// #define hw_sscanf(buf, fmt, ...)			sscanf_s(buf, fmt, ##__VA_ARGS__)
// #endif

//====================== Platform detection ======================
#ifdef __linux__
#define SECODEFUZZ_WINDOWS 0
#elif __APPLE__
#define SECODEFUZZ_WINDOWS 0
#elif _WIN32
#define SECODEFUZZ_WINDOWS 1
#else
#error "Support for your platform has not been implemented"
#endif

#ifdef __x86_64
#define ATT_TARGET_POPCNT __attribute__((target("popcnt")))
#else
#define ATT_TARGET_POPCNT
#endif


#ifdef __clang__  // avoid gcc warning.
#  define ATT_NO_SANITIZE_MEMORY __attribute__((no_sanitize("memory")))
#  define ALWAYS_INLINE __attribute__((always_inline))
#else
#  define ATT_NO_SANITIZE_MEMORY
#  define ALWAYS_INLINE
#endif // __clang__

#define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))

#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
#    define ATT_NO_SANITIZE_ALL ATTRIBUTE_NO_SANITIZE_ADDRESS
#  elif __has_feature(memory_sanitizer)
#    define ATT_NO_SANITIZE_ALL ATT_NO_SANITIZE_MEMORY
#  else
#    define ATT_NO_SANITIZE_ALL
#  endif
#else
#  define ATT_NO_SANITIZE_ALL
#endif

#if SECODEFUZZ_WINDOWS
#define ATTRIBUTE_INTERFACE __declspec(dllexport)
#else
#define ATTRIBUTE_INTERFACE __attribute__((visibility("default")))
#endif
//========================================================
 

// 如果变异算法做成链表，则会做的更灵活，用户自定义变异算法就不需要重新编译lib代码了
struct MutaterGroup {
    const char*        name;
    int                     (*getCount)(SElement *pElement);
    char* 		        (*getValue)(SElement *pElement, int pos);
    int 			(*getIsSupport)(SElement *pElement);
    int 			isMutater;//可以通过这个变量禁止这个变异算法
};

typedef struct LlvmMemTable {
    int    has_value[SIZE_LLVM_MEM_TABLE];
    char A[SIZE_LLVM_MEM_TABLE][SIZE_LLVM_MEM_DATA + 1]; // 加1放置最大长度的/0
    char B[SIZE_LLVM_MEM_TABLE][SIZE_LLVM_MEM_DATA + 1];
    int    len1[SIZE_LLVM_MEM_TABLE];
    int    len2[SIZE_LLVM_MEM_TABLE];
    int     has_value_table[SIZE_LLVM_MEM_TABLE];
    int     has_value_total;
}SLlvmMemTable;

typedef struct LlvmNumberTableU64 {
    int    has_value[SIZE_LLVM_NUMBER_TABLE_U64];
    u64 A[SIZE_LLVM_NUMBER_TABLE_U64];
    u64 B[SIZE_LLVM_NUMBER_TABLE_U64];
    int  has_value_table[SIZE_LLVM_NUMBER_TABLE_U64];
    int   has_value_total;
}SLlvmNumberTableU64;

typedef struct  {
    SPara  para[MAX_PARA_NUM];
    int         paraNum;
    int         weight;
    int         randomSeed;
    int         mutatorCount; // 用于变异的次数
    int         newCorpusCount; // 由这个样本变异产生的新样本数量
    unsigned int       hash;
}S_corpus;

typedef struct  {
    S_corpus*        corpus[MAX_CORPUSP_NUM];
    S_corpus*        corpusBin[MAX_CORPUSP_NUM];

    int*            enumNumberTable[MAX_PARA_NUM];
    char**       enumStringTable[MAX_PARA_NUM];
    char**       enumBlobTable[MAX_PARA_NUM];
    int*            enumBloblTable[MAX_PARA_NUM];
    int             enumCount[MAX_PARA_NUM];

    int             min[MAX_PARA_NUM];
    int             max[MAX_PARA_NUM];

    int             arg[MAX_PARA_NUM];

    int             corpusIONumber;     // 样本文件总数量
    int             corpusReadNumber; // 仅是读样本文件读出来的数量
    int             corpusNum;              // 内存里总样本数量
    int             corpusBinNum;         // 读bin样本的数量
    int             newCorpusNum;       // 新进化出来的所有样本
    int             paraNum;
    int             runCount;

    char*        corpusPath;
}S_corpus_m;

typedef struct  {
    S_corpus_m  *corpusM;

    int                 corpusIsHasInit;
    S_corpus       tempCorpus;
    int                 *priorityCorpus;
    int                 priorityCorpusNum;
    int                 *midPriorityCorpus;
    int                 midPriorityCorpusNum;

    int                 currentCorpusPos;                // 目前正在使用哪个样本变异 
    char *            txt;
    char *            txt1;   

    int           temppNum;                           // 最大样本提示次数
    int           tempParaNum;                     // 读样本文件时临时用于存放参数数量
}S_corpus_module;


typedef struct  {
    int           isEnableFeature;                 // 使能trace-pc工作
    
    //trace-pc
    uintptr_t  *tracePcPcs;                         // 保存覆盖的pc指针
    uintptr_t  *tracePcEdges;                    // 保存用于记录pc跳转的值
    uintptr_t  *tracePcPcsTemp;                // 临时的用于最后覆盖率打印
    int           tracePcPcTatol;                    // 一共覆盖了多少代码块
    int           tracePcEdgeTatol;               // 一共覆盖了多少edge
    int           tracePcIsHasMalloc;            // 是否初始化

    //trace-pc, 单次执行有效
    uintptr_t  pc2Sum;                                // 

    //8bit
    char        *bit8Loops;                          // 存一个完整测试例的每个循环的运行状态，每个循环存8个状态
    uintptr_t  *bit8Hashs;                          //存放每个样本运行的路径hash
    int           bit8HashTatol;                     // 一共覆盖了多少路径
    int           bit8LoopTatol;                     //一共覆盖了多少典型的循环次数
    int           bit8IsHasMalloc;

    //8bit, 单次执行有效
    int           *bit8PerLoopCounters;                      // 存运行一次测试用例，每个代码块运行的次数，因为会很大，所以用整数
    
    char        *bit8LoopIdxHasBeenRecorded;        // 存放这个循环代码块指针是否已经被记录过, 一个指针只记录一次，多了影响速度                                       
    int           *bit8LoopIdxTable;                            // 存玄幻的idx，全局变量可能不太好
    int           bit8LoopIdxTatol;                              // 一共有多少个循环
    uintptr_t  bit8LoopSum;                                    //一次执行所有的loop循环和(加权后)
    
    char        *bit8PcIdxHasBeenRecorded;           // 存放这个代码块指针是否已经被记录过，一个指针只记录一次，多了影响速度
    int           *bit8PcIdxTable;                               // 存放运行一次测试用例，所有执行过得代码块指针'索引'
    int           bit8PcIdxTatol;                                 // 存放运行一次测试用例执行的代码块总数
    uintptr_t  bit8PcsSum;                                      //一次执行所有的pc指针和

    int           isHasNewFeature;                // 本次运行是否有新的路径产生，用于指示生成新样本
    int           weight;                                 // 样本的权重，用于变异样本选取
}S_tracepc_module;

typedef struct  {
    //llvmdata          实际上应该是全局变量，但那样需要加锁，懒得加，以后再说
    SLlvmNumberTableU64 *llvmNumberTableU64;
    SLlvmMemTable           *llvmMemTable;
    int                               llvmDataIsHasInit;                    // 如果没有初始化，则本模块不工作.目前不设置关闭本模块的功能
}S_hook_module;

// 全局对应
typedef struct  {
    const struct MutaterGroup* mutaterGroup[enum_MutatedMAX];     // 用于注册变异算法

    // 存放配置

    // 全局配置，只能外部设定，不清除
    int           isMutatedClose[enum_MutatedMAX];     // 用于保存变异算法开关
    int           maxOutputSize;                     // 存放默认最大变异长度
    int           stringHasTerminal;               // 定义输出的变异的字符串是否有/0结尾
    int           stringHasNull;
    int           isPrintPc;                               // 设置第一次运行到代码块，打印块pc指针到屏幕，默认关闭
    int           isDumpCoverage;                  // 设置是否输出覆盖率sancov文件，在clang编译器下有效
    int           isResetOnNewCorpus;           // 当产生新样本时，运行次数是否从新计算
    int           timeOutSecond;                    // 设置单个测试例执行一次超过多长时间报bug,如果为0，则不检测超时bug
    int           runningTimeSecond;            // 单个测试例运行时间，如果设置的运行次数没到而运行时间到了，提前结束
    int           isPrintfCrashCorpus;            // 设置是否在crash时打印crash样本在屏幕上,默认打印
    int           isLogOpen;                            // 存放日志开关,默认关闭
    int           maxCorpusnum;                   // 支持最大的样本数量
    int           smoothCorpusnum;                   // 支持最大的样本数量
    int           isNeedShowCorpus;              // 设置是否在测试例结束时打印样本在屏幕上,默认不打印
    int           isDelMismatchCopusFile;     // 设置是否删除不匹配的样本文件
    int           mutatorCountFromWeight;  // 设置一个权重值影响多少变异次数
    int           isEnableAsanReport;            // 设置是否注册asan报告回调，默认使能，在报告没去重的场景需要关闭
    int           isEnableFork;
    char*       dictionaryPath;
    char        reportPath[MAX_FILE_PATH];
    int           loopModeWeight;
    int           hashModeWeight;
    int           blockModeWeight;
    int           edgeModeWeight;
    int           stop;

    int           startCheckOutOfMemory;
    int           mallocLimitMb;
    int           rssLimitMb;
    int           mallocs;
    int           frees;
    int           mallocDebug;
    volatile int           isSelfMalloc;         //volatile 防止编译器优化部分操作
    int           enableDoLeakCheck;
    int           hasAddMallocCallBack;

    void        (*crashCallBack)(void);
    void        (*testCaseCallBack)(void);

    
    // 存放bin样本读入的路径，只一个测试用例有效，过后清除
    char*      binCorpusDirIn;
    int           binMaxLen;
    // 存放bin样本写出的路径，只一个测试用例有效，过后清除
    char*      binCorpusDirOut;

    // 存放其他
    int           isSIGINT;                              //是CTRL +C引起的
    int           isCrash;                                // 是crash引起的

    int           hasSignalInit;                      // Signal模块是否初始化
    char        date[40];
    int           nameNo;                              // 给样本文件参数起名字用的起始数字

    // 没啥用，存放加载的模块数量
    uint32_t *modules_Start[MAX_MODULE_NUM];      //加载模块多少，暂时没用
    uint32_t *modules_Stop[MAX_MODULE_NUM];
    int         numModules;                             // linker-initialized.--一共加载了多少个带覆盖率反馈的模块
    int         numGuards;                               // linker-initialized.--一共多少个代码块

    int          pcNo;
}SGlobal;

// 线程对应
typedef struct  {
    // 存放配置，线程有效
    int           temp_is_reproduce;            // 是否复现
    int           seed;
    unsigned  int next;
    char        runningTestcaseName[MAX_FILE_PATH];
    char*      testcaseName;                     // 用于存放测试用例名字  

    // 存放临时变量,单测试用例有效
    int           wholeRandomNum;              // 存放需要随机变异的参数数量
    int           runTime;                              // 用于计算一个测试用例的运行时间
    int           runningTimeStartTime;       // 用于存放一个测试用例的开始时间
    int           isRunningtimeComplete;     // 是否是设置的运行时间到了
    int           gostop;

    // leak
    int           isDoingLeak;                        // 正在做内存检测
    int           doLeakCount;                       // 内存检测次数倒计时

    //trace-pc
    S_tracepc_module tracepcModule;

    //llvmdata 
    S_hook_module hookModule;

    //corpus
    S_corpus_module corpusModule;

    //存放临时变量，单次执行有效
    int           isNeedRecordCrash;            // 本次执行是否记录crash样本
    int           isNeedRecordCorpus;           // 本次执行是否需要记录样本
    int           isNeedForceRecordCorpus;  // 本次执行是否需要强制记录样本
    int           isNeedWriteCorpus;           // 本次执行是否需要写入样本文件
    int           isNeedMutator;                    // 本次执行是否需要变异
    int           isfoundfailed;                      // 是否已经发现错误
    int           randomSeed;                      // 样本的随机seed，用于其他软件变异联动使用
    int           isPass;                                //是否发现bug
    char        asanReportName[MAX_FILE_PATH];
    int          asanReportLen;       // 打印过的asan报告长度

    //纯临时变量
    char*      valueBuf;                            // 变异算法生成值的临时变量
    int           isHasInit;                            // 上边那个是否初始化了
    char        pcDescr[MAX_PC_DESCR_SIZE];
    char        hashString[41];                      // 临时hash字符串

    int           isRunningTestCode;              //是否在运行测试用例代码
}STGlobal;

// 全局变量

extern SGlobal g_global;

#ifdef SUPPORT_M_THREAD 
extern __thread STGlobal g_globalThead;
#else
extern STGlobal g_globalThead;
#endif

extern u64 g_edgeCaseTable[];

extern char const *g_customBlobTable[];
extern  int const g_customBlobTableLen[];
extern  int g_customBlobTableCount;

extern u64 const g_customNumberTable[];
extern int g_customNumberTableCount;

extern char const *g_customStringTable[];
extern int g_customStringTableCount;

extern char const *g_stringStaticTable[];
extern int g_stringStaticTableLen;

extern u8 const g_bom[];
extern int g_bomLen;

extern char g_specialByte[];
extern int g_specialByteLen;


// internal
extern u32 InGetBitNumber(u32 n);
extern int InStringIsNumber(SElement *pElement);
extern const char *InGetStringFromType(int type);
extern int InGetTypeFromString(char* type_name);
extern const char *InGetStringFromInType(int type);
extern int InGetInTypeFromString(char* type_name);

extern int InGetTypeIsEnumOrRange(int Type);
extern int InGetTypeIsChangeLength(int Type);
extern int InGetParaNum(void);


extern char *SetElementOriginalValue(SElement *pElement);
extern char *SetElementInitoutBuf(SElement *pElement, int len);
extern char *SetElementInitoutBufEx(SElement *pElement, int len);
extern char* MagicGetValue(SElement *pElement, char* data, int len, int type);

extern int InGetBufZeroNumber(char* string, int len);
extern void InGetRegion(int length, int *outStart, int *outLength);
extern int InGetLetterNumber(char* string);
extern int InToUpper(int c);
extern int InToLower(int c);
extern int InIsLetter(char c);
extern int InIsAscii(char c);
extern int InIsPrint(char c);

extern int InIsxDigit(char c);
extern int InIsDigit(char c);
extern u32 Instrlen(const char *s);
extern u64 Insqrt(u64 x);
extern char *Inltoa(s64 value, char *string, int radix);
extern s64 Inatol(char *string);
extern s64 Inhtol(char *s);
extern void InDelSpace(char *str);  

extern int InParseStringToBin(char *str, char* buf);
extern int InParseBinToString(char *str, char* buf, int len);
extern int InParseBinToHexString(char *str, char* buf, int len);


u8  InBswap8(u8 x);
u16 InBswap16(u16 x);
u32 InBswap32(u32 x);
u64 InBswap64(u64 x);

// external
extern int HwTime(void);
extern void HwSetTimer(int Seconds);
extern char* HwMalloc(size_t size);
extern void HwFree(void* ptemp);
extern void *HwMemset(void *s, int ch, size_t n);
extern void *HwMemcpy(void *dest, const void *src, size_t n);
extern void *HwMemMove(void *dest, const void *src, size_t n);
extern int HwMemCmp(const void *buf1, const void *buf2, unsigned int count);
extern long int HwStrToL(const char *nptr, char **endptr, int base);
extern int HwStrCmp(const char *s1, const char *s2);
extern int HwRANDMAX(void);
extern int HwRand(void);
extern void HwSrand(unsigned int temp);
extern int HwFork(void);
extern int HwWait(void);
extern void HwExit(int no);
extern int HwWEXITSTATUS(int status);
extern void HwPthread_create(void (*fun)(void));
extern int HWGetPeakRssMb(void);
extern void HWSleep(int time);


extern int HwGetTime(void);
extern char* HwGetDate(void);

// random
extern u32 GaussRandU32(u32 pos);
extern s32 GaussRandS32(u32 pos);
extern u64 GaussRandU64(u32 pos);
extern s64 GaussRandS64(u32 pos);
extern int GetIsMutated(void);

// Mutator
extern void InitDataElementBitFill(void);
extern void InitDataElementBitFlipper(void);
extern void InitDataElementBitZero(void);
extern void InitDataElementChangeASCIIInteger(void);
extern void InitDataElementMBitFlipper(void);
extern void InitDataElementDuplicate(void);
extern void InitDataElementLengthEdgeCase(void);
extern void InitDataElementLengthRandom(void);
extern void InitDataElementLengthGauss(void);
extern void InitDataElementLengthRepeatPart(void);
extern void InitDataElementReduce(void);
extern void InitDataElementByteRandom(void);
extern void InitDataElementOneByteInsert(void);
extern void InitDataElementSwapTwoPart(void);

extern void InitDataElementStringStatic(void);
extern void InitDataElementCopyPartOf(void);
extern void InitDataElementInsertPartOf(void);
extern void InitDataElementAFL(void);
extern void InitDataElementMagic(void);
extern void InitDataElementChange(void);
extern void InitDataElementMagicChange(void);


extern void InitNumberEdgeCase(void);
extern void InitNumberEdgeRange(void);
extern void InitNumberRandom(void);
extern void InitNumberVariance(void);
extern void InitNumberSmallRange(void);
extern void InitNumberPowerRandom(void);
extern void InitNumberMagic(void);
extern void InitNumberEnum(void);
extern void InitNumberRange(void);

extern void InitStringAsciiRandom(void);
extern void InitStringLengthAsciiRandom(void);
extern void InitStringCaseLower(void);
extern void InitStringCaseRandom(void);
extern void InitStringCaseUpper(void);
extern void InitStringLengthEdgeCase(void);
extern void InitStringLengthRandom(void);
extern void InitStringLengthGauss(void);
extern void InitStringUtf8Bom(void);
extern void InitStringUtf8BomLength(void);
extern void InitStringUtf8BomStatic(void);
extern void InitStringStatic(void);
extern void InitStringMagic(void);
extern void InitStringEnum(void);

extern void InitBlobChangeBinaryInteger(void);
extern void InitBlobChangeFromNull(void);
extern void InitBlobChangeRandom(void);
extern void InitBlobChangeSpecial(void);
extern void InitBlobChangeToNull(void);
extern void InitBlobExpandAllRandom(void);
extern void InitBlobExpandSingleIncrementing(void);
extern void InitBlobExpandSingleRandom(void);
extern void InitBlobExpandZero(void);
extern void InitBlobMagic(void);
extern void InitBlobEnum(void);

extern void InitIpv4(void);
extern void InitIpv6(void);
extern void InitMac(void);
extern void InitFloat16(void);
extern void InitFloat32(void);
extern void InitFloat64(void);
extern void InitDouble(void);
extern void InitSelf(void);
extern void InitCustomMutator(void);
extern void InitCross(void);



extern void InitCustomNumber(void);
extern void InitCustomString(void);
extern void InitCustomBlob(void);

extern void InitPcCounters(void);
extern void Init8BitCounters(void);
extern void InitKcov(void);
extern void InitLlvmData(void);

extern void InitCorpus(void);
extern void InitSignalCallback(void);

// llvmdata
extern void LlvmDataNumberAddValue(void *callerPc, u64 s1, u64 s2);
extern u64 LlvmDataNumberGetValue(void);
extern int  LlvmDataNumberGetCount(void);
extern void LlvmDataMemAddValue(void *callerPc, const char* s1, const char* s2, size_t n1, size_t n2);
extern void LlvmDataMemAddValueEx(void *callerPc, const char* s1, const char* s2);
extern char* LlvmDataMemGetValue(int *len);
extern int  LlvmDataMemGetCount(void);
extern void CleanLlvmData(void);


// llvm
extern int LlvmHookIsSupport(void);
extern void LlvmHookRegisterAsanCallBack(void (* fun)(void));
extern void LlvmHookRegisterAsanCallBackno(void (* fun)(void));
extern void LlvmHookRegisterAsanCallBackReport(void (* fun)(char *));
extern void LlvmHookPrintStackTrace(void);

// llvmTracePC
extern int LlvmTracePcIsHasNewFeature(void);
extern void LlvmTracePcStartFeature(void);
extern void LlvmTracePcEndFeature(void);
extern void LlvmTracePcSetHasNewFeature(int weight);
extern int LlvmTracePcGetWeight(void);

// llvmpccounters
extern void LlvmRecordGuards(uint32_t *start, uint32_t *stop);
extern void LlvmDoTracePc(int idx , uintptr_t pc);
extern void LlvmRecordPcCounters(int idx , uintptr_t pc);
extern void LlvmRecordPcCountersEdga(int idx , uintptr_t pcSum);
extern  void LlvmSetIsDumpCoverage(int isDumpCoverage);
extern  void LlvmSetIsPrintNewPC(int isPrintPC);
extern  void LlvmDumpCoverage(void);
extern  int LlvmGetPcTatol(void);
extern  int LlvmGetEdgeTatol(void);
extern  void LlvmCleanPcCounters(void);

// llvm8bitcounters
extern void LlvmRecord8BitCounters(int idx, uintptr_t pc);
extern void LlvmDo8BitCounters(void);
extern int LlvmGetLoopTatol(void);
extern int LlvmGetHashTatol(void);
extern void LlvmClean8BitCounters(void);



// llvmLeakCheck
extern void LlvmEnableLeakCheck(int count);
extern void LlvmDoLeakCheck(void);

extern void MemoryStart(int mallocLimitMb, int rssLimitMb);
extern void MemoryStop(void);


// kcov
extern int KcovIsHasNewFeature(void);
extern int KcovGetWeight(void);
extern void KcovStartFeature(void);
extern void KcovEndFeature(void);

// IO
extern int OpenDir(char* dirPath);
extern char* ReadDir(void);
extern void CloseDir(void);
extern void WriteToFile(char* data, int len, char *path);
extern void WriteToFileFail(char* data, int len, char *path);
extern void ReadFromFile(char** data, int *len, char *path);
extern void RemoveFile(char *path);
extern void RenameFile(char *path ,char *newpath); 

// Hash
extern  char* Hash(char* buf, int len);
extern  unsigned int HwCheckSum32( const unsigned char *buf, unsigned int size);


// Common
extern void SetMaxOutputSize(int imaxOutputSize);
extern void SetStringHasTerminal(int isHasTerminal);
extern int    RegisterMutater(const struct MutaterGroup* pMutaterGroup, enum EnumMutated mutatedNum);

// Report
extern  void ReportSetPath(char* path);
extern  void ReportSetFixPathName(char* pathName);
extern  void ReportWriteFailedTestCase(char *testcaseName, int seed, int runCount, int runTime, int readCorpusNum, int newCorpusNum);
extern  void ReportWriteFailedTestCase1(void);
extern  void ReportWriteSucceedTestCase(char *testcaseName, int seed, int runCount, int runTime, int readCorpusNum, int newCorpusNum);
extern  int ReportGetTime(void);
extern  void ReportSetRunningTestCaseName(char* name);

// Corpus
extern  void InitCorpusMalloc(void);
extern  void CorpusStart(int isReproduce);
extern  int   CorpusCheck(void);
extern  int   CorpusEnd(void);
extern  int CorpusGetCorpusNum(void);
extern  int CorpusGetReadCorpusNum(void);
extern  int CorpusGetNewCorpusNum(void);
extern  void CorpusStartFeature(void);
extern  void CorpusEndFeature(void);
extern  void CleanOneCorpus(S_corpus* corpus);
extern void CleanCorpus(void);
extern void CleanCorpusMemory(void);


// CorpusIO
extern  void CorpusSetIfShowCrash(int isShowAll);
extern  void CorpusSetIfShow(int isShowAll);
extern  void CorpusBinPrintf(char *path);
extern  void CorpusBinWrite(char *path);
extern  void CorpusCorpusWrite(char *path);
extern  void CorpusShowAll(void);
extern  void CorpusSetPath(char* path);
extern  int CorpusShowCur(void);
extern  int CorpusAsanReportWrite(char * report);
extern  void ReadAllCorpus(char *path); 
extern  void ReadAllBinCorpus(char *dirPath);
extern  unsigned int ElementGetHash(void);
extern  void CorpusGetValueAndLen(char** buf,int * len,int pos);
extern  unsigned int CorpusGetHash(int pos);
extern  void CorpusWrite(int pos);
extern  void CorpusWriteBin(int pos);


// CorpusSchedule
extern int CorpusPriorityAdd(int pos, int weigth);
extern int CorpusSelect(void);
extern int CorpusDiscard(void);
extern void CorpusParaValueGet(int corpusPos, int paraPos, char **buf, int *len);


// common
extern  const char *GetVersion(void);

// time
extern  void TimeOutSetSecond(int second);
extern  void TimeOutset(void);
extern  void TimeOutClean(void);
extern  void InitTimeOut(void);


extern  void RunningSetTimeSecond(int second);
extern  void RunningTimeGetStart(void);
extern  int RunningTimeIsOver(void);


extern void CallBackAddCrash(void (*fun)(void));
extern void CallBackAddTestCase(void (*fun)(void));
extern void CallBackRunCrash(void);
extern void CallBackRunTestCase(void);


// 控制debug开关函数
extern  void OpenLog(void);
extern  void CloseLog(void);

// env
void ClearFuzzEnvironment (void);

/******************************************

3.

******************************************/
extern void DTInitValueBuffer(SElementInit *init, int length, char* initValue);
extern char*  DTGetFuzzValueEx(SElementInit *pInit);

/******************************************

4.接口三，这套接口不建议使用了

******************************************/

// 下边为封装后的创建元素结构体的函数，节省用户代码行数

// 创建元素结构体函数，即对外提供，又内部调用
extern  void CreatElementEx(
    SElement *pElement, int isNeedFree, int isHasInitValue, int type, char* inBuf, int inLen);
extern  void FreeMutatedValue(SElement *pElement);
extern  void FreeElement(SElement *pElement);

extern  SElement *CreatElementS64(int isHasInitValue, s64 initValue);
extern  SElement *CreatElementS32(int isHasInitValue, s32 initValue);
extern  SElement *CreatElementS16(int isHasInitValue, s16 initValue);
extern  SElement *CreatElementS8(int isHasInitValue, s8 initValue);
extern  SElement *CreatElementU64(int isHasInitValue, u64 initValue);
extern  SElement *CreatElementU32(int isHasInitValue, u32 initValue);
extern  SElement *CreatElementU16(int isHasInitValue, u16 initValue);
extern  SElement *CreatElementU8(int isHasInitValue, u8 initValue);
extern  SElement *CreatElementNumberEnum(int isHasInitValue, u32 initValue, int* eunmTable, int  eunmCount);
extern  SElement *CreatElementNumberRange(int isHasInitValue, u32 initValue, int min, int  max);
extern  SElement *CreatElementFloat(int isHasInitValue, float initValue);
extern  SElement *CreatElementDouble(int isHasInitValue, double initValue);
extern  SElement *CreatElementString(
    int isHasInitValue, char* initValue, int len, int maxLen); // len字符串长度，strlen(str) + 1
extern  SElement *CreatElementStringNum(int isHasInitValue, char* initValue, int len, int maxLen);
extern  SElement *CreatElementStringEnum(
    int isHasInitValue, char* initValue, int len, int maxLen, char* eunmTableS[], int  eunmCount);
extern  SElement *CreatElementBlob(
    int isHasInitValue, char* initValue, int len, int maxLen);
extern  SElement *CreatElementBlobEnum(
    int isHasInitValue, char* initValue, int len, int maxLen, char* eunmTableB[], int eunmTableL[], int  eunmCount);
extern  SElement *CreatElementFixBlob(int isHasInitValue, char* initValue, int len, int maxLen);
extern  SElement *CreatElementIpv4(int isHasInitValue, char* initValue);
extern  SElement *CreatElementIpv6(int isHasInitValue, char* initValue);
extern  SElement *CreatElementMac(int isHasInitValue, char* initValue);
extern  SElement *CreatElementFloat16(int isHasInitValue, char* initValue);
extern  SElement *CreatElementFloat32(int isHasInitValue, char* initValue);
extern  SElement *CreatElementFloat64(int isHasInitValue, char* initValue);
extern  SElement *CreatElementDouble64(int isHasInitValue, char* initValue);
extern  SElement *CreatElementSelf(int isHasInitValue, char* initValue, int len, int maxLen, int arg);


// 获取测试例的统一接口，获取后需自己强制转换成自己想要的类型
extern  char* GetMutatedValueRandom(SElement *pElement);
extern  char* GetMutatedValueSequence(SElement *pElement, int pos);
extern  int GetMutatedValueLen(SElement *pElement);
extern  void SetMutatedFrequency(SElement *pElement, int frequency);
extern  int GetIsBeMutated(SElement *pElement);


/******************************************

5.其他辅助接口

******************************************/
extern void InitCommon(void);
extern void ClearMemcory(void);
extern int GetWeight(void);

// 设置变异种子，同样的seed和range回放出来的测试例是一样的
// 如果seed设置为0，则系统会随机根据time函数生成seed,如果系统不支持time函数，则seed设置成1
extern void InitSeed(int initSeed, int startRange);

// 全局随机函数
extern  void AddWholeRandom(SElement *pElement);
extern  void LeaveWholeRandom(SElement *pElement);

// 下边函数操作全局变异算法开关，对外提供
extern  void	SetCloseAllMutater(void);
extern  void	SetOpenAllMutater(void);
extern  void	SetCloseOneMutater(enum EnumMutated  mutatedNum);
extern  void	SetOpenOneMutater(enum EnumMutated  mutatedNum);

// 下边函数操作单独元素变异算法开关，对外提供
extern  void	SetElementCloseAllMutater(SElement *pElement);
extern  void	SetElementOpenAllMutater(SElement *pElement);
extern  void	SetElementCloseOneMutater(SElement *pElement, enum EnumMutated mutatedNum);
extern  void	SetElementOpenOneMutater(SElement *pElement, enum EnumMutated mutatedNum);

// 16进制buf打印函数
extern  void HexDump(u8 *buf, u32 len);

// 调试函数
extern  void DebugElement(SElement *pElement);

#ifdef __cplusplus
}
#endif
#endif

