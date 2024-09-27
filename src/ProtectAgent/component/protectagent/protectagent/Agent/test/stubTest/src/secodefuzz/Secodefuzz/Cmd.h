#include <gtest/gtest.h>
#include "secodefuzz/Secodefuzz/secodeFuzz.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int g_count;             //运行次数,默认三千万次
extern int g_time;              //运行时间，默认三个小时
extern int g_isreproduce;       //是否是复现模式
extern int g_isLeakCheck;       //是否检测内存泄漏
extern char* g_reportPath;      //报告路径
extern char* g_corpusPath;      //样本路径
extern char* g_crashCorpusName; // crash样本名字，在复现模式有效
extern char  g_testCaseName[];

extern int cmd(int argc,char *argv[]);

#define DT_FUZZ_START1(testCaseName)  \
    char* testcaseName = testCaseName;\
    if (g_crashCorpusName)\
        testcaseName = g_crashCorpusName;\
    g_testCaseName[0] = 0;\
    sprintf(g_testCaseName, "%s%s", g_corpusPath, testcaseName);\
    DT_FUZZ_START(0, g_count, g_testCaseName, g_isreproduce)


#ifdef __cplusplus
}
#endif