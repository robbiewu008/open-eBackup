/*
版权所有 (c) 华为技术有限公司 2012-2018

用户在这个文件里编辑字典
*/

#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// blob
char const *g_customBlobTable[] = {
    "CustomBlob",
    "\x00\x01\x02\x00\xff",
    "\x00\x01\x02\x00\xff\x00\x01\x02\x00\xff\x00\x01\x02\x00\xff",
    "\xff\x33\x00\x01\x02\x00\xff",
 };
int const g_customBlobTableLen[] = {
    11,
    5,
    15,
    7,
 };

int g_customBlobTableCount = sizeof(g_customBlobTableLen) / sizeof(int);

// number
u64 const g_customNumberTable[] = {
    0x8,
    0x88,
};

int g_customNumberTableCount = sizeof(g_customNumberTable) / sizeof(g_customNumberTable[0]);

// string
char const * g_customStringTable[] = {
    "CustomString",
};

int g_customStringTableCount = sizeof(g_customStringTable) / sizeof(char *);

#ifdef __cplusplus
}
#endif