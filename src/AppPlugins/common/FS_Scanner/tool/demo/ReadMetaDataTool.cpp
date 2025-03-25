/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
* Description: read dcache&fcache.
* Author: w00444223
* Create: 2024-02-28
*/
#ifndef WIN32
#include "BufferQueue.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanConfig.h"

#include "log/Log.h"
#include "ScannerUtils.h"
#include "ControlFileUtils.h"

using namespace std;
using namespace Module;

namespace ReadMetaDataTool
{
ScanConfig m_config {};

int ReadXMeta(std::string &metaDir, uint64_t xMetaId, uint64_t xMetaOffset, std::vector<XMetaField> &xMeta)
{
    ControlFileUtils cfu {};
    std::string xMetaFileName = cfu.GetXMetaFileName(metaDir, xMetaId);
    std::shared_ptr<XMetaParser> xMetaFileObj =
        cfu.CreateXMetaFileObj(xMetaFileName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (xMetaFileObj == nullptr) {
        ERRLOG("Failed to open xmetafile: %s", xMetaFileName.c_str());
        return Module::FAILED;
    }

    if (xMetaFileObj->ReadXMeta(xMeta, xMetaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read xmeta file");
        xMetaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
        return Module::FAILED;
    }

    xMetaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
    return Module::SUCCESS;
}

int ReadDirMeta(std::string &metaDir, uint16_t metaId, uint64_t metaOffset, DirMetaWrapper &dmWrapper)
{
    ControlFileUtils cfu {};
    std::string metaFileName = cfu.GetMetaFileName(metaDir, metaId);
    std::shared_ptr<MetaParser> metaFileObj = cfu.CreateMetaFileObj(metaFileName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (metaFileObj == nullptr) {
        ERRLOG("Failed to open metafile: %s", metaFileName.c_str());
        return Module::FAILED;
    }
    if (metaFileObj->GetFileVersion() != FCACHE_HEADER_VERSION_V20) {
        ERRLOG("Invalid Current Meta File Version.");
        metaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
        return Module::FAILED;
    }
    if (metaFileObj->ReadDirectoryMeta(dmWrapper.m_meta, metaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read meta file.");
        return Module::FAILED;
    }
    metaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
    ReadXMeta(metaDir, dmWrapper.m_meta.m_xMetaFileIndex, dmWrapper.m_meta.m_xMetaFileOffset, dmWrapper.m_xMeta);
    return Module::SUCCESS;
}

int ReadFileMeta(std::string &metaDir, uint16_t metaId, uint64_t metaOffset, FileMetaWrapper &fmWrapper)
{
    ControlFileUtils cfu {};
    std::string metaFileName = cfu.GetMetaFileName(metaDir, metaId);
    std::shared_ptr<MetaParser> metaFileObj = cfu.CreateMetaFileObj(metaFileName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (metaFileObj == nullptr) {
        ERRLOG("Failed to open metafile: %s", metaFileName.c_str());
        return Module::FAILED;
    }
    if (metaFileObj->GetFileVersion() != FCACHE_HEADER_VERSION_V20) {
        ERRLOG("Invalid Current Meta File Version.");
        metaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
        return Module::FAILED;
    }
    if (metaFileObj->ReadFileMeta(fmWrapper.m_meta, metaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read meta file");
        metaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
        return Module::FAILED;
    }
    metaFileObj->Close(CTRL_FILE_OPEN_MODE::READ);
    ReadXMeta(metaDir, fmWrapper.m_meta.m_xMetaFileIndex, fmWrapper.m_meta.m_xMetaFileOffset, fmWrapper.m_xMeta);
    return Module::SUCCESS;
}

int ReadFcache(std::string &metaDir, DirCache &dcache)
{
    uint16_t fcacheFileIndex = dcache.m_fcacheFileId;
    uint64_t fcacheOffset = dcache.m_fcacheOffset;
    uint16_t metaFileIndex = dcache.m_fileId;
    uint32_t totalCount = dcache.m_totalFiles;

    ControlFileUtils cfu {};
    std::string fcacheName = cfu.GetFileCacheFileName(metaDir, fcacheFileIndex);
    std::shared_ptr<FileCacheParser> file = nullptr;
    std::queue<FileCache> fcQueue = {};
    file = cfu.CreateFileCacheObj(fcacheName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (file == nullptr) {
        return Module::FAILED;
    }
    if (file->ReadFileCacheEntries(fcQueue, fcacheOffset, totalCount, metaFileIndex) != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }

    while (true) {
        if (fcQueue.empty() && totalCount > 0) {
            if (file->ReadFileCacheEntries(fcQueue, totalCount, metaFileIndex) != CTRL_FILE_RETCODE::SUCCESS) {
                return Module::FAILED;
            }
        }
        if (fcQueue.empty()) {
            break;
        }
        FileCache fcache = fcQueue.front();
        fcQueue.pop();
        totalCount--;
        FileMetaWrapper fmWrapper {};
        ReadFileMeta(metaDir, fcache.m_fileId, fcache.m_mdataOffset, fmWrapper);
        std::string filename = FS_SCANNER::GetFileOrDirNameFromXMeta(fmWrapper.m_xMeta);
        std::string key = ParserUtils::ParseObjectKey(fmWrapper.m_xMeta);
        ::printf("[METAINFO]fcache - filename: %s, obskey: %s, inode: %u, mdataoffset: %u, hastag: %u, crc: %u, "
            "fileId: %u, metalength: %u, compareFilg: %u\n",
            filename.c_str(), key.c_str(), fcache.m_inode, fcache.m_mdataOffset, fcache.m_filePathHash.crc,
            fcache.m_fileMetaHash.crc, fcache.m_fileId, fcache.m_metaLength, fcache.m_compareFlag);
    }
    return Module::SUCCESS;
}

int ReadDcache(std::string &metaDir)
{
    m_config.jobId = "114514";
    m_config.scanType = ScanJobType::FULL;
    m_config.scanCtrlMaxEntriesFullBkup = 5000000;
    m_config.scanCtrlMaxDataSize = "1073741824000000";

    ControlFileUtils cfu {};
    std::string dcacheName = cfu.GetDirCacheFileName(metaDir);
    shared_ptr<DirCacheParser> currDcacheObj = cfu.CreateDcacheObj(dcacheName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (currDcacheObj == nullptr) {
        ERRLOG("dcache obj create fail");
        return Module::FAILED;
    }

    std::queue<DirCache> dcQueue {};
    while (true) {
        if (dcQueue.empty()) {
            currDcacheObj->ReadDirCacheEntries(dcQueue, DCACHE_ENTRY_BATCH_READ_CNT);
        }
        if (dcQueue.empty()) {
            break;
        }
        while (!dcQueue.empty()) {
            DirCache dcache = dcQueue.front();
            dcQueue.pop();
            DirMetaWrapper dmWrapper {};
            ReadDirMeta(metaDir, dcache.m_fileId, dcache.m_mdataOffset, dmWrapper);
            std::string dirPath = FS_SCANNER::GetPathFromXMeta(dmWrapper.m_xMeta);
            ::printf("[METAINFO]dcache - dirpath: %s, inode: %u, mdataOffset: %u, fcacheOffset: %u, hashTag: %u, "
                "m_crc: %u, totalFiles: %u, fileId: %u, fcacheFileId: %u, metaLength: %u\n", dirPath.c_str(),
                dcache.m_inode, dcache.m_mdataOffset, dcache.m_fcacheOffset, dcache.m_dirPathHash.crc,
                dcache.m_dirMetaHash.crc, dcache.m_totalFiles, dcache.m_fileId, dcache.m_fcacheFileId,
                dcache.m_metaLength);
            ReadFcache(metaDir, dcache);
        }
    }
    currDcacheObj->Close(CTRL_FILE_OPEN_MODE::READ);
    return Module::SUCCESS;
}

} // end namespace ReadMetaDataTool

int TestReadMeta(std::string metaDir)
{
    return ReadMetaDataTool::ReadDcache(metaDir);
}

#endif
