/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
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

// ====================== �꿪�� ======================
#define LIB_DEFINE                              // ϵͳ��궨��
#define HAS_IO                                              // �ļ���д
#define HAS_HOOK
#define HAS_TRACE_PC
#define HAS_LEAK_CHECK
#define HAS_SIGNAL
// #define HAS_KCOV

// ���߳�֧�ֽ���Ϊ�˷��ֶ��̷߳������������
// ����Ϊ����߲����ٶ�ʹ��������߲����ٶ���ʹ�ö����
// �����������Բ�Ҫ����
// #define SUPPORT_M_THREAD

// vs������Ҫ�ü���
#ifdef _MSC_VER

#ifndef __clang__
#undef HAS_HOOK
#undef HAS_TRACE_PC
#undef HAS_LEAK_CHECK
#undef HAS_SIGNAL
#endif
#undef HAS_KCOV
#endif

// ����ں�̬�����ü���
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

����һЩ���

******************************************/
#ifdef __KERNEL__
#define MAX_PARA_NUM                30                          // ����һ��������֧�ֵ�����������
#define DEFAULT_CORPOS_NUM            10                             // ����һ��������֧�ֵ������������
#define DEFAULT_MAX_OUTPUT_SIZE   100000           // ����Ĭ�ϵ�maxOutputSize
#else
#define MAX_PARA_NUM                256                         // ����һ��������֧�ֵ�����������
#define DEFAULT_CORPOS_NUM            5000               // ����һ��������֧�ֵ�Ĭ�������������
#define DEFAULT_MAX_OUTPUT_SIZE   10000000        // ����Ĭ�ϵ�maxOutputSize
#endif

 //#define little_mem            1                                      // �ڴ�ǳ��ǳ��Խ���ʱ��ʹ�������
#ifdef little_mem
#define MAX_PARA_NUM                10                          // ����һ��������֧�ֵ�����������
#define DEFAULT_CORPOS_NUM             1                            // ����һ��������֧�ֵ������������
#define DEFAULT_MAX_OUTPUT_SIZE    1000             // ����Ĭ�ϵ�maxOutputSize

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

// ���û��mallocϵͳ�ģ�һ����(ʹ��ʱ���ܻ�Ҫ�������������)
// #define malloc_self  1

#ifdef __cplusplus
extern "C" {
#endif

/******************************************

������������

******************************************/
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
#ifdef __x86_64__       // ����ʲô�����Ƶ���Ʒ�ߵø�
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

ö������

******************************************/

// �����㷨ö�٣�����ʹ��Ƶ������ò�ƿ�����������Ч��:)
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

// �û��Զ����������ͣ�Ŀǰ�����Ǳ���Ҫ���±���lib����ģ�����Ҫ�޸Ķദ���룬զ�ܼ򵥵���?
enum EnumType {
    ENUM_NUMBER_U        = 0,     //  �޷�������
    ENUM_NUMBER_S,                  //  �з�������
    ENUM_NUMBER_ENUM,         //   ö�����֣�ֻ��ָ��ֵ��Χ�ڱ���
    ENUM_NUMBER_RANGE,        //   ��Χ���֣�ֻ��ָ��ֵ��Χ�ڱ���
    ENUM_STRING ,                      //  �ַ���
    ENUM_STRING_NUM,                //  �ַ�������
    ENUM_STRING_ENUM,            //  ö���ַ�����ֻ��ָ��ֵ��Χ�ڱ���
    ENUM_BLOB,                          //  buffer �ɱ䳤�ڴ��           ���ɱ䳤�ڴ���Ƿ�Ҫ?
    ENUM_BLOB_ENUM,               //   ö���ڴ�飬ֻ��ָ��ֵ��Χ�ڱ���
    ENUM_FIXBLOB,                     //    ���ɱ䳤�ڴ��
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

�ṹ������

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
    int             isHasInitValue;             // �Ƿ��г�ʼֵ
    int             isNeedFree;
    int             isAddWholeRandom;        // ���������ɶ
    int             mutatedFrequency;
    int             isAddWholeSequence;
    int             sequenceStartPos;

    char          numberValue[8];            // ר�����������������ͳ�ʼֵ�ģ�ʡ�ĵ��������ڴ�

    // s64--s32 �޸Ŀ���
    s32           inLen;                         // ���ݵĳ��ȣ���bitΪ��λ��string���뱻8����,blobҲ��
    char*        inBuf;
    int             pos;                             // Ŀǰִ�еĲ�������

    // ��������ݣ�ֻ��
    int             count;                          // һ�����ٲ�����
    int             isNeedFreeOutBuf;              // �ͷ�outBuf��־
    int             length;
    int             sign;                                // 0=�޷��ţ��з���

    // �û����Ըı�
    int             isMutatedClose[enum_MutatedMAX];        // �������ĳ��Ԫ�ص����رձ����㷨��

    // �ڲ����������
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

    int             len;        // number��λbit   other��λbyte
    int             maxLen;
    // ���ڽṹ��
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

0.һЩ�������ܽӿ�

************************************************************************/
// 0.�����Ƿ���crashʱ��ӡcrash��������Ļ��,Ĭ�ϴ�ӡ��1Ϊ��ӡ��0Ϊ����ӡ
extern  void DT_Set_If_Show_crash(int isShowAll);

// 1.�����Ƿ��ڲ���������ʱ��ӡ��������Ļ��,Ĭ�ϲ���ӡ��1Ϊ��ӡ��0Ϊ����ӡ
extern  void DT_Set_If_Show_Corpus(int isShowAll);

// 2.���ô�ӡ������ִ�б����·�������Ϊ���򲻴�ӡ����������(����������Զ�����ʱ���׺)
extern  void DT_Set_Report_Path(char* path);

// 2.1.���ô�ӡ������ִ�б�����ļ����ƣ����Ϊ���򲻴�ӡ����������(�ļ�ԭ�����������²���׷�ӵ��ļ�ĩβ)
//����һ��������ѡһ��˭������˭��Ч
extern  void DT_Set_Report_Fix_PathName(char* pathName);

// 3.�����Ƿ����������sancov�ļ�����clang����������Ч����Ҫͬʱ�򿪻�������export ASAN_OPTIONS=coverage=1
// Ĭ�ϲ������1Ϊ�����0Ϊ�����
extern  void DT_Set_Is_Dump_Coverage(int isDumpCoverage);

// 4.�����������ߴ磬��λbyte, Ĭ��ֵΪDefault_maxOutputSize
extern  void DT_Set_MaxOutputSize(int imaxOutputSize);

// 5.�õ�secodefuzz�İ汾��Ϣ
extern  const char *DT_Get_Version(void);

// 6.���õ���������ִ��һ�γ����೤ʱ�䱨bug,���Ϊ0���򲻼�ⳬʱbug
extern  void DT_Set_TimeOut_Second(int second);

// 7.������������ת�����ı��ַ�����ʽ����ӡ,����Ϊ�������ļ�·��
extern  void DT_Printf_Bin_To_Corpus(char *path);

// 8.������������ת�����ı��ַ�����ʽ��д���ļ���,����Ϊ�������ļ�·��
extern  void DT_Write_Bin_To_Corpus(char *path);

// 9.�������ַ���ת��Ϊ�������ļ�����д���ļ��,����Ϊ�����ļ�·��
extern  void DT_Write_Corpus_To_Bin(char *path);

// 10.�����Ƿ�ʼ����ֹͣ��ظ��Ƿ�����һ�㲻��Ҫ����
extern  void DT_Enable_TracePC(int isEnable);

// 11.������ӡ��ǰ������д���ļ������ú�������ֵ�жϴ����ʱ��ʹ��
extern  void DT_Show_Cur_Corpus(void);

// 12.����debug���غ�����Ĭ�Ϲرգ�1Ϊ������0Ϊ�ر�
extern  void DT_Enable_Log(int isEnable);

// 13.�õ��������ݵĳ���
extern  int DT_GET_MutatedValueLen(SElementInit *init);

// 13.0���ñ������ݱ���Ƶ�ʰٷֱȣ�100Ϊ%100,1Ϊ1%,-1Ϊ�����죬0ΪĬ��ȫ�ֱ���
// ��ʹΪ%100,��Ϊ��Щ�����㷨��ԭ��Ҳ�������������û�б���
extern  void DT_SET_MutatedFrequency(SElementInit *init, int frequency);

// 13.1�õ�ĳ�������Ƿ񱻱���
extern  int DT_GET_IsBeMutated(SElementInit *init);

// 14.�ر����б����㷨��Ĭ��ȫ��������1Ϊ������0Ϊ�ر�
extern  void DT_Enable_AllMutater(int isEnable);

// 15.�ر�ĳ�������㷨��Ĭ�Ͽ�����1Ϊ������0Ϊ�ر�
extern  void DT_Enable_OneMutater(enum EnumMutated  mutatedNum, int isEnable);

// 16.�����ַ��������Ƿ�������\0 (������ȷ�0),Ĭ��Ϊ���
extern  void DT_Set_String_Has_Terminal(int isHasTerminal);

// 16.1�����ַ��������Ƿ���Null
extern  void DT_Set_String_Has_Null(int isHasNull);

// 17.���õ�һ�����е�����飬��ӡ��pcָ�뵽��Ļ��Ĭ�Ϲر�
extern  void DT_Set_Is_Print_New_PC(int isPrintPC);

// 18.���õ�������������ʱ�䣬������õ����д���û��������ʱ�䵽�ˣ���ǰ����
extern  void DT_Set_Running_Time_Second(int second);

// 18.1Ϊ����������������Ҫ����ϣ�������������ʱ��
// 1.���д������¼��㣬2������ʱ�����¼��㣬3��ǰ���߶����¼���,0��Ĭ�϶��ǲ�������
extern  void DT_Set_Reset_On_New_Corpus(int value);

// 19.�������������̶�ѭ������֧�֣���Ϊ��һ������Ӱ�����Ч�ʣ�Ĭ�Ͽ���
extern  void DT_Enable_Support_Loop(int isEnable);

// 20.ʹ��ÿ����һ�β����������һ���ڴ�й¶�������ؼ��������ٶȣ�Ĭ�Ͽ��������һǧ��
// ��������Ѿ������ڴ�й¶������£��ٴ���������ҵ������ڴ�й¶�Ĳ�����
// ����������֮ǰ������Ч��ֻ��Чһ����������
extern  void DT_Enable_Leak_Check(int isEnable, int isDebug);

// 21.�������������ʱ�������±ߺ�������ʹ��ʱ�ı������������������������Ժ����
extern  void DT_Set_HasNewFeature(int weight);

// 22.�����Ƿ�ɾ����ƥ��������ļ�
// �������ɾ��1�������ɾ����������Լ���ִ�У�
// ������ò�ɾ��0�������������ƥ�������ֱ���˳�
// Ĭ�ϲ�ɾ��0
extern  void DT_Set_IsDeleteMismatchCopusFile(int isDelete);

// 23.�����������������Ĭ����MAX_CORPOS_NUM����С��1�����10000,����0�ظ�Ĭ��ֵ
extern  void DT_Set_MCorpusNm(int num);

// 23.1����ƽ�������������ﵽƽ��������������Ȩ������ֱ�Ӷ�����Ĭ��3000
extern  void DT_Set_SMCorpusNm(int num);

// 24.֧�ֶ��߳�ʱ�������߳��˳�ǰɾ�����߷�����ڴ棬�����ڴ�й¶
extern  void DT_Clear_ThreadMemory(void);

// 25.����һ��Ȩ��ֵӰ����ٱ������,Ĭ��1000
extern  void DT_Set_MutatorCountFromWeight(int value);

// 26,27.28����ͬ������
extern  int DT_Get_RandomSeed(void);
extern  void DT_Set_RandomSeed(int value);
extern  int DT_Get_RandomVlaue(void);

// 29.�����Ƿ�ע��asan����ص���Ĭ��ʹ�ܣ��ڱ���ûȥ�صĳ�����Ҫ�ر�
extern  void DT_Enable_AsanReport(int isEnable);

// 30.��Ŷ���������·������һ������������Ч���������
extern  void DT_SetBinCorpusDirIn(char* dir, int maxLen);
extern  void DT_SetBinCorpusDirOut(char* dir);

// 31.ʹ��forkģʽ��DTѭ�����ӽ���������,Ĭ�Ϲر�
extern  void DT_SetEnableFork(int isEnable);

// 32.�����ⲿ�ֵ��ļ�·����������������Ч
extern  void DT_SetDictionaryPath(char *path);

// 33.��������������ͬ������ʽ��Ȩ��
//1,����鸲��(10),2,��ת����(5),3,ѭ������(5),4,��֧����(1)������Ϊ0�����ַ�ʽ��������������Ӱ��
// ���ж�ʱ����ʱ��2Ҫ����Ϊ0
extern  void DT_SetPathModeWeight(int mode, int weight);

// ��ʹ�����Զ��壬ʵ���ⲿ�����㷨
// size_t DT_CustomMutator(uint8_t* data, size_t size, size_t max_size, unsigned int seed);
// size_t DT_CustomCrossOver(const uint8_t *data, size_t size1,const uint8_t *data2, size_t size2, uint8_t *out,size_t max_size, unsigned int Seed);

// 34.ע��ص�������ÿ����crashʱ���ߵ���
extern  void DT_AddCrashCallBack(void (*fun)(void));

// 35.ע��ص�������ÿ����������������ɹ��ߵ���
extern  void DT_AddTestCaseCallBack(void (*fun)(void));

// 36.���ü��OutOfMemory��MallocLimitMbĬ��2048M,һ�η����ڴ����ֵ,RssLimitMbĬ��2048M,�����ڴ�rssʹ�����ֵ
extern  void DT_SetCheckOutOfMemory(int mallocLimitMb, int rssLimitMb);
// Ĭ��û�п�������ڴ�ľ�����
extern  void DT_SetStopOutOfMemory(void);

// 37.��ȡ�ϸ����������Ƿ����гɹ�
extern  int DT_GetIsPass(void);

// 38.���û�ȡ����trace-pc��Ϣ������ֱ���˳�ѭ����Ĭ��Ϊ0�����˳�������Ϊ1���˳�
extern  void DT_SetNoTracePCStop(int stop);

/************************************************************************

1.��װ������ѭ��������

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

˵��:
���⺯���������������DT_FUZZ_START��DT_FUZZ_ENDѭ������
DT_FUZZ_START(seed, count, testCaseName, isReproduce)  
seed            Ϊ�������ʹ�õ�seedֵ��һ������Ϊ0�����߻������seed����  
count           Ϊ���Դ���  
testCaseName    Ϊ�����ļ�·��������  
isReproduce     �Ƿ�Ϊ����ģʽ,0Ϊ����ģʽ��1Ϊ����ģʽ��ֻ����ָ����������1��  

ע:
1.DT_SetGet�������ò�Ҫ��if���������ᵼ�µ�������
2.DT_SetGet������ʼֵ������ڴ�ָ�룬Ҫ��ѭ���ⲿ��ʼ����
����ѭ���´�ִ��ʱ��������ʧЧ���³�ʼֵָ��仯
3.��������Ҫ�������߼���Ӧ����������������޸ģ���ɾ�������ٽ��в���
4.��������ڴ棬���߻��Լ��ͷţ�����������Ҫ�ͷ��ڴ棬
�뽫�������ڴ�copy�����ٵ��ñ��⺯��
5.�������ɵ�������һ�ֲ�����Դ���ٴ����в������������ܹ���ȡ�����������ԣ�
������������߼�û���޸ģ���β����벻Ҫɾ������

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

// û�������ڴ�ĵط��������Σ�һ��Ҫ�����ڴ�Ļ������Լ�����fork�Ⱥ�����
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


// ���ⲿ�ṩ�Ľӿڣ���һ�����ʴ�д��Ȼ�����»������߸���������ÿ����������ĸ��д

/******************************************

2.�ӿ�һDT_SetGetϵ�нӿ�

ע:

1)
��ȡ�����ڴ�ó��Ⱦ�������
char *buf = DT_SetGetBlob(&g_Element[2], 6, 20000,"12345" );
int  buf_len=DT_GET_MutatedValueLen(&g_Element[2]);

2)
�ڴ��������ȡ����ʼ�������ڴ�ͳ�ʼ���ȱ���ƥ��
��ȷ
char *string = DT_SetGetString(&g_Element[1], 10, 10000,"zhangpeng" );
����
char *string = DT_SetGetString(&g_Element[1], 64, 10000,"zhangpeng" );

3)
��һ������Ϊ�̶�Ϊ &g_Element[]���±��0��ʼ�����ε��������������� 

4)
ͬһ���±�ֻ����һ��setget�������ã�����д������
if (a == 1)
{
    DT_SetGetS32(&g_Element[0])
}
else
{
    DT_SetGetString(&g_Element[0])
}

******************************************/
// init Ԫ�ؽṹ��ָ��,ʹ��&g_Element[n],ÿ��������n��0��ʼ����������
// initValue ��ʼֵ��ָ���ʼֵ��ָ��
// ����ֵ,ָ�����ֵ��ָ��,
// ��Ӧ������ֻ������Ӧ��С���ڴ棬����s32���ص�ָ��Ϊ4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetS32
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

// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! initValue               = 0x1234
// ! int eunmTable[]    ={2,4,6,1234,0x1234,0x897654,0x456789}
// ! eunmCount           =7
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberEnum
char *DT_SetGetNumberEnum(SElementInit *init, s32 initValue, int* eunmTable, int eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! initValue               = 0xff
// ! int eunmTable[]     ={2,4,6,1234,0x1234};
// ! eunmCount           =5
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberEnum_EX
char *DT_SetGetNumberEnum_EX(SElementInit *init, s32 initValue, int* eunmTable, int eunmCount);
// min��Сֵmax���ֵ,���������߽�ֵ
// ����
// ! initValue        =0x1234
// ! min               =0x1200
// ! max              =0x1300
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberRange
char *DT_SetGetNumberRange(SElementInit *init, s32 initValue, int min, int  max);
// min��Сֵmax���ֵ,���첻������߽�ֵ
// ����
// ! initValue         =0xff
// ! min                =0x1200
// ! max               =0x1300
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberRange_EX
char *DT_SetGetNumberRange_EX(SElementInit *init, s32 initValue, int min, int  max);
// length         ��ʼֵ���ڴ泤��,strlen(initValue)+1
// maxLength  ���ñ������󳤶ȣ����Ϊ0����ΪĬ����󳤶ȣ�Ĭ��ΪDefault_maxOutputSize
// ����
// ! length                   =6
// ! maxLength            =20
// ! initValue                ="11111"
char *DT_SetGetString(SElementInit *init, int length, int maxLength, char* initValue);
//���㷨��ʼֵΪ�ַ������֣����������ֵΪ�ַ�������
// length        ��ʼֵ���ڴ泤��,strlen(initValue)+1
// maxLength   ���ñ������󳤶ȣ����ﱻ����
// ����
// ! length                   =6
// ! maxLength            =0
// ! initValue                ="11111"
char *DT_SetGetStringNum(SElementInit *init, int length, int maxLength, char* initValue);
// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="1.1.1"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum(
    SElementInit *init, int length, int maxLength, char* initValue, char* eunmTableS[], int eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="11111"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum_EX(
    SElementInit *init, int length, int maxLength, char* initValue, char* eunmTableS[], int  eunmCount);
// length               ��ʼֵ(initValue)���ڴ泤��
// maxLength        ���ñ������󳤶ȣ����Ϊ0����ΪĬ����󳤶ȣ�Ĭ��ΪDefault_maxOutputSize
// ����
// ! length                   =15
// ! maxLength            =40
// ! initValue                ="12345678900000"
char *DT_SetGetBlob(SElementInit *init, int length, int maxLength, char* initValue);
// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! length                       =14
// ! maxLength                =20
// ! initValue                    ="6666666666666"
// ! char* eunmTableB[]  ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableL[]       ={7,7,5,14};
// ! eunmCount               =4
char *DT_SetGetBlobEnum(SElementInit *init, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! length                        =14
// ! maxLength                 =20
// ! initValue                     ="6666666666677"
// ! char* eunmTableB[]   ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableB[]        ={7,7,5,14};
// ! eunmCount                =4
char *DT_SetGetBlobEnum_EX(SElementInit *init, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// length��ʼֵ�ĳ��ȣ�����������ֵ���������������ͬ
// maxLengt�������������
char *DT_SetGetFixBlob(SElementInit *init, int length, int maxLength, char*  initValue);
// ����
// ! u8 temp_ipv4[4]    ={192,168,0,1};
// ! initValue                =(char *)temp_ipv4)
char *DT_SetGetIpv4(SElementInit *init, char* initValue);
// ����
// ! u8 temp_ipv6[16]  ={0x20,0x02,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x01};
// ! initValue               =(char*)temp_ipv6
char *DT_SetGetIpv6(SElementInit *init, char* initValue);
// ����
// ! u8 temp_mac[6]     ={0x28,0x6e,0xd4,0x89,0x26,0xa8};
// ! initValue                =(char*)temp_mac
char *DT_SetGetMac(SElementInit *init, char* initValue);

// �Զ������;���
char *DT_SetGetSelf(SElementInit *init, int length, int maxLength, char* initValue, int arg);

/******************************************/
// רΪ2.0����

// �ں�̬��֧��
#ifndef __KERNEL__
#define secodepits
#endif

// ʹ�ÿ�Դxml�⣬�������
//#define USE_libxm2lib

#ifdef secodepits

// ���⿪Դ�汾������������д���ɾ��
//#define Open_source 

enum Enum_Pits_Type {
    ENUM_BUF_CUT        = 0,        // �м��ȡ
    ENUM_BLOCK_DELETE,           // ����ɾ��          
    ENUM_BLOCK_COPY,               // ���鸴��
    ENUM_BLOCK_CHANGE,          // �������齻��  
    ENUM_ELEMENT_DELETE,       // Ԫ��ɾ��          
    ENUM_ELEMENT_COPY,          // Ԫ�ظ���
    ENUM_ELEMENT_CHANGE,     // ����Ԫ�ؽ���      
    ENUM_RELATION_SIZE,         // relation size��ر���
};

/*
����DT_Pits_Set_DebugOnOff
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

// �������buf����󳤶�
#define OUT_BUF_MAX_LENGTH  10000000
extern int g_tempElementCount;

// 1.�ر�����pits�����㷨��Ĭ��ȫ���رգ�1Ϊ������0Ϊ�ر�
extern  void DT_Pits_Enable_AllMutater(int isEnable);

// 2.�ر�ĳ��pits�����㷨��Ĭ�Ϲرգ�1Ϊ������0Ϊ�ر�
extern  void DT_Pits_Enable_OneMutater(enum Enum_Pits_Type  mutatedNum, int isEnable);

// 3.���ñ���Ƶ�ʣ�Ĭ��100,����100��֮1��ԽСƵ��Խ��
// ������Ҫ��Բ�����Ԫ�صĶ��ٽ��е�����
extern  void DT_Pits_Set_Mutator_Frequency(int frequency);

// 4.�����㷨ѡ��Ƶ�ʣ�Ĭ��20������10��֮1��ԽСƵ��Խ��,����С��20
extern  void DT_Pits_Set_Algorithm_Frequency(int frequency);

#ifndef Open_source
// 5.����ģ��
extern int DT_Pits_ParsePits(char* docname);

// ִ��״̬ģ��
extern void DT_Pits_DoState(void);
#endif

// 6.��������ģ��
extern int DT_Pits_ParseDataModel(char* docname, char* dataModelName, int isfirst);

// ִ��action
extern void DT_Pits_DoAction(int id);

// 7.�ͷŽ���ģ��ʹ�õ����ڴ�
extern void DT_Pits_ParseFree(void);

// �ͷ�����һ�β��������õ����м��ڴ�
extern void DT_Pits_OneRunFree(void);

// 8.���������ļ�·��
extern void DT_Pits_SetBinFile(char* binName);

// 9.���������ļ��ڴ�ָ��ͳ���
extern void DT_Pits_SetBinBuf(char* binBuf, int binBufLength);

// 10.���õ���Ԫ�ر������󳤶�,Ĭ��4000
extern void DT_Pits_Set_MaxOutput(int value);

// 11.����debug����
extern void DT_Pits_Set_DebugOnOff(char* debug);

// 12.�����Ƿ���д�С�˽�����Ĭ�ϲ�����
extern void DT_Pits_Set_BigOrLittleSwap(int swap);

// 13.����config,���ȼ�����config�ļ�,ֻ��һ������������Ч��DT_Pits_ParseFree���
extern void DT_Pits_Set_Config( char* configKey,char* configValue);

// �õ����������ݺͳ���
extern void DT_Pits_GetMutatorBuf(char** buf, int *len);

/************************************************************************

V3ϵ�нӿڣ��Ժ�����ȡ��ͬ���ͽӿڣ�����g_Element,���ؽṹ������
ʹh�ļ���so�����

************************************************************************/

// 13.�õ��������ݵĳ���
extern  int DT_GET_MutatedValueLenV3(int id);

// 13.0���ñ������ݱ���Ƶ�ʰٷֱȣ�100Ϊ%100,1Ϊ1%,-1Ϊ�����죬0ΪĬ��ȫ�ֱ���
// ��ʹΪ%100,��Ϊ��Щ�����㷨��ԭ��Ҳ�������������û�б���
extern  void DT_SET_MutatedFrequencyV3(int id, int frequency);

// 13.1�õ�ĳ�������Ƿ񱻱���
extern  int DT_GET_IsBeMutatedV3(int id);

// 15.�ر�ĳ�������㷨��Ĭ�Ͽ�����1Ϊ������0Ϊ�ر�
extern  void DT_Enable_OneMutaterV3(int  mutatedId, int isEnable);

// ���ⲿ�ṩ�Ľӿڣ���һ�����ʴ�д��Ȼ�����»������߸���������ÿ����������ĸ��д

/******************************************

2.�ӿ�һDT_SetGetϵ�нӿ�

1)
��ȡ�����ڴ�ó��Ⱦ�������
char *buf = DT_SetGetBlobV3(2, 6, 20000,"12345" );
int  buf_len=DT_GET_MutatedValueLenV3(2);

2)
�ڴ��������ȡ����ʼ�������ڴ�ͳ�ʼ���ȱ���ƥ��
��ȷ
char *string = DT_SetGetStringV3(1, 10, 10000,"zhangpeng" );
����
char *string = DT_SetGetStringV3(1, 64, 10000,"zhangpeng" );

3)
idʹ�ô�0��ʼ�����ε��������������� 

4)
ͬһ��idֻ����һ��setget�������ã�����д������
if (a == 1)
{
    DT_SetGetS32V3(0)
}
else
{
    DT_SetGetStringV3(0)
}


******************************************/
// ÿ��������id��0��ʼ����������
// initValue ��ʼֵ��ָ���ʼֵ��ָ��
// ����ֵ,ָ�����ֵ��ָ��
// ��Ӧ������ֻ������Ӧ��С���ڴ棬����s32���ص�ָ��Ϊ4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetS32V3
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

// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! initValue               = 0x1234
// ! int eunmTable[]    ={2,4,6,1234,0x1234,0x897654,0x456789}
// ! eunmCount           =7
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberEnumV3
char *DT_SetGetNumberEnumV3(int id, s32 initValue, int* eunmTable, int eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! initValue               = 0xff
// ! int eunmTable[]     ={2,4,6,1234,0x1234};
// ! eunmCount           =5
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberEnum_EXV3
char *DT_SetGetNumberEnum_EXV3(int id, s32 initValue, int* eunmTable, int eunmCount);
// min��Сֵmax���ֵ,���������߽�ֵ
// ����
// ! initValue        =0x1234
// ! min               =0x1200
// ! max              =0x1300
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberRangeV3
char *DT_SetGetNumberRangeV3(int id, s32 initValue, int min, int  max);
// min��Сֵmax���ֵ,���첻������߽�ֵ
// ����
// ! initValue         =0xff
// ! min                =0x1200
// ! max               =0x1300
// ����ֵ,ָ�����ֵ��ָ��,4���ֽ�
// ��ȡ����ֵ����:s32 number= *(s32 *)DT_SetGetNumberRange_EXV3
char *DT_SetGetNumberRange_EXV3(int id, s32 initValue, int min, int  max);
// length         ��ʼֵ���ڴ泤��,strlen(initValue)+1
// maxLength  ���ñ������󳤶ȣ����Ϊ0����ΪĬ����󳤶ȣ�Ĭ��ΪDefault_maxOutputSize
// ����
// ! length                   =6
// ! maxLength            =20
// ! initValue                ="11111"
char *DT_SetGetStringV3(int id, int length, int maxLength, char* initValue);
//���㷨��ʼֵΪ�ַ������֣����������ֵΪ�ַ�������
// length        ��ʼֵ���ڴ泤��,strlen(initValue)+1
// maxLength   ���ñ������󳤶ȣ����ﱻ����
// ����
// ! length                   =6
// ! maxLength            =0
// ! initValue                ="11111"
char *DT_SetGetStringNumV3(int id, int length, int maxLength, char* initValue);
// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="1.1.1"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnumV3(
    int id, int length, int maxLength, char* initValue, char* eunmTableS[], int eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! length                          =6
// ! maxLength                   =20
// ! initValue                       ="11111"
// ! char* eunmTableS[]      ={"123","abc","zhangpeng","1.1.1","wanghao" ,"12345"};
// ! eunmCount                   =6
char *DT_SetGetStringEnum_EXV3(
    int id, int length, int maxLength, char* initValue, char* eunmTableS[], int  eunmCount);
// length               ��ʼֵ(initValue)���ڴ泤��
// maxLength        ���ñ������󳤶ȣ����Ϊ0����ΪĬ����󳤶ȣ�Ĭ��ΪDefault_maxOutputSize
// ����
// ! length                   =15
// ! maxLength            =40
// ! initValue                ="12345678900000"
char *DT_SetGetBlobV3(int id, int length, int maxLength, char* initValue);
// ������ÿ��ֵ����ö�ٱ����������������
// ����
// ! length                       =14
// ! maxLength                =20
// ! initValue                    ="6666666666666"
// ! char* eunmTableB[]  ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableL[]       ={7,7,5,14};
// ! eunmCount               =4
char *DT_SetGetBlobEnumV3(int id, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// ������ÿ��ֵ����ö�ٱ�֮��
// ����
// ! length                        =14
// ! maxLength                 =20
// ! initValue                     ="6666666666677"
// ! char* eunmTableB[]   ={"123111","abcaaa","\x00\x01\x02\x00\xff","6666666666666"};
// ! int eunmTableB[]        ={7,7,5,14};
// ! eunmCount                =4
char *DT_SetGetBlobEnum_EXV3(int id, int length, int maxLength, char* initValue, 
    char* eunmTableB[], int eunmTableL[], int  eunmCount);
// length��ʼֵ�ĳ��ȣ�����������ֵ���������������ͬ
// maxLengt�������������
char *DT_SetGetFixBlobV3(int id, int length, int maxLength, char*  initValue);
// ����
// ! u8 temp_ipv4[4]    ={192,168,0,1};
// ! initValue                =(char *)temp_ipv4)
char *DT_SetGetIpv4V3(int id, char* initValue);
// ����
// ! u8 temp_ipv6[16]  ={0x20,0x02,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x01};
// ! initValue               =(char*)temp_ipv6
char *DT_SetGetIpv6V3(int id, char* initValue);
// ����
// ! u8 temp_mac[6]     ={0x28,0x6e,0xd4,0x89,0x26,0xa8};
// ! initValue                =(char*)temp_mac
char *DT_SetGetMacV3(int id, char* initValue);

// �Զ������;���
char *DT_SetGetSelfV3(int id, int length, int maxLength, char* initValue, int arg);
#endif
#ifdef __cplusplus
}
#endif
#endif
