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
#include "BackupImageBase.h"

#include <string>
#include <functional>

#include "GuardedFile.h"
#include "InfoFileMgr.h"

using namespace std;
using namespace Module;

namespace {
    const string MODULE_NAME = "BackupImageBase";
    const string IMAGE_INFO_FILE = "image.info";
    const string METAFILE_NAME = "metafile";
    const string META_TYPE = "meta";
    const int THREAD_COUNT = 2;
    const int READ_LINE_SIZE = 50;
    const string NOT_AGGREGATED_FLAG = "";
}

BackupImageBase::BackupImageBase(BackupIOInterface &backupIO) : m_backupIO(backupIO)
{
    m_handle.callBackFunc = reinterpret_cast<void *>(&WriteCallBack);
    m_handle.callBackData = this;
}

LayoutRetCode BackupImageBase::UpdateImageInfo(const ImageInfo &imageInfo)
{
    if (imageInfo.imageId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "imageId is empty." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    string fileName = GetImageFileName(imageInfo, "", IMAGE_INFO_FILE);
    if (!m_backupIO.IsFileExist(fileName)) {
        HCP_Log(ERR, MODULE_NAME) << "Image not exist. image info file:" << fileName << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }

    if (!WriteImageInfo(imageInfo, fileName)) {
        LogImageInfo(ERR, "Write image info failed.", imageInfo);
        return LayoutRetCode::FAILED;
    }
    m_imageInfo = imageInfo;

    LogImageInfo(DEBUG, "Succeeded to update image info.", m_imageInfo);
    return LayoutRetCode::SUCCESS;
}
 
ImageInfo BackupImageBase::GetImageInfo() const
{
    return m_imageInfo;
}

void BackupImageBase::IncreaseImageWrittenSize(uint64_t size)
{
    m_writtenSize += size;
}

void BackupImageBase::IncreaseImageRealWrittenSize(uint64_t size)
{
    m_realWrittenSize += size;
}

void BackupImageBase::IncreaseImageDeletedCount(uint64_t size)
{
    m_deletedFileCount += size;
}

/* image operation */
bool BackupImageBase::CheckIfImageExists(const ImageInfo &imageInfo)
{
    if (imageInfo.imageId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "imageId is empty." << HCPENDLOG;
        return false;
    }
    string fileName = GetImageFileName(imageInfo, "", IMAGE_INFO_FILE);
    return m_backupIO.IsFileExist(fileName);
}

LayoutRetCode BackupImageBase::CreateImage(const ImageInfo &imageInfo)
{
    HCP_Log(DEBUG, MODULE_NAME) << "Begin to create image." << HCPENDLOG;
    if (imageInfo.imageId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "ImageId is empty." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }

    string fileName = GetImageFileName(imageInfo, "", IMAGE_INFO_FILE);
    if (!WriteImageInfo(imageInfo, fileName)) {
        HCP_Log(ERR, MODULE_NAME) << "Write image info failed." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    m_imageInfo = imageInfo;

    LogImageInfo(INFO, "Successfully created image.", m_imageInfo);
    return LayoutRetCode::SUCCESS;
}

void BackupImageBase::LogImageInfo(const int level, const string &message, const ImageInfo &imageInfo)
{
    HCP_Log(level, MODULE_NAME) << WIPE_SENSITIVE(message)
                               << ", imageId:" << imageInfo.imageId
                               << ", jsonVersion:" << imageInfo.jsonVersion
                               << ", backupType:" << imageInfo.backupType
                               << ", imageIndex:" << imageInfo.imageIndex
                               << ", resourceId:" << imageInfo.resourceId
                               << ", imageFileSize:" << imageInfo.imageFileSize
                               << ", imageFileCount:" << imageInfo.imageFileCount
                               << HCPENDLOG;
}

LayoutRetCode BackupImageBase::DeleteImage(const ImageInfo &imageInfo)
{
    HCP_Log(INFO, MODULE_NAME) << "Begin to delete image: " << imageInfo.imageId << HCPENDLOG;
    if (!CheckIfImageExists(imageInfo)) {
        HCP_Log(WARN, MODULE_NAME) << "The image to be deleted does not exist. ImageId:" << imageInfo.imageId
                                   << HCPENDLOG;
        return LayoutRetCode::SUCCESS;
    }

    m_handle.callBackFunc = reinterpret_cast<void *>(&DeleteCallBack);
    if (m_backupIO.GetMediaType() == static_cast<int32_t>(MediaType::OBS)) {
        vector<string> imageList;
        if (!m_backupIO.GetDirectoryList(imageInfo.resourceId, imageList)) {
            HCP_Log(ERR, MODULE_NAME) << "GetDirectoryList failed. imageSetPath: " << imageInfo.resourceId << HCPENDLOG;
            return LayoutRetCode::FAILED;
        }

        string imageName = FormatImageName(imageInfo, "");
        for (vector<string>::iterator itor = imageList.begin(); itor != imageList.end();) {
            itor->erase(0, itor->find_last_of("/") + 1);
            if (*itor == imageName || itor->find(imageName) == string::npos) {
                itor = imageList.erase(itor);
            } else {
                itor++;
            }
        }
        // delete continue image
        for (const auto &continueImage : imageList) {
            string continueImagePath = imageInfo.resourceId + "/" + continueImage;
            if (!m_backupIO.DeleteAll(continueImagePath, m_handle)) {
                HCP_Log(ERR, MODULE_NAME) << "Failed to delete continue image: " << continueImagePath << HCPENDLOG;
                return LayoutRetCode::FAILED;
            }
            HCP_Log(DEBUG, MODULE_NAME) << "Succeeded to delete continue image: " << continueImagePath << HCPENDLOG;
        }
    }

    // delete main image
    string image = GetImageFileName(imageInfo, "", "");
    vector<string> mainImage{image + "data", image + "meta", image};
    for (const auto &dir : mainImage) {
        if (!m_backupIO.DeleteAll(dir, m_handle)) {
            HCP_Log(ERR, MODULE_NAME) << "Failed to delete main image" << image << HCPENDLOG;
            return LayoutRetCode::FAILED;
        }
    }
    HCP_Log(INFO, MODULE_NAME) << "Succeeded to delete image: " << imageInfo.imageId << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode BackupImageBase::OpenImage(ImageInfo &imageInfo)
{
    if (!m_imageInfo.imageId.empty()) {
        return LayoutRetCode::SUCCESS;
    }
    string fileName = GetImageFileName(imageInfo, "", IMAGE_INFO_FILE);
    string imageInfoBuffer;
    if (!m_backupIO.ReadFile(fileName, imageInfoBuffer)) {
        HCP_Log(ERR, MODULE_NAME) << "Read image info failed." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }

    InfoFileMgr infoManager;
    if (infoManager.AnalyzeInfoFile(imageInfoBuffer, m_imageInfo) != LayoutRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Failed to parse image info." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    imageInfo = m_imageInfo; // return image info at the same time
    
    HCP_Log(INFO, MODULE_NAME) << "Successfully read image info. image info file:" << fileName << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode BackupImageBase::CloseImage()
{
    if (UpdateImageInfo(m_imageInfo) == LayoutRetCode::FAILED) {
        HCP_Log(ERR, MODULE_NAME) << "Update image info failed. ImageId" << m_imageInfo.imageId << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }

    HCP_Log(INFO, MODULE_NAME) << "Close image, resourceId:" << m_imageInfo.resourceId
        << ", imageId:" << m_imageInfo.imageId << HCPENDLOG;
    m_imageInfo = ImageInfo {}; // clear local image information
    return LayoutRetCode::SUCCESS;
}

void BackupImageBase::WriteCallBack(LayoutRetCode status, uint64_t writtenSize, uint64_t realWrittenSize,
    void *callbackData)
{
    auto image = static_cast<BackupImageBase*>(callbackData);
    if (image == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "Write call back error." << HCPENDLOG;
        return;
    }
    image->IncreaseImageWrittenSize(writtenSize);
    image->IncreaseImageRealWrittenSize(realWrittenSize);
}

void BackupImageBase::DeleteCallBack(LayoutRetCode status, uint64_t deletedSize, void *callbackData)
{
    auto image = static_cast<BackupImageBase*>(callbackData);
    if (image == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "Write call back error." << HCPENDLOG;
        return;
    }
    image->IncreaseImageDeletedCount(deletedSize);
}

bool BackupImageBase::WriteImageInfo(const ImageInfo &imageInfo, const string &fileName)
{
    InfoFileMgr infoManager;
    string imageInfoBuffer;
    if (infoManager.CreateInfoFile(imageInfo, imageInfoBuffer) != LayoutRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Failed to format image info to string." << HCPENDLOG;
        return false;
    }

    if (!m_backupIO.WriteFile(fileName, imageInfoBuffer)) {
        HCP_Log(ERR, MODULE_NAME) << "Write image info failed." << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, MODULE_NAME) << "Successfully wrote image info, image info file:" << fileName << HCPENDLOG;
    return true;
}

string BackupImageBase::FormatImageName(const ImageInfo &imageInfo, const string &continuePart)
{
    string imageName {std:to_string(imageInfo.imageIndex)};
    imageName += (imageInfo.backupType == static_cast<int32_t>(LayoutBackupType::FULL)) ? "_full_" : "_inc_";
    imageName += imageInfo.imageId;
    imageName += continuePart;
    return imageName;
}

string BackupImageBase::GetImageFileName(const ImageInfo &imageInfo, const string &continuePart,
    const string &file)
{
    string imageName = FormatImageName(imageInfo, continuePart);
    string fileName = imageInfo.resourceId + "/" + imageName + "/" + file;
    HCP_Log(DEBUG, MODULE_NAME) << "Get image file :" << fileName << HCPENDLOG;
    return fileName;
}

int32_t BackupImageBase::GetFileNameSplitPos(const std::string localFile, int32_t availableLength)
{
    string delimiter = (localFile.find("/") != string::npos) ? "/" : "\\";
    int32_t splitPos = localFile.size() - 1;
    while (splitPos > 0
        && (splitPos = localFile.rfind(delimiter, splitPos)) != string::npos
        && splitPos > availableLength - 1) {
        splitPos--;
    }
    return splitPos;
}

bool BackupImageBase::GetRemoteFileName(string prefixPath, string localFile,
    const string &dataType, vector<string> &result)
{
    if (localFile.size() == 0) {
        return false;
    }
    hash<string> pathHashStr;
    string newPrefixPath;
    string pathHash = to_string(pathHashStr(prefixPath));
    string continuePart = prefixPath.empty() ? "" : ("_continue_" + pathHash);
    string imageName = FormatImageName(m_imageInfo, continuePart);
    string layOutPath = m_imageInfo.resourceId + "/" + imageName + "/" + dataType + "/" + m_fsId;
    layOutPath += (localFile[0] == '/') ? "" : "/";

    int32_t objNameSize = layOutPath.size() + localFile.size() + m_backupIO.GetWorkSpace().size();
    if (m_backupIO.GetMediaType() == static_cast<int32_t>(MediaType::FS) // the storage media is file system
        || objNameSize <= m_backupIO.GetS3ObjNameMaxLength()) { //  the storage media is s3
        result.emplace_back(layOutPath + localFile);
        return true;
    }

    // string::npos is maximum value for hash
    string longestContinuePart = "_continue_" + to_string(string::npos) + "/";
    int32_t availableLength = m_backupIO.GetS3ObjNameMaxLength() - layOutPath.size() -
        longestContinuePart.size() - m_backupIO.GetWorkSpace().size();
    if (availableLength <= 0) {
        return false;
    }

    int32_t splitPos = GetFileNameSplitPos(localFile, availableLength);
    if (splitPos <= 0) { // localFile is just a file or directory name, no path exists.
        splitPos = availableLength;
        newPrefixPath = (localFile.size() >= splitPos) ? localFile.substr(0, splitPos) : localFile;
        localFile = (localFile.size() >= splitPos) ? localFile.substr(splitPos) : "";
    } else {
        newPrefixPath = localFile.substr(0, splitPos + 1);
        localFile = localFile.substr(splitPos + 1);
    }

    pathHash =  prefixPath.empty() ? to_string(pathHashStr(m_fsId + newPrefixPath))
        : to_string(pathHashStr(prefixPath + newPrefixPath));

    prefixPath += prefixPath.empty() ? m_fsId + newPrefixPath : newPrefixPath;
    continuePart = "_continue_" + pathHash + "/";
    string continueImageName = layOutPath + newPrefixPath + continuePart;
    result.emplace_back(continueImageName);
    return GetRemoteFileName(prefixPath, localFile, dataType, result);
}

void BackupImageBase::UploadCallBack(LayoutRetCode status, uint64_t writtenSize,
    uint64_t realWrittenSize, void *callbackObj)
{
    auto image = static_cast<BackupImageBase*>(callbackObj);
    if (image == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "Write call back error." << HCPENDLOG;
        return;
    }
    image->IncreaseImageWrittenSize(writtenSize);
    image->IncreaseImageRealWrittenSize(realWrittenSize);
}
