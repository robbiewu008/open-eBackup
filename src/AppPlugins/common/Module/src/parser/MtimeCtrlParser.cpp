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
#ifdef _AIX
#define __STDC_FORMAT_MACROS
#endif
#include "MtimeCtrlParser.h"
#include <cinttypes>
#include "Log.h"
#include "securec.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "MTIME_CTRL_PARSER";
constexpr uint32_t MTIME_CTRL_FILE_MAX_SIZE = (32 * 1024 * 1024);   /* 32 MB */
}

MtimeCtrlParser::MtimeCtrlParser(MtimeCtrlParser::Params params) : FileParser(false)
{
    m_fileName = params.m_ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_maxFileSize = MTIME_CTRL_FILE_MAX_SIZE;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = MTIMECTRL_HEADER_TITLE;
    m_header.version = MTIMECTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
}

MtimeCtrlParser::MtimeCtrlParser(std::string ctlFileName) : FileParser(false)
{
    m_fileName = ctlFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
}

MtimeCtrlParser::~MtimeCtrlParser()
{
    if (m_writeFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::WRITE);

    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE MtimeCtrlParser::OpenWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::GetHeader(MtimeCtrlParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::ValidateEntry(const vector<string> &lineContents, const string &line) const
{
    if (lineContents.empty() || line.empty()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    uint32_t lineContentsVecSize = lineContents.size();
    if (lineContentsVecSize < CTRL_FILE_OFFSET_11) {
        HCP_Log(WARN, MODULE) << "Control Entry for dir has less columns. Line: "
            << line << "File: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    uint32_t commaCount = (uint32_t) atoi(lineContents[CTRL_FILE_NUMBER_ZERO].c_str());
    if (lineContentsVecSize != (CTRL_FILE_OFFSET_11 + commaCount)) {
        HCP_Log(WARN, MODULE) << "Mtime file has Wrong Entries, Line: "
            << line << ", File: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::ReadEntry(MtimeCtrlEntry &mtimeEntry)
{
    string line {};
    vector<string> lineContents {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    do {
        getline(m_readBuffer, line);
        if (line.empty()) {
            return CTRL_FILE_RETCODE::READ_EOF;
        }
        boost::algorithm::split(lineContents, line, boost::is_any_of(","), boost::token_compress_off);
        if (ValidateEntry(lineContents, line) == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        } else if (HandleBreakLine(lineContents, line) == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        }
        line.clear();
        lineContents.clear();
    } while (true);

    HCP_Log(DEBUG, MODULE) << line << HCPENDLOG;
    try {
        TranslateEntry(lineContents, mtimeEntry);
    } catch (const std::out_of_range& e) {
        WARNLOG("out of range capture: %s", line.c_str());
    } catch (...) {
        ERRLOG("unkonwn error: %s", line.c_str());
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

/* 处理存在换行符的名称，直到获取到正常行为止 */
CTRL_FILE_RETCODE MtimeCtrlParser::HandleBreakLine(vector<string> &lineContents, std::string &ctlFileLine)
{
    std::string tmpLine {};
    std::streampos currentPositon = m_readBuffer.tellg();
    while (true) {
        tmpLine.clear();
        currentPositon = m_readBuffer.tellg();
        getline(m_readBuffer, tmpLine);
        // 读到正常的行，说明当前处理的数据为无效错误数据，把pos调整回原来的位置，返回失败
        if (IsNormalEntry(lineContents, tmpLine)) {
            m_readBuffer.seekg(currentPositon);
            return CTRL_FILE_RETCODE::INVALID_CONTENT;
        } else {
            // 说明是截断的行，拼接并增加换行符
            ctlFileLine = ctlFileLine + "\n" + tmpLine;
            // 拼接后的得到有效的数据，返回成功，否则继续
            if (IsNormalEntry(lineContents, ctlFileLine)) {
                return CTRL_FILE_RETCODE::SUCCESS;
            }
        }
    }
}

bool MtimeCtrlParser::IsNormalEntry(vector<string> &lineContents, const std::string& ctlFileLine) const
{
    boost::algorithm::split(lineContents, ctlFileLine, boost::is_any_of(","), boost::token_compress_off);
    return ValidateEntry(lineContents, ctlFileLine) == CTRL_FILE_RETCODE::SUCCESS;
}

void MtimeCtrlParser::TranslateEntry(vector<string> &lineContents, MtimeCtrlEntry &mtimeEntry)
{
    uint32_t offset = CTRL_FILE_OFFSET_1;
    uint32_t totCommaCnt = (uint32_t) stoul(lineContents[CTRL_FILE_OFFSET_0]);

    mtimeEntry.m_absPath = ParserUtils::ConstructStringName(offset, totCommaCnt, lineContents);
    mtimeEntry.m_mode = (uint32_t) stoul(lineContents[offset++]);
    mtimeEntry.m_attr = (uint32_t) stoul(lineContents[offset++]);
    mtimeEntry.m_uid = (uint32_t) stoul(lineContents[offset++]);
    mtimeEntry.m_gid = (uint32_t) stoul(lineContents[offset++]);
    mtimeEntry.m_ctime = (uint64_t) stoull(lineContents[offset++]);
    mtimeEntry.m_atime = (uint64_t) stoull(lineContents[offset++]);
    mtimeEntry.m_mtime = (uint64_t) stoull(lineContents[offset++]);
    mtimeEntry.m_btime = (uint64_t) stoull(lineContents[offset++]);
    mtimeEntry.m_subDirsCnt = (uint32_t) stoul(lineContents[offset]);
}

CTRL_FILE_RETCODE MtimeCtrlParser::WriteEntry(MtimeCtrlEntry &mtimeEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = ParserUtils::GetCommaCountOfString(mtimeEntry.m_absPath.c_str());

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as control file is not open: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    int ret = memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE);
    if (ret != 0) {
        ERRLOG("failed to memset m_writeCtrlLine %p, ret %d", m_writeCtrlLine, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                        "%" PRIu32 ",%s,%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu64 ""
                        ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu32"\n",
                        commaCount,
                        mtimeEntry.m_absPath.c_str(),
                        mtimeEntry.m_mode,
                        mtimeEntry.m_attr,
                        mtimeEntry.m_uid,
                        mtimeEntry.m_gid,
                        mtimeEntry.m_ctime,
                        mtimeEntry.m_atime,
                        mtimeEntry.m_mtime,
                        mtimeEntry.m_btime,
                        mtimeEntry.m_subDirsCnt);
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Failed to write buffer: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;

    m_entries++;
    m_header.stats.noOfDirs++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}
#ifndef WIN32
void MtimeCtrlParser::PrintEntry(MtimeCtrlEntry& mtimeEntry)
{
    DBGLOG("write to mtime ctrl - %s", m_fileName.c_str());
    DBGLOG("write mtimeEntry - m_absPath: %s, ctime: %llu, atime: %llu, mtime: %llu, btime: %llu,"
        "uid: %lu, gid: %lu, attr: %lu, mode: %lu, subDirsCnt: %lu",
        mtimeEntry.m_absPath.c_str(), mtimeEntry.m_ctime, mtimeEntry.m_atime,
        mtimeEntry.m_mtime, mtimeEntry.m_btime, mtimeEntry.m_uid, mtimeEntry.m_gid,
        mtimeEntry.m_attr,
        mtimeEntry.m_mode,
        mtimeEntry.m_subDirsCnt);
}
#endif
CTRL_FILE_RETCODE MtimeCtrlParser::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readBuffer, cltHeaderLine)) {
            HCP_Log(ERR, MODULE) << "ReadHeader (getline) failed: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed, incomplete header: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed: " << m_fileName << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (FillHeader(headerLine, cltHeaderLineSplit, cltHeaderLine) != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine); /* To skip the blank line after header */
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TITLE)) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_HEADER_VERSION)) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TIMESTAMP)) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKID)) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKTYPE)) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSERVER)) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSHARE)) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL)) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL_VERSION)) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_DIR_MTIME_COUNT)) {
        m_header.stats.noOfDirs = (uint64_t) atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MtimeCtrlParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }
    headerBuffer << "\n";
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, !CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header for Bkup Mtime Ctrl file Failed: " << m_fileName;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string MtimeCtrlParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TITLE):
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_HEADER_VERSION):
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TIMESTAMP):
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKID):
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKTYPE):
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSERVER):
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSHARE):
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL):
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL_VERSION):
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_DIR_MTIME_COUNT):
            ctlHeaderLine = "DirectoryMtimeSetCount:" + to_string(m_header.stats.noOfDirs) + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE MtimeCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), MTIMECTRL_HEADER_TITLE.c_str()) != 0) ||
        (strcmp(m_header.version.c_str(), MTIMECTRL_HEADER_VERSION.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.protoVersion.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint32_t MtimeCtrlParser::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

std::string MtimeCtrlParser::GetCtrlFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}
