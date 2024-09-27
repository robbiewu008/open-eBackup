/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018
����:
wanghao 			w00296180
wangchengyun 	wwx412654


*/

#ifndef _COMMON_H
#define _COMMON_H

#include "../secodeFuzz.h"

#ifdef __cplusplus
extern "C" {
#endif

// Ŀǰ�ⶫ�������ڴ�ӡ���ܲ���������ģ��м��м�
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
    ENUM_CHANGE	= 3, // �滻һ���֣����Ȳ���
};

#define INTEGER_8		8


#define EXIT_CODE		             50
#define EXIT_CODE_LEAK		      51
#define EXIT_CODE_OTHER             52
#define EXIT_CODE_RssLimit          53
#define EXIT_CODE_MallocLimit     54


// Corpus

// ���������������ܳ���10000��ʵ���ϳ�����Ҳûɶ������
#define MAX_CORPUSP_NUM                        10000
#define MAX_Corpus_Priority_Num            (MAX_CORPUSP_NUM*4)
#define MAX_Corpus_Hash_Num                (MAX_CORPUSP_NUM*10)
#define NewPc_Weight                                30
#define NewEdge_Weight                            3 //��ʱ��С��holdס������ʱ��
#define NewLoop_Weight                            20
#define NewRoute_Weight                          1
#define NewCorpus_Weight                        1
#define DEFAULT_MUTATOR_COUNT           1000

#define SMOOTH_CORPOS_NUM           (DEFAULT_CORPOS_NUM * 2 / 3)


// �����������󳤶�ΪMAX_valueBuf��һ���ڴ����4���ֽڱ�ʾ
// ����\x00  �����1000�ֽڱ�ʾ���ֵȺ�ɶ��:)
// ���������ļ�һ����������
#define MAX_ONE_LINE  ((DEFAULT_MAX_OUTPUT_SIZE*(4|0ULL)+1000)  * 2)
#define SWITCH_CORPUS_COUNT 100                 // �����������л��ĵ����������
#define SWITCH_SUB_COUNT        4                   // ����һ������ʹ���У�ʹ�ñ���ֵ�ٴα���ĵ��Ӵ���

// Peport
#define MAX_FILE_PATH 512                             // ���������ļ�·���ַ�������

// Trace_PC
#define MAX_PC_NUM  (1<<21)                         // ���屣�����Ĵ����pcָ������
#define MAX_MODULE_NUM  4096                     // �������ģ������
#define MAX_PC_DESCR_SIZE  1024                 // �����������pc��ӡ�ַ����ĳߴ�

// LLVMData
#define SIZE_LLVM_MEM_TABLE  20480           // �ڴ�hook���������ڴ����ݵ�����
#define SIZE_LLVM_MEM_DATA  40                // ÿ�����ݲ�Ӧ����40�ֽڣ�̫����ò��Ҳûɶ���� ::��Լ1M�ڴ�ռ��
#define SIZE_LLVM_NUMBER_TABLE_U64  102400   // ����hook���������������ݵ�����

// Kcov
#define MAX_KERNEL_PC_NUM  (1<<21)

// Common
#define DEBUG_MAX_OUTPUT_SIZE  1000        // ��ӡ���������������ֽ���

#define MAX_COUNT                500                    // һ�������㷨 ���������������
#define STRING_NUMBER_LEN       33

#define MIN(_a, _b)     (((_a) > (_b)) ? (_b) : (_a))
#define MAX(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))

#define IS_ADD_ONE(x)   (((x) > (0)) ? (1) : (0))

#define POS_ORIGINAL    0xffffffff

// Internal
#define IS_USE_GLOBAL_MALLOC            1       // ����Ϊ�������ݷ�����ڴ��Ƿ�ʹ��һ���Է���Ĵ��ڴ�

// ��bit��ת
#define FLIP_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] ^= (128 >> ((bf) & 7)); \
    } while (0)

// ��bit��0
#define ZERO_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] &= ~(128 >> ((bf) & 7)); \
    } while (0)

// ��bit��1
#define FILL_BIT(ar, b) do { \
        u8* arf = (u8*)(ar); \
        u32 bf = (b); \
        arf[(bf) >> 3] |= (128 >> ((bf) & 7)); \
    } while (0)

#define ONE_IN(x)				((HwRand() % (x)) == 0)	// limit of RAND_MAX-1
#define RAND_16()				(HwRand() & 0xFFFF)
#define RAND_32()				(((0UL | HwRand()) << 1) | (HwRand() & 1))
// rand_64ΪʲôҪ����д
#define RAND_64()				(((0ULL | HwRand()) << 33) | ((0ULL | HwRand()) << 2) | (HwRand() & 0x3))
#define RAND_BOOL()				(HwRand() & 1)
#define RAND_BYTE()				(HwRand() & 0xff)

#define RAND_RANGE(min, max)	((min) + (HwRand() % ((max) - (min) + 1)))
#define RAND_RANGE64(min, max)	((min) + (RAND_64() % ((max) - (min) + 1)))

//====================== �꿪�� ======================
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

// #ifndef _MSC_VER //Ϊ�˺���������ע�͵�
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
 

// ��������㷨��������������ĸ����û��Զ�������㷨�Ͳ���Ҫ���±���lib������
struct MutaterGroup {
    const char*        name;
    int                     (*getCount)(SElement *pElement);
    char* 		        (*getValue)(SElement *pElement, int pos);
    int 			(*getIsSupport)(SElement *pElement);
    int 			isMutater;//����ͨ�����������ֹ��������㷨
};

typedef struct LlvmMemTable {
    int    has_value[SIZE_LLVM_MEM_TABLE];
    char A[SIZE_LLVM_MEM_TABLE][SIZE_LLVM_MEM_DATA + 1]; // ��1������󳤶ȵ�/0
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
    int         mutatorCount; // ���ڱ���Ĵ���
    int         newCorpusCount; // ����������������������������
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

    int             corpusIONumber;     // �����ļ�������
    int             corpusReadNumber; // ���Ƕ������ļ�������������
    int             corpusNum;              // �ڴ�������������
    int             corpusBinNum;         // ��bin����������
    int             newCorpusNum;       // �½�����������������
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

    int                 currentCorpusPos;                // Ŀǰ����ʹ���ĸ��������� 
    char *            txt;
    char *            txt1;   

    int           temppNum;                           // ���������ʾ����
    int           tempParaNum;                     // �������ļ�ʱ��ʱ���ڴ�Ų�������
}S_corpus_module;


typedef struct  {
    int           isEnableFeature;                 // ʹ��trace-pc����
    
    //trace-pc
    uintptr_t  *tracePcPcs;                         // ���渲�ǵ�pcָ��
    uintptr_t  *tracePcEdges;                    // �������ڼ�¼pc��ת��ֵ
    uintptr_t  *tracePcPcsTemp;                // ��ʱ��������󸲸��ʴ�ӡ
    int           tracePcPcTatol;                    // һ�������˶��ٴ����
    int           tracePcEdgeTatol;               // һ�������˶���edge
    int           tracePcIsHasMalloc;            // �Ƿ��ʼ��

    //trace-pc, ����ִ����Ч
    uintptr_t  pc2Sum;                                // 

    //8bit
    char        *bit8Loops;                          // ��һ��������������ÿ��ѭ��������״̬��ÿ��ѭ����8��״̬
    uintptr_t  *bit8Hashs;                          //���ÿ���������е�·��hash
    int           bit8HashTatol;                     // һ�������˶���·��
    int           bit8LoopTatol;                     //һ�������˶��ٵ��͵�ѭ������
    int           bit8IsHasMalloc;

    //8bit, ����ִ����Ч
    int           *bit8PerLoopCounters;                      // ������һ�β���������ÿ����������еĴ�������Ϊ��ܴ�����������
    
    char        *bit8LoopIdxHasBeenRecorded;        // ������ѭ�������ָ���Ƿ��Ѿ�����¼��, һ��ָ��ֻ��¼һ�Σ�����Ӱ���ٶ�                                       
    int           *bit8LoopIdxTable;                            // �����õ�idx��ȫ�ֱ������ܲ�̫��
    int           bit8LoopIdxTatol;                              // һ���ж��ٸ�ѭ��
    uintptr_t  bit8LoopSum;                                    //һ��ִ�����е�loopѭ����(��Ȩ��)
    
    char        *bit8PcIdxHasBeenRecorded;           // �����������ָ���Ƿ��Ѿ�����¼����һ��ָ��ֻ��¼һ�Σ�����Ӱ���ٶ�
    int           *bit8PcIdxTable;                               // �������һ�β�������������ִ�й��ô����ָ��'����'
    int           bit8PcIdxTatol;                                 // �������һ�β�������ִ�еĴ��������
    uintptr_t  bit8PcsSum;                                      //һ��ִ�����е�pcָ���

    int           isHasNewFeature;                // ���������Ƿ����µ�·������������ָʾ����������
    int           weight;                                 // ������Ȩ�أ����ڱ�������ѡȡ
}S_tracepc_module;

typedef struct  {
    //llvmdata          ʵ����Ӧ����ȫ�ֱ�������������Ҫ���������üӣ��Ժ���˵
    SLlvmNumberTableU64 *llvmNumberTableU64;
    SLlvmMemTable           *llvmMemTable;
    int                               llvmDataIsHasInit;                    // ���û�г�ʼ������ģ�鲻����.Ŀǰ�����ùرձ�ģ��Ĺ���
}S_hook_module;

// ȫ�ֶ�Ӧ
typedef struct  {
    const struct MutaterGroup* mutaterGroup[enum_MutatedMAX];     // ����ע������㷨

    // �������

    // ȫ�����ã�ֻ���ⲿ�趨�������
    int           isMutatedClose[enum_MutatedMAX];     // ���ڱ�������㷨����
    int           maxOutputSize;                     // ���Ĭ�������쳤��
    int           stringHasTerminal;               // ��������ı�����ַ����Ƿ���/0��β
    int           stringHasNull;
    int           isPrintPc;                               // ���õ�һ�����е�����飬��ӡ��pcָ�뵽��Ļ��Ĭ�Ϲر�
    int           isDumpCoverage;                  // �����Ƿ����������sancov�ļ�����clang����������Ч
    int           isResetOnNewCorpus;           // ������������ʱ�����д����Ƿ���¼���
    int           timeOutSecond;                    // ���õ���������ִ��һ�γ����೤ʱ�䱨bug,���Ϊ0���򲻼�ⳬʱbug
    int           runningTimeSecond;            // ��������������ʱ�䣬������õ����д���û��������ʱ�䵽�ˣ���ǰ����
    int           isPrintfCrashCorpus;            // �����Ƿ���crashʱ��ӡcrash��������Ļ��,Ĭ�ϴ�ӡ
    int           isLogOpen;                            // �����־����,Ĭ�Ϲر�
    int           maxCorpusnum;                   // ֧��������������
    int           smoothCorpusnum;                   // ֧��������������
    int           isNeedShowCorpus;              // �����Ƿ��ڲ���������ʱ��ӡ��������Ļ��,Ĭ�ϲ���ӡ
    int           isDelMismatchCopusFile;     // �����Ƿ�ɾ����ƥ��������ļ�
    int           mutatorCountFromWeight;  // ����һ��Ȩ��ֵӰ����ٱ������
    int           isEnableAsanReport;            // �����Ƿ�ע��asan����ص���Ĭ��ʹ�ܣ��ڱ���ûȥ�صĳ�����Ҫ�ر�
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
    volatile int           isSelfMalloc;         //volatile ��ֹ�������Ż����ֲ���
    int           enableDoLeakCheck;
    int           hasAddMallocCallBack;

    void        (*crashCallBack)(void);
    void        (*testCaseCallBack)(void);

    
    // ���bin���������·����ֻһ������������Ч���������
    char*      binCorpusDirIn;
    int           binMaxLen;
    // ���bin����д����·����ֻһ������������Ч���������
    char*      binCorpusDirOut;

    // �������
    int           isSIGINT;                              //��CTRL +C�����
    int           isCrash;                                // ��crash�����

    int           hasSignalInit;                      // Signalģ���Ƿ��ʼ��
    char        date[40];
    int           nameNo;                              // �������ļ������������õ���ʼ����

    // ûɶ�ã���ż��ص�ģ������
    uint32_t *modules_Start[MAX_MODULE_NUM];      //����ģ����٣���ʱû��
    uint32_t *modules_Stop[MAX_MODULE_NUM];
    int         numModules;                             // linker-initialized.--һ�������˶��ٸ��������ʷ�����ģ��
    int         numGuards;                               // linker-initialized.--һ�����ٸ������

    int          pcNo;
}SGlobal;

// �̶߳�Ӧ
typedef struct  {
    // ������ã��߳���Ч
    int           temp_is_reproduce;            // �Ƿ���
    int           seed;
    unsigned  int next;
    char        runningTestcaseName[MAX_FILE_PATH];
    char*      testcaseName;                     // ���ڴ�Ų�����������  

    // �����ʱ����,������������Ч
    int           wholeRandomNum;              // �����Ҫ�������Ĳ�������
    int           runTime;                              // ���ڼ���һ����������������ʱ��
    int           runningTimeStartTime;       // ���ڴ��һ�����������Ŀ�ʼʱ��
    int           isRunningtimeComplete;     // �Ƿ������õ�����ʱ�䵽��
    int           gostop;

    // leak
    int           isDoingLeak;                        // �������ڴ���
    int           doLeakCount;                       // �ڴ����������ʱ

    //trace-pc
    S_tracepc_module tracepcModule;

    //llvmdata 
    S_hook_module hookModule;

    //corpus
    S_corpus_module corpusModule;

    //�����ʱ����������ִ����Ч
    int           isNeedRecordCrash;            // ����ִ���Ƿ��¼crash����
    int           isNeedRecordCorpus;           // ����ִ���Ƿ���Ҫ��¼����
    int           isNeedForceRecordCorpus;  // ����ִ���Ƿ���Ҫǿ�Ƽ�¼����
    int           isNeedWriteCorpus;           // ����ִ���Ƿ���Ҫд�������ļ�
    int           isNeedMutator;                    // ����ִ���Ƿ���Ҫ����
    int           isfoundfailed;                      // �Ƿ��Ѿ����ִ���
    int           randomSeed;                      // ���������seed���������������������ʹ��
    int           isPass;                                //�Ƿ���bug
    char        asanReportName[MAX_FILE_PATH];
    int          asanReportLen;       // ��ӡ����asan���泤��

    //����ʱ����
    char*      valueBuf;                            // �����㷨����ֵ����ʱ����
    int           isHasInit;                            // �ϱ��Ǹ��Ƿ��ʼ����
    char        pcDescr[MAX_PC_DESCR_SIZE];
    char        hashString[41];                      // ��ʱhash�ַ���

    int           isRunningTestCode;              //�Ƿ������в�����������
}STGlobal;

// ȫ�ֱ���

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


// ����debug���غ���
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

4.�ӿ��������׽ӿڲ�����ʹ����

******************************************/

// �±�Ϊ��װ��Ĵ���Ԫ�ؽṹ��ĺ�������ʡ�û���������

// ����Ԫ�ؽṹ�庯�����������ṩ�����ڲ�����
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
    int isHasInitValue, char* initValue, int len, int maxLen); // len�ַ������ȣ�strlen(str) + 1
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


// ��ȡ��������ͳһ�ӿڣ���ȡ�����Լ�ǿ��ת�����Լ���Ҫ������
extern  char* GetMutatedValueRandom(SElement *pElement);
extern  char* GetMutatedValueSequence(SElement *pElement, int pos);
extern  int GetMutatedValueLen(SElement *pElement);
extern  void SetMutatedFrequency(SElement *pElement, int frequency);
extern  int GetIsBeMutated(SElement *pElement);


/******************************************

5.���������ӿ�

******************************************/
extern void InitCommon(void);
extern void ClearMemcory(void);
extern int GetWeight(void);

// ���ñ������ӣ�ͬ����seed��range�طų����Ĳ�������һ����
// ���seed����Ϊ0����ϵͳ���������time��������seed,���ϵͳ��֧��time��������seed���ó�1
extern void InitSeed(int initSeed, int startRange);

// ȫ���������
extern  void AddWholeRandom(SElement *pElement);
extern  void LeaveWholeRandom(SElement *pElement);

// �±ߺ�������ȫ�ֱ����㷨���أ������ṩ
extern  void	SetCloseAllMutater(void);
extern  void	SetOpenAllMutater(void);
extern  void	SetCloseOneMutater(enum EnumMutated  mutatedNum);
extern  void	SetOpenOneMutater(enum EnumMutated  mutatedNum);

// �±ߺ�����������Ԫ�ر����㷨���أ������ṩ
extern  void	SetElementCloseAllMutater(SElement *pElement);
extern  void	SetElementOpenAllMutater(SElement *pElement);
extern  void	SetElementCloseOneMutater(SElement *pElement, enum EnumMutated mutatedNum);
extern  void	SetElementOpenOneMutater(SElement *pElement, enum EnumMutated mutatedNum);

// 16����buf��ӡ����
extern  void HexDump(u8 *buf, u32 len);

// ���Ժ���
extern  void DebugElement(SElement *pElement);

#ifdef __cplusplus
}
#endif
#endif

