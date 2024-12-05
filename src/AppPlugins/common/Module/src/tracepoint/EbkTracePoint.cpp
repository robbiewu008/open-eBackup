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
#include "EbkTracePoint.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include "system/System.hpp"
#include "config_reader/ConfigIniReaderImpl.h"

using namespace std;

namespace {
const std::string MODULE_NAME = "EbkTracePoint";
const size_t DEACTIVATE_PARAMS_NUM = 2;
const size_t ACTIVATE_PARAMS_NUM = 4;
const size_t TP_NAME_LOC = 1;
const size_t TP_ALIVE_NUM_LOC = 2;
const size_t TP_BLOCK_TIME_LOC = 3;
const int STR_TO_INT_BASE = 10;
}

EbkTracePoint::EbkTracePoint()
{
    const std::string cfgPath = Module::ConfigReader::getString(
        std::string("MicroService"), std::string("ProcessRootPath")) + "/conf/tracepoint.ini";
    HCP_Log(DEBUG, "TracePoint") << "ParseConfigFile from " << cfgPath << HCPENDLOG;
    ParseConfigFile(cfgPath);
}

EbkTracePoint& EbkTracePoint::GetInstance()
{
    static EbkTracePoint instance;
    return instance;
}

// 注册TracePoint到hashmap中
int32_t EbkTracePoint::RegTP(const std::string& name, const std::string& desc, TPType type, FnTpCommon fnHook,
                             uint32_t pid)
{
    int32_t activeState = TP_STAT_DEACTIVE;
    uint32_t aliveNum = 0;
    std::string param;
    uint32_t blockTimes = 0;
    if (name.empty() || desc.empty()) {
        HCP_Log(ERR, "TracePoint") << "Parameter is null, trace point register failed" << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }

    if (type == TP_TYPE_CALLBACK && fnHook == nullptr) {
        HCP_Log(ERR, "TracePoint") << "No callback function provided: " << name << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }

    auto iter = m_initActiveTPs.find(name);
    if (iter != m_initActiveTPs.end()) {
        if (iter->second.alive) {
            activeState = TP_STAT_ACTIVE;
        } else {
            HCP_Log(INFO, "TracePoint") << "TP " << name << " has been deactivate by config file." << HCPENDLOG;
        }
        aliveNum = iter->second.aliveNum;
        param = iter->second.userParam;
        blockTimes = iter->second.blockTimes;
    }

    // 初始化TracePoint
    auto tpItem = std::make_shared<TP>();
    tpItem->szName = name;
    tpItem->szDesc = desc;
    tpItem->uiPid = pid;
    tpItem->iActive = activeState;

    tpItem->type = type;
    tpItem->aliveNum = aliveNum;
    tpItem->timeCalled = 0;
    tpItem->fnHook = fnHook;
    tpItem->szParam = param;
    tpItem->blockTimes = blockTimes;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_hashMap.count(name) == 0) {
            // 将初始化TracePoint加入hashmap
            m_hashMap.insert(std::make_pair(name, tpItem));
        } else {
            // 若已存在，更新
            m_hashMap[name] = tpItem;
        }
    }
    HCP_Log(INFO, "TracePoint") << "TP " << name << " has been registered successfully" << HCPENDLOG;
    return TRACE_POINT_SUCCESS;
}

// 注销TracePoint
int32_t EbkTracePoint::DeleteTP(const std::string& name)
{
    // 入参检查
    if (name.empty()) {
        HCP_Log(ERR, "TracePoint") << "Parameter is null" << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }

    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_hashMap.erase(name) == 0) {
            HCP_Log(ERR, "TracePoint") << "TP " << name << " is not registered" << HCPENDLOG;
            return TRACE_POINT_ERROR;
        };
    }
    HCP_Log(INFO, "TracePoint") << "TP " << name << " has been deleted." << HCPENDLOG;
    return TRACE_POINT_SUCCESS;
}

int32_t EbkTracePoint::GetTP(const std::string& name, std::shared_ptr<TP>& tracepoint)
{
    if (name.empty()) {
        HCP_Log(ERR, "TracePoint") << "Parameter is null" << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }

    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_hashMap.count(name) == 0) {
            HCP_Log(ERR, "TracePoint") << "name is:" << name << HCPENDLOG;
            HCP_Log(ERR, "TracePoint") << "tp count is:" << m_hashMap.size() << HCPENDLOG;
            return TRACE_POINT_ERROR;
        }
        tracepoint = m_hashMap[name];
    }
    return TRACE_POINT_SUCCESS;
}

int32_t EbkTracePoint::ParseLine(std::string line)
{
    if (line.empty()) {
        return TRACE_POINT_ERROR;
    }
    size_t pos = line.find_first_not_of(" ");
    if (pos != std::string::npos) {
        const char c = line[pos];
        if (c == '#' || c == ';') {
            return TRACE_POINT_ERROR;
        }
    }
    TPInit tracePoint;
    auto params = Split(line, " ");
    if (params.empty() || params.size() < DEACTIVATE_PARAMS_NUM || (params[0] != "0" && params[0] != "1")) {
        HCP_Log(ERR, "TracePoint") << "Config file parameter format wrong!" << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }
    if (params[0] == "0") {
        tracePoint.alive = false;
    }
    if (params.size() < ACTIVATE_PARAMS_NUM) {
        HCP_Log(ERR, "TracePoint") << "Config file parameter number wrong!" << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }
    tracePoint.tpName = params[TP_NAME_LOC];
    char* strPtr = nullptr;
    tracePoint.aliveNum = strtol(params[TP_ALIVE_NUM_LOC].c_str(), &strPtr, STR_TO_INT_BASE);
    tracePoint.blockTimes = strtol(params[TP_BLOCK_TIME_LOC].c_str(), &strPtr, STR_TO_INT_BASE);
    tracePoint.userParam = ((params.size() > ACTIVATE_PARAMS_NUM) ? params[ACTIVATE_PARAMS_NUM] : params[TP_NAME_LOC]);
    m_initActiveTPs[tracePoint.tpName] = tracePoint;
    HCP_Log(INFO, "TracePoint") << "TP " << params[TP_NAME_LOC] << " has been load from config file successfully"
                                << HCPENDLOG;
    return TRACE_POINT_SUCCESS;
}

void EbkTracePoint::ParseConfigFile(const std::string& filename)
{
    std::string str;
    std::ifstream fd(filename);
    if (fd.is_open()) {
        while (!fd.eof()) {
            getline(fd, str);
            ParseLine(str);
        }
    } else {
        HCP_Log(ERR, "TracePoint") << "Open config file failed: " << filename << HCPENDLOG;
        return;
    }
    fd.close();
}

int32_t EbkTracePoint::DeactiveTPImpl(const std::string& name)
{
    std::shared_ptr<TP> tp = nullptr;
    if (name.empty()) {
        HCP_Log(ERR, "TracePoint") << "Parameter is null" << HCPENDLOG;
        return TRACE_POINT_PARAM_ERR;
    }

    if (GetTP(name, tp) != 0) {
        HCP_Log(ERR, "TracePoint") << "Get tracepoint " << name << " failed" << HCPENDLOG;
        return TRACE_POINT_PARAM_ERR;
    }
    
    tp->iActive = TP_STAT_DEACTIVE;
    tp->aliveNum = 0;
    tp->timeCalled = 0;
    tp->szParam = "";
    return TRACE_POINT_SUCCESS;
}

int32_t EbkTracePoint::ActiveTPImpl(const std::string &name, uint32_t aliveNum, const std::string &userParam,
                                    uint32_t blockTimes, bool ignoreConfFile)
{
    if (!ignoreConfFile) {
        auto iter = m_initActiveTPs.find(name);
        if (iter != m_initActiveTPs.end()) {
            HCP_Log(INFO, "TracePoint") << "TP" << name << "has been setup in config file, skip." << HCPENDLOG;
            return TRACE_POINT_SUCCESS;
        }
    }
    std::shared_ptr<TP> tp = nullptr;
    if (name.empty()) {
        HCP_Log(ERR, "TracePoint") << "Parameter is null" << HCPENDLOG;
        return TRACE_POINT_PARAM_ERR;
    }

    if (GetTP(name, tp) != 0) {
        HCP_Log(ERR, "TracePoint") << "Get tracepoint " << name << " failed" << HCPENDLOG;
        return TRACE_POINT_PARAM_ERR;
    }

    tp->iActive = TP_STAT_ACTIVE;
    tp->aliveNum = aliveNum;
    tp->szParam = userParam;
    tp->timeCalled = 0;
    tp->blockTimes = blockTimes;
    HCP_Log(INFO, "TracePoint") << "Activate " << name << " success!" << HCPENDLOG;
    return TRACE_POINT_SUCCESS;
}

int32_t EbkTracePoint::ExportTP(std::string& exportFilePath)
{
    exportFilePath = Module::ConfigReader::getString(
    std::string("MicroService"), std::string("ProcessRootPath")) + "/tmp/TracePointExport.csv";
    std::vector<std::string> typeString {"Callback", "Reset"};
    ofstream outFile;
    outFile.open(exportFilePath.c_str(), ios::out);
    if (!outFile.is_open()) {
        HCP_Log(ERR, "TracePoint") << "Export trace point failed, open file error: " << exportFilePath << HCPENDLOG;
        return TRACE_POINT_ERROR;
    }
    outFile << "Name" << ',' << "Description" << ',' << "IsAlive" << ',' << "Type" << ',' << "AliveNum" << ','
            << "TimeCalled" << ',' << "BlockTime" << ',' << "UserParam" << ',' << "UI_Pid" << endl;
    for (auto& item : m_hashMap) {
        outFile << item.second->szName << ',' << item.second->szDesc << ',' << item.second->iActive << ','
                << typeString[item.second->type] << ',' << item.second->aliveNum << ',' << item.second->timeCalled
                << ',' << item.second->blockTimes << ',' << item.second->szParam << ',' << item.second->uiPid << endl;
    }
    outFile.close();
    return TRACE_POINT_SUCCESS;
}

vector<string> EbkTracePoint::Split(const string& str, const string& delim)
{
    vector<string> res;
    if (str.empty()) {
        return res;
    }
    char *strs = new char[str.length() + 1];
    int ret = strcpy_s(strs, str.length() + 1, str.c_str());
    if (ret != 0) {
        HCP_Log(ERR, "TracePoint") << "Copy string error: " << ret << HCPENDLOG;
        return res;
    }

    char *d = new char[delim.length() + 1];
    ret = strcpy_s(d, delim.length() + 1, delim.c_str());
    if (ret != 0) {
        HCP_Log(ERR, "TracePoint") << "Copy string error: " << ret << HCPENDLOG;
        return res;
    }

    char *p = strtok(strs, d);
    while (p) {
        string s = p;
        res.push_back(s);
        p = strtok(nullptr, d);
    }
    delete[] strs;
    delete[] d;

    return res;
}

void NullHook(std::string userparam, ...)
{}

// make all params become -1
// e.g. SsizeReturnFailHook("", 2, &i, &j)
void SsizeReturnFailHook(std::string userparam, size_t paramNum, ...)
{
    va_list argList;
    va_start(argList, paramNum);
    for (size_t i = 0; i < paramNum; i++) {
        ssize_t* dataLen = va_arg(argList, ssize_t*);
        *dataLen = -1;
    }
    va_end(argList);
}

// make all params become -1
void IntReturnFailHook(std::string userparam, size_t paramNum, ...)
{
    va_list argList;
    va_start(argList, paramNum);
    for (size_t i = 0; i < paramNum; i++) {
        int* dataLen = va_arg(argList, int*);
        *dataLen = -1;
    }
    va_end(argList);
}

// make all params become false
void BoolReturnFailHook(std::string userparam, size_t paramNum, ...)
{
    va_list argList;
    va_start(argList, paramNum);
    for (size_t i = 0; i < paramNum; i++) {
        bool* dataLen = va_arg(argList, bool*);
        *dataLen = false;
    }
    va_end(argList);
}

void PauseThreadHook(std::string userparam, uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

std::vector<std::string> stringSplit(const std::string& str, char delim) {
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

void ParseLineData(std::string str, const std::string& tpName, std::string &data)
{
    HCP_Log(DEBUG, "TracePointCallback") << "Parse line data begin!" << HCPENDLOG;
	if (str.empty()) {
		HCP_Log(ERR, "TracePointCallback") << "getline wrong,str=" << str << HCPENDLOG;
		return;
	}
	size_t pos = str.find_first_not_of(" ");
	if (pos != std::string::npos) {
		const char c = str[pos];
		if (c == '#' || c == ';') {
			return;
		}
	}
	auto params = stringSplit(str, ' ');
	if (params.empty() || params.size() < DEACTIVATE_PARAMS_NUM || (params[0] != "0" && params[0] != "1")) {
		HCP_Log(ERR, "TracePointCallback") << "Config file parameter format wrong!" << HCPENDLOG;
		return;
	}
	
	if (params.size() < ACTIVATE_PARAMS_NUM) {
		HCP_Log(ERR, "TracePointCallback") << "Config file parameter number wrong!" << HCPENDLOG;
		return;
	}
	
	if (params[1] == tpName) {
		str[0] = '0';
	}
	data += str;
	data += "\n";
    HCP_Log(DEBUG, "TracePointCallback") << "Parse line data success!" << HCPENDLOG;
}

void ModifyLineData(const string& fileName, const string& tpName)
{
    std::string str;
    std::ifstream fd(fileName);
    string strFileData = "";
    if (fd.is_open()) {
        while (!fd.eof()) {
            getline(fd, str);
            ParseLineData(str, tpName, strFileData);
        }
    } else {
        HCP_Log(ERR, "TracePointCallback") << "Open config file failed: " << fileName << HCPENDLOG;
        return;
    }
    fd.close();

    ofstream out;
    out.open(fileName);
    out.flush();
    out << strFileData;
    out.close();
}

void ModifyConfFile(const std::string& tpName)
{
    const std::string cfgPath = Module::ConfigReader::getString(
    std::string("MicroService"), std::string("ProcessRootPath")) + "/conf/tracepoint.ini";
    ModifyLineData(cfgPath, tpName);
}

void ResetReturnFailHook(std::string userparam, size_t paramNum, ...)
{
    va_list argList;
    va_start(argList, paramNum);
    if (paramNum != 1) {
        va_end(argList);
        return;
    }

    std::string *tpName = va_arg(argList, std::string*);
    ModifyConfFile(*tpName);
    va_end(argList);
    std::raise(SIGKILL);
}

