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
#include "message/archivestream/ArchiveStreamService.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <random>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "common/DB.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/Uuid.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "securecom/RootCaller.h"
#include "securecom/Ip.h"
#include "message/tcp/CDppMessage.h"
#include "message/archivestream/ArchiveStreamClientHandler.h"
#include "servicecenter/services/device/PrepareFileSystem.h"

#ifdef LINUX
    #define AGENT_EXPORT __attribute__((visibility("default")))
#else
    #ifdef AGENT_EXPORT
        #undef AGENT_EXPORT
    #endif // AGENT_EXPORT
    #define AGENT_EXPORT
#endif

namespace {
const mp_string RECOVERYMESSAGE_OFFSET = "offset";
const mp_string RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID = "archiveBackupId";
const mp_string RECOVERYMESSAGE_FILE_PATH = "filePath";
const mp_string RECOVERYMESSAGE_READ_SIZE = "readSize";
const mp_string RECOVERYMESSAGE_MAX_RESPONSE_SIZE = "maxResponseSize";
const mp_string RECOVERYMESSAGE_ARCHIVE_BACKUP_ID = "backupID";
const mp_string RECOVERYMESSAGE_DIRS = "dirs";
const mp_string RECOVERYMESSAGE_STATE = "state";
const mp_string RECOVERYMESSAGE_DIR_COUNT = "dirCount";
const mp_string RECOVERYMESSAGE_FILE_COUNT = "dirCount";
const mp_string RECOVERYMESSAGE_BACKUP_FILE_SIZE = "backupFileSize";
const mp_string RECOVERYMESSAGE_READ_COUNT_LIMIT = "readCountLimit";
const mp_string RECOVERYMESSAGE_OBJECT_LIST = "objectList";
const mp_string RECOVERYMESSAGE_OBJECT_NAME = "objectName";
const mp_string RECOVERYMESSAGE_OBJECT_FSID = "fsid";
const mp_string RECOVERYMESSAGE_OBJECT_IS_DIR = "isDir";
const mp_string RECOVERYMESSAGE_STATUS = "status";
const mp_string RECOVERYMESSAGE_SPLIT_FILE = "splitFile";
const mp_string RECOVERYMESSAGE_OBJECT_NUM = "objectNum";
const mp_string RECOVERYMESSAGE_METADATA = "metadata";
const mp_string RECOVERYMESSAGE_CHECKPOINT = "checkpoint";
const mp_string HOST_ENV_DEPLOYTYPE = "DEPLOY_TYPE";

const std::string DEFAULT_MOUNT_PATH = "/mnt/databackup/archivestream/";
const mp_string RECOVERYMESSAGE_FS_SHARE_NAME = "fsShareName";
const mp_string RECOVERYMESSAGE_META_FILE_DIR = "metaFileDir";
const mp_string RECOVERYMESSAGE_PARENT_DIR = "parentDir";
const mp_string RECOVERYMESSAGE_CACHE_FILE_NAME = "cacheFileSystemSharePath";
const mp_string CACHE_REPO_NAME = "cacheRepoName";
const mp_string RECOVERYMESSAGE_LOGIC_IPV4_LIST = "logicIpv4List";
const mp_string RECOVERYMESSAGE_LOGIC_IPV6_LIST = "logicIpv6List";
const mp_string RECOVERYMESSAGE_AGENT_IP_LIST = "agentIpList";

const mp_int32 DPP_MSG_TIMEOUT = 300;
const mp_int32 DEFULAT_SEND_TIMES = 3;  // send retry times, default 3
const mp_int32 ARCHIVE_STREAM_FORMAT_STRING = 512;
const mp_int32 ARCHIVE_STREAM_FORMAT_LONG_STRING = 10240;
const mp_int32 ARCHIVE_STREAM_FORMAT_UUID = 36;
const mp_int32 ARCHIVE_STREAM_FORMAT_UINT32 = 4294967295;
const mp_int32 ARCHIVE_STREAM_FORMAT_UINT64 = 512;
const mp_int32 ARCHIVE_STREAM_FORMAT_PORT = 65535;
const mp_int32 CHECK_HOST_RETRY_TIMES = 3;
const mp_int32 CHECK_HOST_TIMEOUT = 1000; // 1s

mp_void DestoryDppMessage(CDppMessage* dppMsg)
{
    if (dppMsg != nullptr) {
        delete dppMsg;
        dppMsg = nullptr;
    }
}
mp_int32 ProcesserResponseMSG(CDppMessage *rspMsg, Json::Value &respParam)
{
    if (rspMsg == NULL) {
        ERRLOG("response message is NULL.");
        return MP_FAILED;
    }

    Json::Value respBady;
    rspMsg->GetManageBody(respBady);

    respParam = respBady[MANAGECMD_KEY_BODY];
    mp_int32 errorCode = respParam[MANAGECMD_KEY_ERRORCODE].asUInt();
    mp_string errorDeteal = respParam[MANAGECMD_KEY_ERRORDETAIL].isString() ?
        respParam[MANAGECMD_KEY_ERRORDETAIL].asString() : "";
    mp_string taskId = respParam[MANAGECMD_KEY_TASKID].isString() ?
        respParam[MANAGECMD_KEY_TASKID].asString() : "";
    DestoryDppMessage(rspMsg);
    if (errorCode != MP_SUCCESS) {
        ERRLOG("Get msg body failed, taskid=%s, errorcode:%d, errordetail=%s.",
            taskId.c_str(),
            errorCode,
            errorDeteal.c_str());
        return errorCode;
    }

    INFOLOG("Get msg body success, taskid=%s.", taskId.c_str());
    return MP_SUCCESS;
}
}  // namespace

AGENT_EXPORT ArchiveStreamService::ArchiveStreamService()
{
    m_handler = new ArchiveStreamClientHandler();
}

AGENT_EXPORT ArchiveStreamService::~ArchiveStreamService()
{
    m_backupId.clear();
    m_taskID.clear();
    m_dirList.clear();
    INFOLOG("Destruct finish.");
}

AGENT_EXPORT \
mp_int32 ArchiveStreamService::Init(const mp_string &backupId, const mp_string &taskID, const mp_string &dirList)
{
    if (!m_backupId.empty()) {
        DBGLOG("ArchiveStreamService  Inited.");
        return MP_SUCCESS;
    }

    if (backupId.empty() || taskID.empty()) {
        ERRLOG("response message is NULL.");
        return MP_FAILED;
    }

    m_backupId = backupId;
    m_taskID = taskID;
    CMpString::StrSplit(m_dirList, dirList, CHAR_COMMA);
    DBGLOG("backupId:[%s] taskID:[%s] dirList:[%s].", backupId.c_str(), taskID.c_str(), dirList.c_str());

    return m_handler->Init();
}

AGENT_EXPORT mp_int32 ArchiveStreamService::Connect(const mp_string &busiIp, mp_int32 busiPort, bool openSsl)
{
    if (busiPort < 0 || busiPort > ARCHIVE_STREAM_FORMAT_PORT) {
        ERRLOG("Input port[%d] is invalid.", busiPort);
    }
    if (m_handler->GetConnectState() == MP_SUCCESS) {
        return MP_SUCCESS;
    }
    std::vector<mp_string> archieveIPList;
    if (!SplitIpList(busiIp, busiPort, archieveIPList)) {
        return MP_FAILED;
    };
    mp_bool connectTooMuch = MP_FALSE;
    mp_int32 iRet = MP_FAILED;
    for (auto perArchieve : archieveIPList) {
        iRet = BuildConnect(perArchieve, busiPort, true);
        if (iRet == MP_ARCHIVE_TOO_MUCH_CONNECTION) {
            connectTooMuch = MP_TRUE;
        } else if (iRet == MP_SUCCESS) {
            Json::Value reqInfo;
            reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
            reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
            std::vector<mp_string> ipv4List;
            std::vector<mp_string> ipv6List;
            iRet = CIP::GetHostIPList(ipv4List, ipv6List);
            int ipv4Size = ipv4List.size();
            for (int i = 0; i < ipv4Size; ++i) {
                reqInfo[RECOVERYMESSAGE_AGENT_IP_LIST].append(ipv4List.at(i));
            }
            int ipv6Size = ipv6List.size();
            for (int i = 0; i < ipv6Size; ++i) {
                reqInfo[RECOVERYMESSAGE_AGENT_IP_LIST].append(ipv6List.at(i));
            }

            Json::Value rspInfo;
            DBGLOG("Copy Id is %s,task[%s], ip[%s].", m_backupId.c_str(), m_taskID.c_str(), perArchieve.c_str());
            iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_SSH_CHECK);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Check ssh info failed, ret[%d], task[%s] ip[%s].", iRet, m_taskID.c_str(), perArchieve.c_str());
                m_handler->Disconnect();
                continue;
            }

            m_handler->Disconnect();
            m_busiIp = busiIp;
            m_busiPort = busiPort;
            DBGLOG("First Connect task[%s] ip[%s] Success.", m_taskID.c_str(), perArchieve.c_str());
            return m_handler->Connect(perArchieve, busiPort + 1, false);
        }
    }
    ERRLOG("Connect all archieves(%s) failed.", busiIp.c_str());
    return connectTooMuch == MP_TRUE ? MP_ARCHIVE_TOO_MUCH_CONNECTION : iRet;
}

mp_int32 ArchiveStreamService::BuildConnect(const mp_string &ip, mp_int32 port, bool openSsl)
{
    mp_int32 iRet = MP_FAILED;
    for (int i = 0; i < DEFULAT_SEND_TIMES; i++) {
        mp_string tIp = ip;
        mp_int32 iRet = m_handler->Connect(tIp, port, openSsl);
        if (iRet == MP_SUCCESS) {
            return MP_SUCCESS;
        } else if (iRet == MP_ARCHIVE_TOO_MUCH_CONNECTION) {
            DBGLOG("Connection too much.");
            return iRet;
        }
        m_handler->Disconnect();
        DBGLOG("Rebuild connect.");
    }
    return MP_FAILED;
}

mp_bool ArchiveStreamService::SplitIpList(const mp_string &busiIp, const mp_int32 &busiPort,
    std::vector<mp_string> &IPList)
{
    std::vector<mp_string> vecIP;
    CMpString::StrSplit(vecIP, busiIp, ',');
    if (vecIP.empty()) {
        ERRLOG("Split ip failed, PM ip list is empty(%s).", busiIp.c_str());
        return false;
    }
    // 先检查连通性，过滤不通的IP
    for (const auto &ip : vecIP) {
        for (mp_int32 i = 0; i < CHECK_HOST_RETRY_TIMES; ++i) {
            if (CSocket::CheckHostLinkStatus("", ip, static_cast<mp_uint16>(busiPort), CHECK_HOST_TIMEOUT)
                == MP_SUCCESS) {
                IPList.push_back(ip);
                break;
            }
        }
    }
    // 随机打乱server列表，负荷分担
    std::random_device rd;
    std::shuffle(std::begin(IPList), std::end(IPList), rd);
    return true;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::Disconnect()
{
    return m_handler->Disconnect();
}

mp_int32 ArchiveStreamService::SendDppMsgWithRespones(const mp_string &taskId, const Json::Value &strReqMsg,
    ArchiveStreamGetFileRsq &strRspMsg, mp_uint32 reqCmd, mp_uint32 reciveCount)
{
    CDppMessage *reqMsg = NULL;
    mp_int32 iRet = MP_SUCCESS;

    mp_uint64 seq = m_handler->GetSeqNo();
    NEW_CATCH_RETURN_FAILED(reqMsg, CDppMessage);
    reqMsg->InitMsgHead(reqCmd, 0, seq);
    reqMsg->SetMsgSrc(ROLE_HOST_AGENT);
    reqMsg->SetMsgTgt(ROLE_DME_ARCHIVE);

    Json::Value protectedMsg;
    protectedMsg[MANAGECMD_KEY_CMDNO] = reqCmd;
    protectedMsg[MANAGECMD_KEY_BODY] = strReqMsg;
    reqMsg->SetMsgBody(protectedMsg);
    m_handler->SendDPMessage(taskId, reqMsg);

    for (mp_uint32 i = 0; i < reciveCount; i++) {
        CDppMessage *rspMsg = NULL;
        std::ostringstream strBuf;
        strBuf << seq;

        m_handler->GetResponseMessage(strBuf.str(), rspMsg, seq, DPP_MSG_TIMEOUT);
        if (rspMsg == NULL) {
            ERRLOG("GetResponseMessage failed, taskid=%s, cmd=0x%x, seq=%llu.", taskId.c_str(), reqCmd, seq);
            strRspMsg.fileSize = 0;
            return MP_FAILED;
        }
        if (rspMsg->GetCmd() == CMD_ARCHIVE_GET_FILE_DATA_JSON_ACK) {
            Json::Value respParam;
            iRet = ProcesserResponseMSG(rspMsg, respParam);
            if (iRet == MP_SUCCESS) {
                strRspMsg.readEnd = 1;
            }
            break;
        } else {
            if (memcpy_s(strRspMsg.data + strRspMsg.fileSize, rspMsg->GetSize2(),
                rspMsg->GetBuffer(), rspMsg->GetSize2()) != 0) {
                DestoryDppMessage(rspMsg);
                DBGLOG("memcpy_s  failed");
                return MP_FAILED;
            }
            strRspMsg.fileSize += rspMsg->GetSize2();
        }
        DestoryDppMessage(rspMsg);
    }

    if (iRet != MP_SUCCESS) {
        ERRLOG("Errors occur in service, taskid=%s, cmd=0x%x(%llu), iRet=%d.", taskId.c_str(), reqCmd, seq, iRet);
    }

    return iRet;
}

mp_int32 ArchiveStreamService::SendDppMsg(
    const mp_string &taskId, const Json::Value &strReqMsg, Json::Value &strRspMsg, mp_uint32 reqCmd)
{
    CDppMessage *reqMsg = NULL;
    mp_int32 iRet = MP_SUCCESS;

    mp_uint64 seq = m_handler->GetSeqNo();
    NEW_CATCH_RETURN_FAILED(reqMsg, CDppMessage);
    reqMsg->InitMsgHead(reqCmd, 0, seq);
    reqMsg->SetMsgSrc(ROLE_HOST_AGENT);
    reqMsg->SetMsgTgt(ROLE_DME_ARCHIVE);

    Json::Value protectedMsg;
    protectedMsg[MANAGECMD_KEY_CMDNO] = reqCmd;
    protectedMsg[MANAGECMD_KEY_BODY] = strReqMsg;
    reqMsg->SetMsgBody(protectedMsg);
    m_handler->SendDPMessage(taskId, reqMsg);
    CDppMessage *rspMsg = NULL;

    m_handler->GetResponseMessage(taskId, rspMsg, seq, DPP_MSG_TIMEOUT);
    if (rspMsg == NULL) {
        ERRLOG("Recev message failed, taskid=%s, cmd=0x%x, seq=%llu.", taskId.c_str(), reqCmd, seq);
        return MP_FAILED;
    }
    iRet = ProcesserResponseMSG(rspMsg, strRspMsg);
    INFOLOG("Recev message success, taskid=%s, cmd=0x%x, seq=%llu",
        taskId.c_str(),
        reqCmd,
        seq);

    if (iRet != MP_SUCCESS) {
        ERRLOG("Errors occur in service, taskid=%s, cmd=0x%x(%llu), iRet=%d.", taskId.c_str(), reqCmd, seq, iRet);
    }

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::GetFileData(
    const ArchiveStreamGetFileReq &getFileReq, ArchiveStreamGetFileRsq &getFileRsp)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;
    DBGLOG("taskId[%s] GetFileData. path[%s]  hadnle[%s]",
        m_taskID.c_str(), getFileReq.filePath, getFileReq.taskID.c_str());

    reqInfo[RECOVERYMESSAGE_OBJECT_FSID] = getFileReq.fsID;
    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;

    reqInfo[RECOVERYMESSAGE_FILE_PATH] = getFileReq.filePath;

    mp_uint32 readSize = TrReadSize(getFileReq.readSize);
    mp_uint32 maxResponseSize = TrReponseSize(getFileReq.maxResponseSize);
    reqInfo[RECOVERYMESSAGE_READ_SIZE] = (Json::UInt)readSize;
    reqInfo[RECOVERYMESSAGE_MAX_RESPONSE_SIZE] = (Json::UInt)maxResponseSize;
    reqInfo[RECOVERYMESSAGE_OFFSET] = (Json::UInt64)getFileReq.fileOffset;
    getFileRsp.offset = getFileReq.fileOffset;
    reciveCount = readSize / maxResponseSize;
    if (readSize % maxResponseSize != 0) {
        reciveCount++;
    }
    getFileRsp.data = (char *)calloc(1, readSize + 1);
    if (getFileRsp.data == NULL) {
        ERRLOG("taskId[%s] malloc failed.", getFileReq.archiveBackupId.c_str());
        return MP_FAILED;
    }

    for (mp_int32 times = 0; times < DEFULAT_SEND_TIMES; times++) {
        iRet = SendDppMsgWithRespones(m_taskID, reqInfo, getFileRsp, CMD_ARCHIVE_GET_FILE_DATA, reciveCount);
        if (iRet == MP_SUCCESS) {
            break;
        }
        Disconnect();
        Connect(m_busiIp, m_busiPort, true); // 失败重试需重新建连，保证连接稳定
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    } else {
        DBGLOG("taskId[%s] send dpp message success. file offset[%d], file size:%d",
            m_taskID.c_str(), getFileRsp.offset, getFileRsp.fileSize);
        getFileRsp.offset += getFileRsp.fileSize;
    }

    return iRet;
}
mp_int32 ArchiveStreamService::MountFileSystem(
    const mp_string &storeIp, const mp_string &sharePath, const mp_string &mountPath)
{
#ifndef WIN32
    std::ostringstream cmdParam;
    std::vector<mp_string> cmdResult;

    cmdParam << "serviceType=archivestream\n"
             << "storageIp=" << storeIp << "\n"
             << "nasFileSystemName=" << sharePath << "\n"
             << "LocalPath=" << mountPath << "\n";

    CRootCaller rootCaller;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PREPARE_NASMEDIA, cmdParam.str(), &cmdResult);
    if (ret != MP_SUCCESS) {
        WARNLOG("mount failed!storeIp(%s),sharePath(%s),mountPath(%s)",
            storeIp.c_str(),
            sharePath.c_str(),
            mountPath.c_str());
        return MP_ERROR;
    }

    INFOLOG(
        "mount success!storeIp(%s),sharePath(%s),mountPath(%s)", storeIp.c_str(), sharePath.c_str(), mountPath.c_str());
#endif

    return MP_SUCCESS;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::UnMountFileSystem(const mp_string &mountPath)
{
#ifndef WIN32

    std::ostringstream cmdParam;
    std::vector<mp_string> cmdResult;

    cmdParam << "serviceType=archivestream\n"
        << "LocalPath=" << mountPath << "\n";

    CRootCaller rootCaller;
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NASMEDIA, cmdParam.str(), &cmdResult);
    if (ret != MP_SUCCESS) {
        ERRLOG("umount failed!mountPath(%s)", mountPath.c_str());
        return MP_ERROR;
    }
#endif
    return MP_SUCCESS;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::PrepareRecovery(mp_string &metaFileDir,
    mp_string &parentDir, const std::string& cacheRepoName)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
    reqInfo[RECOVERYMESSAGE_CACHE_FILE_NAME] = metaFileDir;
    reqInfo[CACHE_REPO_NAME] = cacheRepoName;

    m_cachePath = metaFileDir;
    mp_size dirSize = m_dirList.size();
    for (mp_size i = 0; i < dirSize; ++i) {
        reqInfo[RECOVERYMESSAGE_DIRS].append(m_dirList.at(i));
    }

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_PREPARE_RECOVERY);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }

    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_META_FILE_DIR, metaFileDir);
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_PARENT_DIR, parentDir);
    CHECK_FAIL_EX(CheckParamStringEnd(metaFileDir, 0, ARCHIVE_STREAM_FORMAT_STRING));
    DBGLOG("taskId[%s] send dpp message success , m_mountPath:%s .", m_taskID.c_str(), m_mountPath.c_str());

#ifndef WIN32
    mp_string FSSharePath;
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_FS_SHARE_NAME, FSSharePath);
    CHECK_FAIL_EX(CheckParamStringEnd(FSSharePath, 0, ARCHIVE_STREAM_FORMAT_STRING));
    std::vector<mp_string> hostIpv4List;
    std::vector<mp_string> hostIpv6List;
    GET_JSON_ARRAY_STRING_OPTION(rspInfo, RECOVERYMESSAGE_LOGIC_IPV4_LIST, hostIpv4List);
    GET_JSON_ARRAY_STRING_OPTION(rspInfo, RECOVERYMESSAGE_LOGIC_IPV6_LIST, hostIpv6List);

    iRet = CheckIPLinkStatus(hostIpv4List, hostIpv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] add white list failed.", m_taskID.c_str());
        return iRet;
    }

    mp_int32 iMountRet = MP_FAILED;
    for (auto ip : hostIpv4List) {
        mp_string mountPath = mp_string(DEFAULT_MOUNT_PATH).append(PATH_SEPARATOR).append(FSSharePath);
        iMountRet = MountFileSystem(ip, FSSharePath + PATH_SEPARATOR, mountPath);
        if (iMountRet == MP_SUCCESS) {
            metaFileDir = mountPath + PATH_SEPARATOR + metaFileDir;
            m_mountPath = mountPath;
            INFOLOG("backupId: [%s]  mount success,mount path:%s .", m_backupId.c_str(), m_mountPath.c_str());
            break;
        }
    }
    return iMountRet;
#endif
    return MP_SUCCESS;
}

mp_int32 ArchiveStreamService::GetDoradoIp(std::vector<mp_string> &hostIpv4List)
{
    std::vector<mp_string> vecAgentIPInfo;
#ifndef WIN32
    CRootCaller rootCaller;
    if (rootCaller.Exec((mp_int32)ROOT_COMMAND_CAT, "/etc/hosts", &vecAgentIPInfo) != MP_SUCCESS) {
        ERRLOG("Get hosts info failed.");
        return MP_FAILED;
    }
    std::size_t idx;
    hostIpv4List.clear();
    for (auto ipInfo : vecAgentIPInfo) {
        idx = ipInfo.find("nas.storage.protectengine_a.host", 0);
        if (mp_string::npos != idx) {
            std::vector<mp_string> vecIPInfo;
            CMpString::StrSplit(vecIPInfo, ipInfo, ' ');
            hostIpv4List.push_back(vecIPInfo.front());
            INFOLOG("Get DoradoIp: [%s] success.", vecIPInfo.front().c_str());
        }
    }
#endif // WIN32
    return MP_SUCCESS;
}

#ifndef WIN32
// 检查ip连通性
mp_int32 ArchiveStreamService::CheckIPLinkStatus(
    std::vector<mp_string> &hostIpv4List, std::vector<mp_string> &hostIpv6List)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
    std::vector<mp_string> ipv4List;
    std::vector<mp_string> ipv6List;
    iRet = CIP::GetHostIPList(ipv4List, ipv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Host ip  failed");
        return iRet;
    }
    mp_string strDeployType;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, strDeployType);
    INFOLOG("CheckIPLinkStatus iRet is %d, strDeployType is %s", iRet, strDeployType.c_str());
    mp_bool isDorado = false;
    mp_string doradoLunIP = "";
    iRet = CIP::CheckIsDoradoEnvironment(isDorado);
    if (isDorado && iRet == MP_SUCCESS && strDeployType != HOST_ENV_DEPLOYTYPE_E6000) {
        GetDoradoIp(hostIpv4List);
        std::vector<mp_string> ipInfo;
        CMpString::StrSplit(ipInfo, hostIpv4List.front(), '.');

        if (!ipInfo.empty()) {
            doradoLunIP = ipInfo.front() + ".0.0.0/8";
            reqInfo[RECOVERYMESSAGE_AGENT_IP_LIST].append(doradoLunIP);
        }
    } else {
        iRet = SecureCom::CIP::CheckHostLinkStatus(hostIpv4List, hostIpv6List, ipv4List, ipv6List);
        if (iRet != MP_SUCCESS) {
            ERRLOG("CheckHostLinkStatus failed, iRet = %d", iRet);
            return iRet;
        }
        mp_size ipv4Size = ipv4List.size();
        for (mp_size i = 0; i < ipv4Size; ++i) {
            reqInfo[RECOVERYMESSAGE_AGENT_IP_LIST].append(ipv4List.at(i));
        }
        mp_size ipv6Size = ipv6List.size();
        for (mp_size i = 0; i < ipv6Size; ++i) {
            reqInfo[RECOVERYMESSAGE_AGENT_IP_LIST].append(ipv6List.at(i));
        }
    }

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_ADD_WHITE_LIST);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }

    return iRet;
}
#endif
AGENT_EXPORT mp_int32 ArchiveStreamService::QueryPrepareStatus(mp_int32 &state)  // fsid格式未定，需要修改
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_QUERY_PREPARE_STATE);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }
    GET_JSON_INT32_OPTION(rspInfo, RECOVERYMESSAGE_STATE, state);

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::GetBackupInfo(ArchiveStreamCopyInfo &copyInfo)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_GET_BACKUP_INFO);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }

    std::vector<mp_int64> vecExclude;
    GET_JSON_INT64_OPTION(rspInfo, RECOVERYMESSAGE_DIR_COUNT, copyInfo.dirCount);
    CHECK_FAIL_EX(CheckParamInteger64(copyInfo.dirCount, 0, ARCHIVE_STREAM_FORMAT_UINT32, vecExclude));
    GET_JSON_INT64_OPTION(rspInfo, RECOVERYMESSAGE_FILE_COUNT, copyInfo.fileCount);
    CHECK_FAIL_EX(CheckParamInteger64(copyInfo.fileCount, 0, ARCHIVE_STREAM_FORMAT_UINT32, vecExclude));
    GET_JSON_INT64_OPTION(rspInfo, RECOVERYMESSAGE_BACKUP_FILE_SIZE, copyInfo.backupSize);
    CHECK_FAIL_EX(CheckParamInteger64(copyInfo.backupSize, 0, ARCHIVE_STREAM_FORMAT_UINT32, vecExclude));

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::GetRecoverObjectList(
    mp_int64 readCountLimit, mp_string &checkpoint, mp_string &splitFile, mp_int64 &objectNum, mp_int32 &status)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
    reqInfo[RECOVERYMESSAGE_READ_COUNT_LIMIT] = (Json::Int64)readCountLimit;
    reqInfo[RECOVERYMESSAGE_CHECKPOINT] = checkpoint;
    reqInfo[RECOVERYMESSAGE_CACHE_FILE_NAME] = m_cachePath;

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_GET_RECOVERY_OBJECT_LIST);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }
    std::vector<mp_int64> vecExclude;
    GET_JSON_INT32_OPTION(rspInfo, RECOVERYMESSAGE_STATUS, status);
    CHECK_FAIL_EX(CheckParamInteger64(status, 0, ARCHIVE_STREAM_FORMAT_UINT32, vecExclude));
    GET_JSON_INT64_OPTION(rspInfo, RECOVERYMESSAGE_OBJECT_NUM, objectNum);
    CHECK_FAIL_EX(CheckParamInteger64(objectNum, 0, ARCHIVE_STREAM_FORMAT_UINT32, vecExclude));
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_SPLIT_FILE, splitFile);
    CHECK_FAIL_EX(CheckParamStringEnd(splitFile, 0, ARCHIVE_STREAM_FORMAT_STRING));
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_CHECKPOINT, checkpoint);
    CHECK_FAIL_EX(CheckParamStringEnd(checkpoint, 0, ARCHIVE_STREAM_FORMAT_LONG_STRING));
    splitFile = m_mountPath + PATH_SEPARATOR + splitFile;
    DBGLOG("taskId[%s] send dpp message success , splitFile:%s .", m_taskID.c_str(), splitFile.c_str());

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::GetDirMetaData(
    const mp_string &ObjectName, const mp_string &fsID, mp_string &MetaData)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
    reqInfo[RECOVERYMESSAGE_OBJECT_FSID] = fsID;
    reqInfo[RECOVERYMESSAGE_OBJECT_NAME] = ObjectName;

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_GET_DIR_META_DATA);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_METADATA, MetaData);
    CHECK_FAIL_EX(CheckParamStringEnd(MetaData, 0, ARCHIVE_STREAM_FORMAT_STRING));

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::GetFileMetaData(
    const mp_string &ObjectName, const mp_string &fsID, mp_string &MetaData)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;
    reqInfo[RECOVERYMESSAGE_OBJECT_NAME] = ObjectName;
    reqInfo[RECOVERYMESSAGE_OBJECT_FSID] = fsID;

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_GET_FILE_META_DATA);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }
    GET_JSON_STRING_OPTION(rspInfo, RECOVERYMESSAGE_METADATA, MetaData);
    CHECK_FAIL_EX(CheckParamStringEnd(MetaData, 0, ARCHIVE_STREAM_FORMAT_STRING));

    return iRet;
}

AGENT_EXPORT mp_int32 ArchiveStreamService::EndRecover()
{
    mp_int32 iRet = MP_FAILED;
    Json::Value reqInfo;
    int reciveCount = 0;

    reqInfo[RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID] = m_backupId;
    reqInfo[MANAGECMD_KEY_TASKID] = m_taskID;

#ifndef WIN32
    UnMountFileSystem(m_mountPath);
#endif

    Json::Value rspInfo;
    iRet = SendDppMsg(m_taskID, reqInfo, rspInfo, CMD_ARCHIVE_END_RECOVERY);
    if (iRet != MP_SUCCESS) {
        ERRLOG("taskId[%s] send dpp message failed.", m_taskID.c_str());
        return iRet;
    }

    return iRet;
}
