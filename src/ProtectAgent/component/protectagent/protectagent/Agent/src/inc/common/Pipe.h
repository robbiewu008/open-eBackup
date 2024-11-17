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
#ifndef PIPE_FILE_H
#define PIPE_FILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <atomic>
#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include "common/Defines.h"

class AGENT_API CMpPipe {
public:
    CMpPipe();
    ~CMpPipe() {}
    mp_int32 UnlinkPipe(const mp_string& strPipeName);
    mp_int32 ReadInput(const mp_string& strUniqueID, mp_string& strInput);
    mp_int32 WriteInput(const mp_string& strUniqueID, const mp_string& strInput);
    mp_int32 ReadPipe(const mp_string& strFileName, std::vector<mp_string>& vecOutput);
    mp_int32 WritePipe(const mp_string& strFileName, std::vector<mp_string>& vecInput);
    mp_void ClosePipe();
    mp_void SetUniqueID(const mp_string& strUniqueID);
    mp_void SetStrInput(const mp_string& strInput);
    mp_void SetVecInput(std::vector<mp_string>& vecInput);
    const mp_string& GetUniqueID();
    const mp_string& GetStrInput();
    mp_void GetVecInput(std::vector<mp_string>& vecInput);
    mp_int32 OpenPipe(const mp_string& strFileName, mp_bool bReader);
    mp_void SetOperateFlage(bool bOperateEnd)
    {
        std::atomic_init(&m_bOperateEnd, bOperateEnd);
    }
    bool GetOperateFlage()
    {
        return m_bOperateEnd.load();
    }

#ifndef WIN32
    mp_int32 CreatePipe(const mp_string& strFileName);
#endif

private:
    std::atomic<bool> m_bOperateEnd;    // 读写管道完成标识
    mp_string m_strUniqueID;
    mp_string m_strInput;
    std::vector<mp_string> m_vecInput;
#ifdef WIN32
    HANDLE  m_hPipe;
#else
    int m_fd;
#endif // WIN32
    mp_int32 WritePipeInner(const mp_string& strFileName, const mp_string& strInput, int& nByteWrite);
};

#endif  // __AGENT_FILE_H__
