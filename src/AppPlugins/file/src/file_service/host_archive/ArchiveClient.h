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
#ifndef ARCHIVE_COMMON_H
#define ARCHIVE_COMMON_H

#include <string>
#include <memory>
#include "message/archivestream/ArchiveStreamService.h"
#include "ArchiveClientBase.h"

namespace FilePlugin {
enum class ArchiveClientErrorCode {
    NONE = -1,
    SUCCESS = 0,
    FAILED = 1,
    ABORT = 2,
    NASSHARE_ERROR = 3,
    EMPTY_COPY = 4,
    PARTIAL_SUCCESS = 5
};

// control client connect to archive server
class ArchiveClientControl {
public:
    virtual int InitClient(const std::vector<std::string>& ipList, int port, bool enableSSL) = 0;
    virtual int Disconnect() = 0;
    virtual int EndRecover() = 0;
};

class ArchiveClient : public ArchiveClientBase, public ArchiveClientControl {
public:
    ArchiveClient(const std::string& jobId, const std::string& copyId) : m_jobId(jobId), m_copyId(copyId) {}
    ~ArchiveClient() {}

    void SetFsId(const std::string& fsId);
    // data
    int GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp) override;
    // control
    int InitClient(const std::vector<std::string>& ipList, int port, bool enableSSL) override;
    int Disconnect() override;
    int EndRecover() override;

private:
    const std::string m_jobId;
    const std::string m_copyId;
    std::string m_fsId;
    std::shared_ptr<ArchiveStreamService> m_clientHandler = std::make_shared<ArchiveStreamService>();
    ArchiveClientErrorCode m_errCode {ArchiveClientErrorCode::NONE};
    bool m_isInit {false};
};
}
#endif // ARCHIVE_COMMON_H