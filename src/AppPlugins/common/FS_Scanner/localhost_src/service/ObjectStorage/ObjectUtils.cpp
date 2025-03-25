/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage utility function.
* Author: w00444223
* Create: 2023-12-04
*/

#include "ObjectUtils.h"
#include "ParserStructs.h"
#include "ScannerUtils.h"

using namespace std;
using namespace Module;

namespace {
    const std::string CONFLICTED_FILE_NAME = "conflicted_key.txt";
}

uint64_t ObjectUtils::HashForObject(const std::string& key)
{
    return m_genInode->GetInodeValue(key.c_str(), key.length());
}

bool ObjectUtils::IsBucketName(const std::vector<ObjectStorageBucket>& buckets, const std::string& path)
{
    for (auto item : buckets) {
        if (path == item.bucketName) {
            return true;
        }
    }
    return false;
}

std::string ObjectUtils::GetFullPath(const std::string& prefix, const std::string& path)
{
    std::string tmpPrefix = prefix;
    std::string tmpPath = path;

    if (!prefix.empty() && (prefix.back() == PATH_SEPERATOR[0])) {
        tmpPrefix = prefix.substr(0, prefix.size() - 1);
    }

    if (!path.empty() && (path.front() == PATH_SEPERATOR[0])) {
        tmpPath = path.substr(1);
    }

    return tmpPrefix + PATH_SEPERATOR + tmpPath;
}

std::string ObjectUtils::GetObjectDirName(const std::vector<XMetaField>& xMeta)
{
    for (auto &item : xMeta) {
        if (item.m_xMetaType == XMETA_TYPE::XMETA_TYPE_NAME) {
            return item.m_value;
        }
    }
    return "";
}

std::string ObjectUtils::GetObjectPath(const std::vector<XMetaField>& xMeta)
{
    for (auto &item : xMeta) {
        if (item.m_xMetaType == XMETA_TYPE::XMETA_TYPE_KEY) {
            return item.m_value;
        }
    }
    return "";
}
std::string ObjectUtils::MakeObjectDirName(const DirStat &dirStat)
{
    std::string curPath = dirStat.m_path;
    // 桶前缀可能超过1024字节，需要hash之后保存，减少空间占用
    if (dirStat.flag == static_cast<uint8_t>(DirStatFlag::PREFIX)) {
        // 添加“/”开头，与对象 a/b/c/ 区分，否则必然存在hash冲突，因为输入 a/b/c/ 既是前缀又是对象
        curPath = std::to_string(HashForObject(PATH_SEPERATOR + dirStat.m_path)); // 前缀，如： /a/b/c/
    }
    return GetFullPath(dirStat.m_prefix, curPath);
}

std::string ObjectUtils::GetObjectFileName(const FileMeta &fmeta)
{
    // 文件名与inode保持一致
    if (fmeta.m_inode != 0) {
        return std::to_string(fmeta.m_inode);
    }
    return "";
}

uint64_t ObjectUtils::GetObjectDirInode(const DirStat &dirStat)
{
    std::string fullPath = GetFullPath(dirStat.m_prefix, dirStat.m_path);
    return HashForObject(fullPath); // 目录全路径，如: /bucket/prefix1/a
}

uint64_t ObjectUtils::GetObjectFileInode(const FileStat& fileStat)
{
    return HashForObject(fileStat.m_key); // 对象文件的key, 如：a/b/c/1.txt
}

void ObjectUtils::CopyStatToDirMeta(DirMeta &dmeta, const DirStat &dirStat)
{
    dmeta.type = static_cast<uint16_t>(MetaType::OBJECT);
    dmeta.m_inode = GetObjectDirInode(dirStat);
    dmeta.m_size  = dirStat.m_size;
    dmeta.m_mode  = S_IFDIR;
    dmeta.m_uid   = dirStat.m_uid;
    dmeta.m_gid   = dirStat.m_gid;
    dmeta.m_atime = dirStat.m_atime;
    dmeta.m_mtime = dirStat.m_mtime;
    dmeta.m_ctime = dirStat.m_ctime;
}

void ObjectUtils::CopyStatToFileMeta(FileMeta &fmeta, const FileStat& fileStat)
{
    fmeta.type = static_cast<uint16_t>(MetaType::OBJECT);
    fmeta.m_inode = GetObjectFileInode(fileStat);
    fmeta.m_size  = fileStat.m_size;
    fmeta.m_atime = fileStat.m_atime;
    fmeta.m_mtime = fileStat.m_mtime;
    fmeta.m_ctime = fileStat.m_ctime;
    fmeta.m_gid   = fileStat.m_gid;
    fmeta.m_uid   = fileStat.m_uid;
    fmeta.m_mode  = fileStat.m_mode;
    fmeta.m_rdev  = fileStat.m_rdev;
    fmeta.m_nlink = fileStat.m_nlink;
}

int ObjectUtils::SetObjectInode()
{
    m_genInode = std::make_shared<ObjectInode>();
    return (m_genInode == nullptr) ? Module::FAILED : Module::SUCCESS;
}

void ObjectUtils::SaveInodeConflict(std::string dirPath)
{
    if (!m_genInode->IsExistConflict()) {
        m_genInode.reset();
        return;
    }

    FS_SCANNER::CreateDirRecurve(dirPath);
    std::string conflictHashValueFile = dirPath + PATH_SEPERATOR + CONFLICTED_FILE_NAME;
    m_genInode->SaveConflictRecord(conflictHashValueFile);
    m_genInode.reset();
}

void ObjectUtils::ReadInodeConflict(std::string dirPath)
{
    if (!FS_SCANNER::PathExist(dirPath)) {
        return;
    }

    std::string conflictHashValueFile = dirPath + PATH_SEPERATOR + CONFLICTED_FILE_NAME;
    m_genInode->ReadConflictRecord(conflictHashValueFile);
}
