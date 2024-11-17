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
#ifndef __ARCHIVE_STREAM_SERVICE_H__
#define __ARCHIVE_STREAM_SERVICE_H__

#include <vector>
#include <map>
#include "common/Defines.h"
#include "common/Types.h"
#include "json/json.h"

#ifdef __cplusplus
extern "C" {
#endif
#define SIZE_1KB 1024

namespace {
    const mp_int32 CONSTANTS_1 = 1;
    const mp_int32 CONSTANTS_2 = 2;
    const mp_int32 CONSTANTS_4 = 4;
    const mp_int32 CONSTANTS_8 = 8;
    const mp_int32 CONSTANTS_16 = 16;
    const mp_int32 CONSTANTS_32 = 32;
    const mp_int32 CONSTANTS_64 = 64;
    const mp_int32 CONSTANTS_128 = 128;
    const mp_int32 CONSTANTS_256 = 256;
    const mp_int32 CONSTANTS_512 = 512;
}
class ArchiveStreamClientHandler;
AGENT_EXPORT typedef enum {
    ARCHIVESTREAM_READ_SIZE_4M = 0,
    ARCHIVESTREAM_READ_SIZE_8M = 1,
    ARCHIVESTREAM_READ_SIZE_16M = 2,
    ARCHIVESTREAM_READ_SIZE_32M = 3,
    ARCHIVESTREAM_READ_SIZE_64M = 4,
} ARCHIVESTREAM_READ_SIZE;

AGENT_EXPORT typedef enum {
    ARCHIVESTREAM_RESPONSE_SIZE_8K = 0,
    ARCHIVESTREAM_RESPONSE_SIZE_16K,
    ARCHIVESTREAM_RESPONSE_SIZE_32K,
    ARCHIVESTREAM_RESPONSE_SIZE_64K,
    ARCHIVESTREAM_RESPONSE_SIZE_128K,
    ARCHIVESTREAM_RESPONSE_SIZE_256K,
    ARCHIVESTREAM_RESPONSE_SIZE_512K,
    ARCHIVESTREAM_RESPONSE_SIZE_1M,
    ARCHIVESTREAM_RESPONSE_SIZE_2M,
    ARCHIVESTREAM_RESPONSE_SIZE_4M,
} ARCHIVESTREAM_RESPONSE_SIZE;

AGENT_EXPORT typedef enum {
    ARCHIVESTREAM_PREPARING = 0,
    ARCHIVESTREAM_PREPARE_SUCC = 1,
    ARCHIVESTREAM_PREPARE_FAILED = -1,
} ARCHIVESTREAM_PREPARE_STATE;

AGENT_EXPORT typedef enum {
    ARCHIVESTREAM_GET_RECOVERY_SUCC = 0,
    ARCHIVESTREAM_GET_RECOVERY_PREPARING = 1,
    ARCHIVESTREAM_GET_RECOVERY_END = 2,
    ARCHIVESTREAM_GET_RECOVERY_FAILED = 3,
} ARCHIVESTREAM_GET_RECOVERY_STATE;

static mp_uint32 TrReponseSize(mp_int32 readSizeEnum)
{
    mp_uint32 readSize;
    switch (readSizeEnum) {
        case ARCHIVESTREAM_RESPONSE_SIZE_8K:
            readSize = CONSTANTS_8 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_16K:
            readSize = CONSTANTS_16 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_32K:
            readSize = CONSTANTS_32 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_64K:
            readSize = CONSTANTS_64 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_128K:
            readSize = CONSTANTS_128 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_256K:
            readSize = CONSTANTS_256 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_512K:
            readSize = CONSTANTS_512 * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_1M:
            readSize = CONSTANTS_1 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_2M:
            readSize = CONSTANTS_2 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_RESPONSE_SIZE_4M:
            readSize = CONSTANTS_4 * SIZE_1KB * SIZE_1KB;
            break;
        default:
            readSize = CONSTANTS_512 * SIZE_1KB;
            break;
    }
    return readSize;
}

static mp_uint32 TrReadSize(mp_int32 readSizeEnum)
{
    mp_uint32 readSize;
    switch (readSizeEnum) {
        case ARCHIVESTREAM_READ_SIZE_4M:
            readSize = CONSTANTS_4 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_READ_SIZE_8M:
            readSize = CONSTANTS_8 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_READ_SIZE_16M:
            readSize = CONSTANTS_16 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_READ_SIZE_32M:
            readSize = CONSTANTS_32 * SIZE_1KB * SIZE_1KB;
            break;
        case ARCHIVESTREAM_READ_SIZE_64M:
            readSize = CONSTANTS_64 * SIZE_1KB * SIZE_1KB;
            break;
        default:
            readSize = CONSTANTS_8 * SIZE_1KB * SIZE_1KB;
            break;
    }
    return readSize;
}

struct AGENT_EXPORT ArchiveStreamGetFileReq {
    ArchiveStreamGetFileReq()
    {
        fileOffset = 0;
    }
    mp_string taskID; // 任务id
    mp_string archiveBackupId; // 副本id
    mp_string fsID; // 文件系统id 支持多文件系统，由splitFile中读取
    mp_string filePath; // 读取的文件的路径，由splitFile中读取
    mp_int64 fileOffset; // 文件偏移量   因s3查询数据原因，读取文件得以4M块形式读取，所以offset和readsize必须为4M的倍数，不然会读取失败
    mp_int32 readSize; // 单次读取文件的长度   枚举值，具体见 ARCHIVESTREAM_READ_SIZE
    // sdk与archive交互时，每帧报文大小；可根据网络状况进行修改，影响报文重传次数及报文交互次数，不影响具体功能 枚举值，具体见ARCHIVESTREAM_RESPONSE_SIZE
    mp_int32 maxResponseSize;
};

struct AGENT_EXPORT ArchiveStreamGetFileRsq {
    ArchiveStreamGetFileRsq()
    {
        offset = 0;
        fileSize = 0;
        readEnd = 0;
    }
    mp_string archiveBackupId; // 副本id
    mp_string filePath; // 任务id
    mp_string fsID; // 文件系统id
    mp_int64 offset; // 文件偏移量
    mp_int32 fileSize; // 读取到的文件的实际长度（当文件长度不如想要读取长度时，不等于读取长度）
    mp_int32 readEnd; // 文件是否读取完  1：文件已获取完  0：文件尚未获取完
    char* data; // 文件数据的二进制流
};

struct AGENT_EXPORT ArchiveStreamCopyInfo  {
    ArchiveStreamCopyInfo()
    {
        dirCount = 0;
        fileCount = 0;
        backupSize = 0;
    }

    mp_int64 dirCount; // 目录数量，
    mp_int64 fileCount; // 文件数量
    mp_int64 backupSize; // 备份副本大小
};

class AGENT_EXPORT ArchiveStreamService {
public:
    ArchiveStreamService();
    
    ~ArchiveStreamService();

    // 初始化接口，传入需要恢复的副本id、任务id、目录结构    dirList为细腻度恢复的目录及文件名列表，以,隔开
    mp_int32 Init(const mp_string &backupId, const mp_string &taskID, const mp_string &dirList);

    // 连接archive busiIp为archive的ip列表，以,隔开    参数由dme下发归档直接恢复参数中携带
    mp_int32 Connect(const mp_string &busiIp, mp_int32 busiPort, bool openSsl);

    // 准备备份  archive去s3上读取副本相关信息耗时较长，所以做成异步接口，该函数为下发命令接口，后续调用QueryPrepareStatus接口查询archive是否完成读取操作
    // metaFileDir为元数据文件路径，后续根据读取到的splitFile文件中的元数据文件名称来读取对应的元数据文件列表
    // cacheRepoName 默认为空，可选择传入，传入后将使用cache仓来跟Archive共享数据，不传入时由Archive创建的文件系统来共享数据
    mp_int32 PrepareRecovery(mp_string &metaFileDir, const std::string& cacheRepoName = "");

    // 查询准备状态  state标识备份状态：0表示进行中  1表示已完成  -1表示失败
    mp_int32 QueryPrepareStatus(mp_int32 &state);

    // 获取副本信息  // 非必须接口，可根据该信息做子任务的划分，将子任务划分的更加均匀恰当
    mp_int32 GetBackupInfo(ArchiveStreamCopyInfo &copyInfo);

    // 获取对象列表 readCountLimit -- 读取数量  checkpoint -- 检查点,每次上传该字段，
    // 为上次成功获取的字段（需要做持久化，如果正常读取，那就发送上次的字段，如果是续作，那就发成功读取那次的字段，不需要关系具体格式，做透传即可）
    // status  0-本次获取已完成(可以读取），1-列举中（文件写入中，再次调用该接口），2-所有对象都已获取完成，3-失败
    // objectNum -- 实际返回的数量
    // splitFile  一份存储文件列表及文件信息的文件，返回值为绝对路径，由插件读取文件并解析
    // 文件列表csv文件格式如下（每行一个文件）：
    //  #操作类型(0-恢复，1-删除),文件系统ID,文件名,文件类型（0-非聚合文件，1-聚合文件，2-细粒度恢复文件）,
    // 元数据信息(metafile，offset，length),1(软连接),/lib/libscheduler.so.1（链接的原始文件）
    mp_int32 GetRecoverObjectList(mp_int64 readCountLimit, mp_string &checkpoint, mp_string &splitFile,
        mp_int64 &objectNum, mp_int32 &status);

    // 获取指定文件内容
    mp_int32 GetFileData(const ArchiveStreamGetFileReq &getFileReq, ArchiveStreamGetFileRsq &getFileRsp);

    // 获取对应的元数据， 细腻度恢复且只恢复文件时，调用该接口  ObjectName -- 需要读取的元数据对应的文件/目录路径  MetaData -- 元数据字符串，具体格式由各插件自身备份时决定
    mp_int32 GetDirMetaData(const mp_string &ObjectName, const mp_string &fsID, mp_string &MetaData);
    mp_int32 GetFileMetaData(const mp_string &ObjectName, const mp_string &fsID, mp_string &MetaData);

    // 结束备份
    mp_int32 EndRecover();

    // 断开连接
    mp_int32 Disconnect();
    
    mp_int32 UnMountFileSystem(const mp_string &mountPath);

private:
#ifndef WIN32
    // 检查ip连通性
    mp_int32 CheckIPLinkStatus(std::vector<mp_string> &hostIpv4List, std::vector<mp_string> &hostIpv6List);
#endif
    mp_int32 MountFileSystem(const mp_string &storeIp, const mp_string &sharePath, const mp_string &mountPath);
    mp_int32 SendDppMsgWithRespones(const mp_string &taskId, const Json::Value &strReqMsg,
        ArchiveStreamGetFileRsq &strRspMsg, mp_uint32 reqCmd, mp_uint32 reciveCount);
    mp_int32 SendDppMsg(const mp_string &taskId, const Json::Value &strReqMsg, Json::Value &strRspMsg,
        mp_uint32 reqCmd);
    mp_int32 GetDoradoIp(std::vector<mp_string> &hostIpv4List);
    mp_bool SplitIpList(const mp_string &busiIp, const mp_int32 &busiPort, std::vector<mp_string> &IPList);

private:
    mp_string m_backupId;
    mp_string m_taskID;
    std::vector<mp_string> m_dirList;
    ArchiveStreamClientHandler *m_handler;
    mp_string m_mountPath;
    mp_string m_cachePath;
    mp_string m_busiIp;
    mp_int32 m_busiPort;
};
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
