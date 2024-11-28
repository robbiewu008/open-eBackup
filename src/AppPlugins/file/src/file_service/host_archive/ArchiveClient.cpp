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
#include "ArchiveClient.h"
#include "log/Log.h"
#include "PluginTypes.h"
#include "PluginUtilities.h"
#include "securec.h"

namespace FilePlugin {
namespace {
    const int SUCCESS = 0;
    const int FAILED = -1;
    auto const MODULE = "ArchiveClient";
    const int PREPARE_ARCHIVE_CLIENT_INTERVAL = 10;

    const uint32_t ARCHIVE_REQ_BUFFER_SIZE = 4 * 1024 * 1024;
    const int SKD_PREPARE_CODE_NE1 = -1;
    const int SKD_PREPARE_CODE_2 = 2;
}

void ArchiveClient::SetFsId(const std::string& fsId)
{
    m_fsId = fsId;
}

int ArchiveClient::InitClient(const std::vector<std::string>& ipList, int port, bool enableSSL)
{
    INFOLOG("Enter InitClient");
    int ret = m_clientHandler->Init(m_copyId, m_jobId, "");
    if (ret != SUCCESS) {
        ERRLOG("Archive client init failed, ret: %d", ret);
        return FAILED;
    }
    INFOLOG("Archive client connect");
    std::string listString;
    for (std::size_t i = 0; i < ipList.size(); i++) {
        listString.append(ipList[i]);
        if (i == ipList.size() -1) {
            break;
        }
        listString.append(",");
    }
    INFOLOG("Archive client connnect ip [%s], port [%d], ssl [%d]",
        listString.c_str(), port, (enableSSL ? 1 : 0));
    ret = m_clientHandler->Connect(listString, port, enableSSL);
    if (ret != SUCCESS) {
        ERRLOG("Archive client connect failed, ret: %d", ret);
        return FAILED;
    }
    m_isInit = true;
    INFOLOG("Archive client connect success");
    return SUCCESS;
}

int ArchiveClient::GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp)
{
    INFOLOG("Enter GetFileData");
    ArchiveStreamGetFileReq archiveReq;
    archiveReq.taskID = m_jobId;
    archiveReq.archiveBackupId = m_copyId;
    archiveReq.fsID = m_fsId; // archive fs id
    archiveReq.filePath = req.m_fileName; // 文件归档的路径
    archiveReq.fileOffset = req.m_offset;
    archiveReq.readSize = ARCHIVESTREAM_READ_SIZE_4M ; // 0 -- 4 * 1024 * 1024
    archiveReq.maxResponseSize = ARCHIVESTREAM_RESPONSE_SIZE_512K; // 6 -- 512 * 1024
    uint64_t blockSize = req.m_size;
#ifdef WIN32
    // win 场景 去掉路径里的反斜线， 不然归档会报错
    for (char& c : archiveReq.filePath) {
        if (c == '\\') {
            c = '/';
        }
    }
#endif

    DBGLOG("Archive client request taskId[%s], copyId[%s], fsId[%s], filePath[%s], offset[%ld], blockSize[%llu]",
        archiveReq.taskID.c_str(), archiveReq.archiveBackupId.c_str(), archiveReq.fsID.c_str(),
        archiveReq.filePath.c_str(), archiveReq.fileOffset, blockSize);
    ArchiveStreamGetFileRsq archiveRsp;
    int ret = m_clientHandler->GetFileData(archiveReq, archiveRsp);
    if (ret != SUCCESS) {
        ERRLOG("Archive client get file data failed, ret: %d", ret);
        return ret;
    }

    // rsp path is empty, print req path here
    rsp.m_offset = archiveRsp.offset;
    rsp.m_size = archiveRsp.fileSize;
    rsp.m_readEnd = archiveRsp.readEnd;

    DBGLOG("Archive client response filePath[%s], offset[%lld], size[%d], readEnd[%d]",
        archiveReq.filePath.c_str(), archiveRsp.offset, archiveRsp.fileSize, archiveRsp.readEnd);

    // archiveRsp.data size is 4M, but blockSize is flexible
    ret = memcpy_s(req.m_buffer, blockSize, archiveRsp.data, blockSize);
    if (ret != 0) {
        free(archiveRsp.data);
        ERRLOG("memcpy failed for %s, ret: %d", req.m_fileName.c_str(), ret);
        return FAILED;
    }
    free(archiveRsp.data);
    DBGLOG("Archive client response success.");
    return SUCCESS;
}

int ArchiveClient::Disconnect()
{
    INFOLOG("Enter archive client disconnect");
    if (!m_isInit) {
        INFOLOG("Archive client not init connect");
        return SUCCESS;
    }
    int ret = m_clientHandler->Disconnect();
    if (ret != SUCCESS) {
        ERRLOG("archive client disconnect fail, ret: %d", ret);
        return FAILED;
    }
    INFOLOG("CloseArchiveClient success");
    return SUCCESS;
}

int ArchiveClient::EndRecover()
{
    INFOLOG("Enter archive client end recover");

    int ret = m_clientHandler->EndRecover();
    if (ret != SUCCESS) {
        INFOLOG("End whole archive recovery failed, ret: %d", ret);
        return FAILED;
    }
    INFOLOG("End whole archive recovery success");
    return SUCCESS;
}
}

