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
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <ostream>
#include <chrono>

#include "BackupFailureRecorder.h"
#include "log/Log.h"

#ifdef WIN32
#include <filesystem>
#else
#include <boost/filesystem.hpp>
#endif

namespace {
    constexpr auto DATE_BUF_MAX = 50;
    const std::size_t RECORDER_FILE_NUMBER_LIMIT = 100;
    const std::string RECORDER_FILE_SUFFIX = "_failure.csv";
    const std::string FAILED_FILE_SET = "recordFailedFileSet";
#ifdef WIN32
    const std::string PATH_SEPARATOR = "\\";
#else
    const std::string PATH_SEPARATOR = "/";
#endif
}

#ifdef WIN32
namespace fs = std::filesystem;
#else
namespace fs = boost::filesystem;
#endif

namespace Module {

    size_t BackupFailureRecorder::m_failureRecordsFileMax = 10 * 1024 * 1024; /* 10M */

BackupFailureRecorder::BackupFailureRecorder(
    const std::string& outputDirRootPath,
    const std::string& jobID,
    const std::string& subTaskID,
    size_t bufferMax,
    uint64_t recordMax)
    : m_outputDirRootPath(outputDirRootPath),
    m_jobID(jobID),
    m_subTaskID(subTaskID)
{
    m_filepath = GetJobRecordRootPath() + PATH_SEPARATOR + m_subTaskID + "_failure.csv";
    INFOLOG("backup failure recorder csv path: %s", m_filepath.c_str());
    m_buffer.reserve(bufferMax); /* prealloc fixed sized buffer */
    m_numWrited = 0;
}

BackupFailureRecorder::~BackupFailureRecorder()
{
    FlushInner();
}

void BackupFailureRecorder::RecordFailure(const std::string& path, const std::string& reason)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    using namespace std::chrono;
    uint64_t timestamp = static_cast<uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
    m_buffer.push_back(BackupFailureRecord{
        m_subTaskID,
        timestamp,
        path,
        reason
    });
    /* reached limit, block the thread and flush */
    if (m_buffer.size() < m_buffer.capacity()) {
        return;
    }
    if (!FlushInner()) {
        ERRLOG("flush failed");
    }
}

uint64_t BackupFailureRecorder::NumWrited() const
{
    return m_numWrited;
}

uint64_t BackupFailureRecorder::NumWriteFailed() const
{
    return m_numWriteFailed;
}

/* invoked by outside manually, used to flush data and do statistics */
bool BackupFailureRecorder::Flush()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    return FlushInner();
}

/* flush data from buffer to file and clean the buffer,
 * Blocking, used as private inner method, no lock require
 */
bool BackupFailureRecorder::FlushInner()
{
    /* private method, must be invoked by RecordFailure(), not need to use lock */
    if (m_buffer.empty()) {
        return true;
    }
    std::string jobRecordRootPath = GetJobRecordRootPath();
    DBGLOG("failure record triggered flush, job record root path: %s", jobRecordRootPath.c_str());
    try {
        if (!fs::exists(jobRecordRootPath) &&
            !fs::create_directories(jobRecordRootPath)) {
            ERRLOG("failure record root path create failed: %s, errno: %d", jobRecordRootPath.c_str(), errno);
            m_numWriteFailed += m_buffer.size();
            m_buffer.clear();
            return false;
        }
        std::ofstream outfile(m_filepath, std::ios::app);
        if (!outfile.is_open()) {
            ERRLOG("failure record csv file open failed: %s, errno: %d", m_filepath.c_str(), errno);
            m_numWriteFailed += m_buffer.size();
            m_buffer.clear();
            return false;
        }

        while (!m_buffer.empty()) {
            if (m_numWrited >= m_recordMax) {
                WARNLOG("num of failure records reached limit %llu, break", m_recordMax);
                outfile.close();
                m_numWriteFailed += m_buffer.size();
                m_buffer.clear();
                return false;
            }
            const BackupFailureRecord& record = m_buffer.back();
            outfile << RecordToString(record) << std::endl;
            m_numWrited++;
            m_buffer.pop_back();
        }

        outfile.close();
        m_buffer.clear();
    } catch (const std::exception& e) {
        ERRLOG("flush failure recorder caught exception: %s", e.what());
        m_numWriteFailed += m_buffer.size();
        m_buffer.clear();
        return false;
    }
    return true;
}

/* each line conclude path, datetime, reason. subTaskID  */
std::string BackupFailureRecorder::RecordToString(const BackupFailureRecord& record) const
{
    const std::string splitor = ",";
    const std::string quotation = "\"";
    std::string path = EscapePath(record.path);
    std::string dateStr = SecTimestampToDateStr(record.timestamp);
    std::stringstream ss;
    /* output as csv format using  */
    ss  << quotation << dateStr << quotation << splitor
        << quotation << record.subTaskID << quotation << splitor
        << quotation << path << quotation << splitor
        << quotation << record.reason << quotation;
    return ss.str();
}

std::string BackupFailureRecorder::EscapePath(const std::string& path) const
{
    std::string res;
    for (const char ch: path) {
        switch (ch) {
            case '"':
            case '\\':
                res.push_back('\\');
                res.push_back(ch);
                break;
            default: {
                res.push_back(ch);
                break;
            }
        }
    }
    return res;
}

std::string BackupFailureRecorder::SecTimestampToDateStr(uint64_t timestampSec) const
{
    auto millsec = std::chrono::seconds(timestampSec);
    auto tp = std::chrono::time_point<
    std::chrono::system_clock, std::chrono::seconds>(millsec);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);
    char strtime[DATE_BUF_MAX] = "";
    if (std::strftime(strtime, sizeof(strtime), "%Y-%m-%d %H:%M:%S", now) > 0) {
        return std::string(strtime);
    }
    return std::to_string(timestampSec);
}

std::string BackupFailureRecorder::GetJobRecordRootPath() const
{
    std::string jobRecordRootPath = m_outputDirRootPath + PATH_SEPARATOR + m_jobID;
    return jobRecordRootPath;
}

std::vector<std::string> BackupFailureRecorder::GetFileListInDirectory(const std::string& path)
{
    std::vector<std::string> result;
    try {
        if (!fs::exists(path)) {
            ERRLOG("get file list in directory failed, %s not exists", path.c_str());
            return result;
        }
        for (const auto& dirIterator: fs::directory_iterator(path)) {
            if (!fs::is_regular_file(dirIterator.status())) {
                continue;
            }
            // filter all file has suffix _failure.csv
            const std::string filePath = dirIterator.path().string();
            if (std::equal(RECORDER_FILE_SUFFIX.rbegin(), RECORDER_FILE_SUFFIX.rend(), filePath.rbegin())) {
                result.push_back(filePath);
            }
        }
    } catch (const std::exception &e) {
        ERRLOG("get file list in directory %s, caught exception %s", path.c_str(), e.what());
        return result;
    }
    return result;
}

void BackupFailureRecorder::DeleteExcessFiles(const std::string& directoryPath)
{
    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            ERRLOG("The directoryPath %s have error",
                directoryPath.c_str());
            return;
        }
    } catch (const std::exception& e) {
        ERRLOG("the directoryPath： %s caught exception: %s", directoryPath.c_str(), e.what());
        return;
    }
    uint32_t fileCountInDirectory = CountFiles(directoryPath);
    if (fileCountInDirectory >= RECORDER_FILE_NUMBER_LIMIT) {
        INFOLOG("The number file : %d of files up to the limit: %d",
            fileCountInDirectory, RECORDER_FILE_NUMBER_LIMIT);
        DeleteExcessFilesInner(directoryPath);
    }
    return;
}

void BackupFailureRecorder::DeleteExcessFilesInner(const std::string& directoryPath)
{
    try {
        std::vector<FileInfo> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                FileInfo file;
                file.filePath = entry.path().string();
#ifdef WIN32
                auto file_time = std::filesystem::last_write_time(directoryPath);
                auto sys_epoch = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(0);
                auto file_time_since_epoch = file_time - std::filesystem::file_time_type::clock::now();
                auto file_time_in_system_clock = std::chrono::system_clock::from_time_t(0) + file_time_since_epoch;
                file.creationTime = std::chrono::system_clock::to_time_t(file_time_in_system_clock);
#else
                file.creationTime = fs::last_write_time(entry.path());
#endif
                files.push_back(file);
            }
        }

        if (files.empty()) {
            INFOLOG("The directoryPath %s have no files",
                directoryPath.c_str());
            return;
        }
        std::sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
            return a.creationTime < b.creationTime;
        });

        const auto& fileToDelete = files.front().filePath;
        if (fs::remove(fileToDelete)) {
            INFOLOG("Successfully deleted the earliest created file: %s", fileToDelete);
        } else {
            ERRLOG("Failed to delete the file!");
        }
    } catch (const std::exception& e) {
        ERRLOG("Delete subtask failure record in directory %s, caught exception %s",
            directoryPath.c_str(), e.what());
    }
}
uint32_t BackupFailureRecorder::CountFiles(const std::string& directoryPath)
{
    std::size_t fileCount = 0;
    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                 ++fileCount;
            }
        }
        INFOLOG("The number of files is %d", fileCount);
    } catch (const std::exception& e) {
        ERRLOG("Count subtask failure record in directory %s, caught exception %s",
            directoryPath.c_str(), e.what());
    }
    return fileCount;
}

bool BackupFailureRecorder::Merge(
    const std::string& finaloutputDirRootPath, const std::string& jobID, const std::string& srcoutputDirRootPath)
{
    std::string jobRecordRootPath = srcoutputDirRootPath + PATH_SEPARATOR + jobID;
    INFOLOG("jobRecordRootPath: %s, finaloutputDirRootPath : %s",
        jobRecordRootPath.c_str(), finaloutputDirRootPath.c_str());
    std::vector<std::string> fileList = GetFileListInDirectory(jobRecordRootPath);
    if (fileList.empty()) {
        INFOLOG("no failure csv file found to be merged, job record root path: %s",
            jobRecordRootPath.c_str());
        return true;
    }
    if (!MergeInner(finaloutputDirRootPath, jobID, jobRecordRootPath, fileList)) {
        return false;
    }
    // 清除子目录后面剩下的空目录
    try {
        fs::remove_all(jobRecordRootPath);
    } catch (const std::exception& e) {
        ERRLOG("remove jobRecordRootPath： %s caught exception: %s", jobRecordRootPath.c_str(), e.what());
    }
    return true;
}

bool BackupFailureRecorder::MergeInner(const std::string& outputDirRootPath, const std::string& jobID,
    const std::string& jobRecordRootPath, const std::vector<std::string>& fileList)
{
    std::string recordFailedFilePath =  outputDirRootPath + PATH_SEPARATOR + FAILED_FILE_SET;
    try {
        if (fs::exists(recordFailedFilePath)) {
            DeleteExcessFiles(recordFailedFilePath);
        } else {
            fs::create_directories(recordFailedFilePath);
        }
        std::string outBundleFilePath = recordFailedFilePath + PATH_SEPARATOR + jobID + "_bundle.csv";
        std::ofstream outFile(outBundleFilePath, std::ios::app);
        if (!outFile.is_open()) {
            ERRLOG("Create main job failure recorde file failed, file name: %s", outBundleFilePath.c_str());
            ClearSubTaskRecordFile(jobRecordRootPath);
            return false;
        }
        size_t totalFileSize = GetFileSize(outFile);
        size_t fileSize = 0;
        for (const std::string& subFile : fileList) {
            std::ifstream inFile(subFile, std::ios::in);
            if (!inFile.is_open()) {
                ERRLOG("Open subjob failure recorde file failed, file: %s", subFile.c_str());
                continue;
            }
            fileSize = GetFileSize(inFile);
            INFOLOG("totalFileSize: %d , fileSize: %d, m_failureRecordsFileMax: %d", totalFileSize, fileSize, m_failureRecordsFileMax);
            if (fileSize + totalFileSize > m_failureRecordsFileMax)  {
                inFile.close();
                fs::remove(subFile);
                continue;
            }
            outFile << inFile.rdbuf();
            totalFileSize += fileSize;
            inFile.close();
            /* remove in time to prevent merge failing due to insufficient disk space */
            INFOLOG("%s merged, remove now", subFile.c_str());
            fs::remove(subFile);
        }
        outFile.close();
        ClearSubTaskRecordFile(jobRecordRootPath);
    } catch (const std::exception& e) {
        ERRLOG("process merge failure record in %s caught exception: %s", jobRecordRootPath.c_str(), e.what());
        ClearSubTaskRecordFile(jobRecordRootPath);
        return false;
    }
    return true;
}

uint64_t BackupFailureRecorder::GetFileSize(std::ifstream& infile)
{
    infile.seekg(0, infile.end);
    uint64_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    return length;
}

uint64_t BackupFailureRecorder::GetFileSize(std::ofstream& outfile)
{
    outfile.seekp(0, outfile.end);
    uint64_t length = outfile.tellp();
    return length;
}

bool BackupFailureRecorder::ClearSubTaskRecordFile(const std::string& jobRecordRootPath)
{
    try {
        if (!fs::exists(jobRecordRootPath)) {
            ERRLOG("clear subtask failure records failed, job root dir %s not exists",
                jobRecordRootPath.c_str());
            return false;
        }
        for (const auto& dirIterator: fs::directory_iterator(jobRecordRootPath)) {
            // filter all file has suffix _failure.csv
            const std::string filePath = dirIterator.path().string();
            if (std::equal(RECORDER_FILE_SUFFIX.rbegin(), RECORDER_FILE_SUFFIX.rend(), filePath.rbegin())) {
                DBGLOG("remove subtask failure record %s", filePath.c_str());
                fs::remove(filePath);
            }
        }
    } catch (const std::exception& e) {
        ERRLOG("clear subtask failure record in directory %s, caught exception %s",
            jobRecordRootPath.c_str(), e.what());
        return false;
    }
    return true;
}

}