#ifndef _FILETEST_H_
#define _FILETEST_H_
#include <iostream>
#include "xbsa.h"

class FileTest
{
    FILE* m_readFp; // 用于读本地文件，作为备份测试文件
public:
    FileTest();
    ~FileTest();
    bool OpenFile(const std::string &fileName);
    bool Read(BSA_DataBlock32 *dataBlockPtr);
    int Write(BSA_DataBlock32 *dataBlockPtr);
};

#endif