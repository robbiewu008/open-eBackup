/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file GetFileFromArchive.cpp
 * @brief  Contains function declarations for GetFileFromArchive
 * @version 1.0.0
 * @date 2022-07-15
 * @author hexiaoqiang wx886037
 */
#include "tools/agentcli/GetFileFromArchive.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Uuid.h"
#include "common/Utils.h"
#include "securecom/CodeConvert.h"
#include "host/host.h"
namespace {
const mp_int32 READ_FILE_COUNT_LIMIT = 1000;
const mp_int32 BASE64_DATA_LENGTH = 2;
const mp_int32 AGENTCLI_NUM_2 = 2;
const mp_int32 AGENTCLI_NUM_3 = 3;
const mp_int32 GET_FILELIST_INTERVAL = 200;
}
mp_int32 GetFileFromArchive::GetFileListInfo(std::unique_ptr<ArchiveStreamService> &clientHandler,
    const mp_string &localPath)
{
    mp_string MetaDataFileDir;
    if (clientHandler->PrepareRecovery(MetaDataFileDir) != MP_SUCCESS) {
        ERRLOG("Archive client PrepareRecovery false");
        return MP_FAILED;
    }

    mp_int32 prepareStatus = ARCHIVESTREAM_PREPARING;
    while (prepareStatus == 0) {
        if (clientHandler->QueryPrepareStatus(prepareStatus) != MP_SUCCESS ||
            prepareStatus == ARCHIVESTREAM_PREPARE_FAILED) {
            ERRLOG("Archive client QueryPrepareStatus false");
            return MP_FAILED;
        }
        if (prepareStatus != ARCHIVESTREAM_PREPARE_SUCC) {
            CMpTime::DoSleep(GET_FILELIST_INTERVAL);
        }
    };

    mp_int32 satate = ARCHIVESTREAM_GET_RECOVERY_PREPARING;
    mp_int64 readLimit = READ_FILE_COUNT_LIMIT;
    mp_int64 objectNum = 0;
    mp_string checkpoint;
    mp_string splitFile;

    mp_int32 iRet = MP_FAILED;
    while (satate == ARCHIVESTREAM_GET_RECOVERY_PREPARING || satate == ARCHIVESTREAM_GET_RECOVERY_SUCC) {
        iRet = clientHandler->GetRecoverObjectList(readLimit, checkpoint, splitFile, objectNum, satate);
        if (iRet != MP_SUCCESS || satate == ARCHIVESTREAM_GET_RECOVERY_FAILED) {
            ERRLOG("Archive client GetRecoverObjectList false");
            return MP_FAILED;
        }

        if (satate == ARCHIVESTREAM_GET_RECOVERY_SUCC || satate == ARCHIVESTREAM_GET_RECOVERY_END) {
            DBGLOG("Archive client GetRecoverObjectList success splitFile:[%s]", splitFile.c_str());
            iRet = HandleFileListInfo(clientHandler, splitFile, localPath);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Archive client HandleSplitFile false");
                break;
            }
        } else {
            CMpTime::DoSleep(GET_FILELIST_INTERVAL);
        }
    }
    return iRet;
}

mp_int32 GetFileFromArchive::HandleFileListInfo(std::unique_ptr<ArchiveStreamService> &clientHandler,
    mp_string &splitFile, const mp_string &localPath)
{
    mp_int32 iRet = MP_FAILED;
    std::vector<mp_string> vecRlt;
    if (CMpFile::ReadFile(splitFile, vecRlt)!= MP_SUCCESS) {
        ERRLOG("Read splitFile false");
        return MP_FAILED;
    }
    for (mp_uint32 i = 0; i < vecRlt.size(); i++) {
        mp_string strTmp = vecRlt[i];
        std::vector<std::string> strSpintInfo;
        CMpString::StrSplit(strSpintInfo, strTmp, CHAR_COMMA);
        if (strSpintInfo.size() < AGENTCLI_NUM_3) {
            ERRLOG("File info is error: %s ", strTmp.c_str());
            return MP_FAILED;
        }
        mp_string strFileName = strSpintInfo[AGENTCLI_NUM_2];
        mp_string strfsID = strSpintInfo[1];
        mp_string strDecoFileName;
        if (!CodeConvert::DecodeBase64(strFileName.size() * BASE64_DATA_LENGTH, strFileName, strDecoFileName)) {
            ERRLOG("Base64 decode file name failed: ", strFileName);
            return MP_FAILED;
        }
        INFOLOG("DecodeBase64 strDecoFileName:[%s]", strDecoFileName.c_str());
        if (strDecoFileName[strDecoFileName.length() -1] == '/') {
            INFOLOG("Current Object is Path :[%s], not need to read.", strDecoFileName.c_str());
            continue;
        }
        std::vector<std::string> filepath;
        CMpString::StrSplit(filepath, strDecoFileName, CHAR_SLASH);
        if (!filepath.empty()) {
            std::string fileName = localPath + "/" + filepath[filepath.size() - 1];
            if (CMpFile::FileExist(fileName)) {
                ERRLOG("File exist %s.", fileName.c_str());
                return MP_FAILED;
            }
            iRet = DownloadFile(clientHandler, strDecoFileName, strfsID, fileName);
        }
    }
    return iRet;
}

mp_int32 GetFileFromArchive::DownloadFile(std::unique_ptr<ArchiveStreamService> &clientHandler,
    const mp_string &strDecoFileName, const mp_string &strfsID, const mp_string &fileName)
{
    mp_int32 iRet = MP_FAILED;
    ArchiveStreamGetFileReq getFileReq;
    getFileReq.filePath = strDecoFileName;
    getFileReq.fsID = strfsID;
    getFileReq.readSize = ARCHIVESTREAM_READ_SIZE_32M;
    getFileReq.maxResponseSize = ARCHIVESTREAM_RESPONSE_SIZE_4M;

    ArchiveStreamGetFileRsq getFileRsp;
    getFileRsp.offset = 0;
    getFileRsp.readEnd = 0;

    FILE* FileHandle = fopen(fileName.c_str(), "ab+");
    if (FileHandle == nullptr) {
        ERRLOG("Open File[%s] failed.", fileName.c_str());
        return MP_FAILED;
    }

    while (getFileRsp.readEnd == 0) {
        getFileRsp.fileSize = 0;
        getFileReq.fileOffset = getFileRsp.offset;
        iRet = clientHandler->GetFileData(getFileReq, getFileRsp);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Archive client GetFileData false");
            (void)fclose(FileHandle);
            return MP_FAILED;
        }

        size_t result = fwrite(getFileRsp.data, sizeof(char), getFileRsp.fileSize, FileHandle);
        if (getFileRsp.data != nullptr) {
            free(getFileRsp.data);
            getFileRsp.data = nullptr;
        }

        if (result != getFileRsp.fileSize) {
            ERRLOG("Write File failed:%s numBytes:%u, not written bytes:%u", fileName.c_str(),
                getFileRsp.fileSize, getFileRsp.fileSize - result);
            (void)fclose(FileHandle);
            return MP_FAILED;
        }
    }
    (void)fclose(FileHandle);
    return MP_SUCCESS;
}

mp_int32 GetFileFromArchive::Handle(const mp_string &backupId, const mp_string &busiIp,
    const mp_string &localPath, const mp_string &dirList)
{
    if (backupId.empty() || busiIp.empty() || localPath.empty()) {
        ERRLOG("Input info error, backupId:[%s] , backupIp:[%s] , localPath:[%s] .",
            backupId.c_str(), busiIp.c_str(), localPath.c_str());
        return MP_FAILED;
    }
    if (!CMpFile::DirExist(localPath.c_str())) {
        ERRLOG("The Path is not Exist or Permission denied,localPath:[%s] .", localPath.c_str());
        return MP_FAILED;
    }
    std::vector<std::string> vecParam;
    CMpString::StrSplit(vecParam, busiIp, CHAR_COLON);
    if (vecParam.size() != AGENTCLI_NUM_2) {
        ERRLOG("Input archive connection info[%s] is incorrect .\n", busiIp.c_str());
        return MP_FAILED;
    }

    std::unique_ptr<ArchiveStreamService> clientHandler = std::make_unique<ArchiveStreamService>();
    if (clientHandler == nullptr) {
        ERRLOG("Archive client ptr is null");
        return MP_FAILED;
    }
    mp_string uuid;
    CUuidNum::GetUuidStandardStr(uuid);

    int ret = clientHandler->Init(backupId, uuid, dirList);
    if (ret != MP_SUCCESS) {
        ERRLOG("Init archive client failed");
        return MP_FAILED;
    }

    ret = clientHandler->Connect(vecParam[0], CMpString::SafeStoi(vecParam[1], 0), true);
    if (ret != MP_SUCCESS) {
        ERRLOG("Archive client Connect false");
    } else {
        ret = GetFileListInfo(clientHandler, localPath);
        clientHandler->EndRecover();
    }
    clientHandler->Disconnect();
    return ret;
}
