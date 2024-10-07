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
#include "BackupImageSet.h"

#include <string>

#include "InfoFileMgr.h"

using namespace std;
using namespace Module;

namespace {
    const string MODULE_NAME = "BackupImageSet";
    const string IMAGESET_INFO_NAME = "imageset.info";
    const int IMAGEID_INDEX = 2;
}

BackupImageSet::BackupImageSet(BackupIOInterface &backupIO, const AggregationInfo &aggregateInfo)
    : m_backupIO(backupIO)
{
    m_imageSetInfo.aggregateInfo = aggregateInfo;
}

bool BackupImageSet::CheckIfImageSetExists(const std::string &imageSetId)
{
    if (imageSetId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "imageSetId is empty." << HCPENDLOG;
        return false;
    }
    string fileName = imageSetId + "/" + IMAGESET_INFO_NAME;
    return m_backupIO.IsFileExist(fileName);
}

LayoutRetCode BackupImageSet::CreateBackupSet(const string &imageSetId)
{
    HCP_Log(DEBUG, MODULE_NAME) << "Begin to create image set." << HCPENDLOG;
    if (imageSetId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "ImageSetId is empty when create image set." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    ImageSetInfo writeInfo = m_imageSetInfo;
    writeInfo.creationDatetime = to_string(time(nullptr));
    writeInfo.imageSetId = imageSetId;
    writeInfo.jsonVersion = JSONCPP_VERSION_STRING;
    if (!WriteImageSetInfo(writeInfo)) {
        HCP_Log(ERR, MODULE_NAME) << "Failed to create image set." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    m_imageSetInfo = writeInfo;
    HCP_Log(INFO, MODULE_NAME) << "Successfully created image set."
                               << ", imageSetId:" << imageSetId
                               << ", jsonVersion:" << m_imageSetInfo.jsonVersion
                               << ", aggregated:" << m_imageSetInfo.aggregateInfo.aggregated
                               << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

// before update image set info, please get the image set info firstly,
LayoutRetCode BackupImageSet::UpdateBackupSetInfo(const ImageSetInfo &imageSetInfo)
{
    if (imageSetInfo.imageSetId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "imageSetId is empty when update image set info." << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    string fileName = imageSetInfo.imageSetId + "/" + IMAGESET_INFO_NAME;
    if (!m_backupIO.IsFileExist(fileName)) {
        HCP_Log(ERR, MODULE_NAME) << "Image set is not exist. Info file:" << fileName << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }

    if (!WriteImageSetInfo(imageSetInfo)) {
        HCP_Log(ERR, MODULE_NAME) << "Write image set info failed. imageSetId:" << imageSetInfo.imageSetId << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    m_imageSetInfo = imageSetInfo;
    HCP_Log(DEBUG, MODULE_NAME) << "Succeeded to update image set info. imageSetId: "
        << imageSetInfo.imageSetId << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

string BackupImageSet::GetBackupSetID() const
{
    HCP_Log(DEBUG, MODULE_NAME) << "Get backupSetID :" <<  m_imageSetInfo.imageSetId << HCPENDLOG;
    return m_imageSetInfo.imageSetId;
}

void BackupImageSet::GetBackupSetInfo(ImageSetInfo &imageSetInfo) const
{
    imageSetInfo = m_imageSetInfo;
}

bool BackupImageSet::WriteImageSetInfo(const ImageSetInfo &imageSetInfo)
{
    InfoFileMgr infoManager;
    string imageSetInfoBuffer;
    if (infoManager.CreateInfoFile(imageSetInfo, imageSetInfoBuffer) != LayoutRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Failed to format image set info to string." << HCPENDLOG;
        return false;
    }
    string fileName = imageSetInfo.imageSetId + "/" + IMAGESET_INFO_NAME;
    if (!m_backupIO.WriteFile(fileName, imageSetInfoBuffer)) {
        HCP_Log(ERR, MODULE_NAME) << "Write image set info failed." << imageSetInfo.imageSetId << HCPENDLOG;
        return false;
    }
    HCP_Log(INFO, MODULE_NAME) << "Successfully wrote image set info. imageSetId:" <<
        imageSetInfo.imageSetId << ", imageCount:" << imageSetInfo.imageCount << HCPENDLOG;
    return true;
}

LayoutRetCode BackupImageSet::RemoveBackupSet()
{
    CallBackHandle handle;
    string imageSet = m_imageSetInfo.imageSetId + "/";
    if (m_imageSetInfo.imageCount != 0 || !m_backupIO.DeleteAll(imageSet, handle)) {
        HCP_Log(ERR, MODULE_NAME) << "Delete imageset failed, imageSetID:" << m_imageSetInfo.imageSetId <<
            ", imageCount" << m_imageSetInfo.imageCount << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    HCP_Log(INFO, MODULE_NAME) << "Succeeded to remove image set: " << imageSet << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode BackupImageSet::ListBackupImages(vector<string> &imageList)
{
    string directoryName = m_imageSetInfo.imageSetId + "/";
    vector<string> imageNameList;
    if (!m_backupIO.GetDirectoryList(directoryName, imageNameList)) {
        HCP_Log(ERR, MODULE_NAME) << "List backup images failed. " << directoryName << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    set<string> uniqueSet;
    for_each(imageNameList.begin(), imageNameList.end(), [&](string &imageName) {
        vector<string> elementList;
        boost::split(elementList, imageName, boost::is_any_of("_"));
        if (elementList.size() > IMAGEID_INDEX) {
            uniqueSet.insert(elementList[IMAGEID_INDEX]);
        }
    });
    for_each(uniqueSet.begin(), uniqueSet.end(), [&](const string &imageName) {imageList.push_back(imageName);});
    
    HCP_Log(DEBUG, MODULE_NAME) << "Successfully List backup images, size:" << imageList.size() << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}

// Read backup set info from s3 or file system
LayoutRetCode BackupImageSet::ReadBackupSetInfo(const string &imageSetId)
{
    HCP_Log(DEBUG, MODULE_NAME) << "Begin to read image set info." << imageSetId << HCPENDLOG;
    string imageSetInfoBuffer;
    string fileName = imageSetId + "/" + IMAGESET_INFO_NAME;
    if (!m_backupIO.ReadFile(fileName, imageSetInfoBuffer)) {
        HCP_Log(ERR, MODULE_NAME) << "Read image set info failed." << fileName << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    InfoFileMgr infoManager;
    if (infoManager.AnalyzeInfoFile(imageSetInfoBuffer, m_imageSetInfo) != LayoutRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Failed to parse image set info." << fileName << HCPENDLOG;
        return LayoutRetCode::FAILED;
    }
    HCP_Log(DEBUG, MODULE_NAME) << "Successfully read image set info." << fileName << HCPENDLOG;
    return LayoutRetCode::SUCCESS;
}
