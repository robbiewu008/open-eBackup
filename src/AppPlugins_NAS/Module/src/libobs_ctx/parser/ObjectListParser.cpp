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
#include <mutex>
#include "Log.h"
#include "system/System.hpp"
#include "common/CloudServiceUtils.h"
#include "ObjectListParser.h"

using namespace Module;

namespace {
constexpr auto MODULE = "OBJECT_LIST_PARSER";
constexpr int MAX_DUMP_ENTRIES_NUM = 10;
constexpr int OBJECT_LIST_ENTRY_FILED_NUM = 3;
std::map<std::string, int> OBJECT_LIST_FIELD_ID_MAP = {
    {"operateTime", 0}, {"objectOperation", 1}, {"objectName", 2}};
}

ObjectListParser::~ObjectListParser()
{
    if (m_writeFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }
    if (m_readFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::READ);
    }

    HCP_Log(DEBUG, MODULE) << "Close object list file success, file name " << m_fileName
        << " entry count " << m_entryCount << HCPENDLOG;
}

CTRL_FILE_RETCODE ObjectListParser::WriteEntry(const BucketLogInfo& bucketLogInfo)
{
    std::lock_guard<std::mutex> lk(m_lock);

    if (memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE) != 0) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write file not open, file name " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = snprintf_s(m_writeCtrlLine,
        CTRL_WRITE_LINE_SIZE,
        CTRL_WRITE_LINE_SIZE,
        "%s %d %s\n",
        bucketLogInfo.operateTime.c_str(),
        static_cast<int>(bucketLogInfo.objectOperation),
        bucketLogInfo.objectName.c_str());
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Failed to prepare buffer: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;

    ++m_entryCount;
    if (m_entryCount % MAX_DUMP_ENTRIES_NUM == 0) {
        HCP_Log(INFO, MODULE) << "Object list write entry exceeded the upper limit " << MAX_DUMP_ENTRIES_NUM
            << ", need dump file, object list file name " << m_fileName << HCPENDLOG;
        if (WriteToFile(m_writeBuffer, false) != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Write buffer to file failed, entry count " << m_entryCount << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_writeBuffer.str("");
        m_writeBuffer.clear();
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE ObjectListParser::ReadEntry(BucketLogInfo& bucketLogInfo)
{
    std::string ctlFileLine;
    std::vector<std::string> ctlFileLineSplit;
    std::lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read file not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    do {
        getline(m_readBuffer, ctlFileLine);
        if (ctlFileLine.empty()) {
            return CTRL_FILE_RETCODE::READ_EOF;
        }

        boost::algorithm::split(ctlFileLineSplit, ctlFileLine, boost::is_any_of(" "), boost::token_compress_off);
        if (ValidateEntry(ctlFileLineSplit) == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        } else {
            HCP_Log(WARN, MODULE) << "Line content " << ctlFileLine << " is invalid" << HCPENDLOG;
        }
        ctlFileLine.clear();
        ctlFileLineSplit.clear();
    } while (true);

    bucketLogInfo.operateTime = ctlFileLineSplit[OBJECT_LIST_FIELD_ID_MAP["operateTime"]];
    bucketLogInfo.objectOperation = static_cast<ObjectOperation>(
        std::stoi(ctlFileLineSplit[OBJECT_LIST_FIELD_ID_MAP["objectOperation"]]));
    if (!ConvertSpecailChar2Normal(
        ctlFileLineSplit[OBJECT_LIST_FIELD_ID_MAP["objectName"]], bucketLogInfo.objectName)) {
        HCP_Log(ERR, MODULE) << "Convert special char failed, input is "
            << ctlFileLineSplit[OBJECT_LIST_FIELD_ID_MAP["objectName"]] << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    ++m_entryCount;

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE ObjectListParser::ValidateEntry(const std::vector<std::string>& fields)
{
    if (fields.size() != OBJECT_LIST_ENTRY_FILED_NUM) {
        HCP_Log(ERR, MODULE) << "Fields size is not correct" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    for (const auto& filed : fields) {
        if (filed.empty()) {
            HCP_Log(ERR, MODULE) << "Field is empty" << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }

    std::string objectOperation = fields[OBJECT_LIST_FIELD_ID_MAP["objectOperation"]];
    if (std::find_if(objectOperation.begin(), objectOperation.end(), [](unsigned char c) {
            return !std::isdigit(c);
        }) != objectOperation.end()) {
        HCP_Log(ERR, MODULE) << "ObjectOperation contains non-numeric" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE ObjectListParser::Convert2UniqueObjectList() const
{
    std::vector<std::string> output;
    std::vector<std::string> errOutput;
    std::string execCmd = "sort -t ' ' -k 3 " + m_fileName  + " | tac | uniq -f 2 > " + m_uniqueObjectListFilePath;

    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, execCmd, { }, output, errOutput) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Run Convert2UniqueObjectList shell failed, exec cmd " << execCmd << HCPENDLOG;
        std::string msg;
        std::string errmsg;
        for (auto& it : output) {
            msg += it + " ";
        }
        HCP_Log(ERR, MODULE) << "Run Convert2UniqueObjectList shell msg: "
            << Module::WipeSensitiveDataForLog(msg) << HCPENDLOG;
        for (auto& it : errOutput) {
            errmsg += it + " ";
        }
        HCP_Log(ERR, MODULE) << "Run Convert2UniqueObjectList shell error msg: "
            << Module::WipeSensitiveDataForLog(errmsg) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}
