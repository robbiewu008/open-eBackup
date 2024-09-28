/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "FileTest.h"
#include "CommonDefine.h"

FileTest::FileTest()
{
    m_readFp = nullptr;
}

 FileTest::~FileTest()
 {
     if (m_readFp) {
         fclose(m_readFp);
     }
 }
bool FileTest::OpenFile(const std::string &fileName)
{
    m_readFp = fopen(fileName.c_str(), "rb+");
    if (m_readFp == nullptr) {
        Log("Open file '%s' failed.", fileName.c_str());
        return false;
    }
    return true;
}
bool FileTest::Read(BSA_DataBlock32 *dataBlockPtr)
{
    char *p = (char*)dataBlockPtr->bufferPtr + dataBlockPtr->headerBytes;
    size_t rc = fread((void*)p, sizeof(char), dataBlockPtr->bufferLen, m_readFp);

    dataBlockPtr->numBytes = rc;
    return true;
}

int FileTest::Write(BSA_DataBlock32 *dataBlockPtr)
{
    // char *p = (char*)dataBlockPtr->bufferPtr + dataBlockPtr->headerBytes;
    // size_t rc = fwrite((void*)p, sizeof(char), dataBlockPtr->numBytes, m_fp);   
    // if (rc != dataBlockPtr->numBytes) {
    //     COMMLOG(OS_LOG_ERROR,"[bsaHandle=%ld]The number of bytes that fail to be written is:%d",
    //             bsaHandle, dataBlockPtr->numBytes - rc);
    //     if (m_fp != NULL) {
    //         fclose(m_fp);
    //     }
    //     return MP_FAILED;
    // }
}
