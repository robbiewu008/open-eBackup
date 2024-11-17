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
#if (defined LINUX) && (!defined ENABLE_TSAN)
#include <gperftools/malloc_extension.h>
#endif

#include <iostream>
#include "common/CMpThread.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#ifndef WIN32
#include "common/StackTracer.h"
#endif
#include "message/tcp/CSocket.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/datapath/DataPath.h"
#include "dataprocess/datapath/CdpDataPath.h"
#include "dataprocess/datapath/VMwareNativeDataPathProcess.h"

static const mp_string LOG_FILE_NAME_SEPARATE = "_";
static const mp_string LOG_FILE_SUFFIX = ".log";
static const mp_int32 VERSION_NUM = 2;
static const mp_int32 RELEASE_MEM_FRE = 10;
static const std::vector<mp_string> DATAPROCESS_VERSION_LIST = {"5.1", "5.5", "6.0", "6.5", "6.7", "7.0", "8.0"};

namespace {
mp_int32 InitLog(const char *path, const mp_string &version)
{
    mp_int32 iRet = MP_SUCCESS;
    iRet = CPath::GetInstance().Init(path);
    if (iRet != MP_SUCCESS) {
        printf("Init path %s failed.\n", path);
        return iRet;
    }
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (iRet != MP_SUCCESS) {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }
    mp_string strFilePath = CPath::GetInstance().GetSlogPath();
    mp_string strFileName = DATAPROCESS_LOGNAME;
    if (!version.empty()) {
        strFileName += LOG_FILE_NAME_SEPARATE + version + LOG_FILE_SUFFIX;
    } else {
        strFileName += LOG_FILE_SUFFIX;
    }
    CLogger::GetInstance().Init(strFileName.c_str(), strFilePath);

    return iRet;
}

mp_int32 InitDataPath(const std::shared_ptr<DataPath> &pDataPath)
{
    mp_int32 iRet = MP_SUCCESS;
    pDataPath->StartExtCmdHandler();

    if (!pDataPath->GetSendExitFlag()) {
        if (pDataPath->handlerTid.os_id != 0) {
            CMpThread::WaitForEnd(&pDataPath->handlerTid, NULL);
        }
    }
    return (iRet);
}

std::shared_ptr<DataPath> HandleDataProcessType(mp_int32 argc, mp_char **argv)
{
    static const mp_int32 paramNum = 3;
    std::shared_ptr<DataPath> retDataPath = NULL;
    mp_int32 dpService = atoi(argv[1]);

    switch (dpService) {
        case OCEAN_MOBILITY_SERVICE:
            if (argc != VERSION_NUM) {
                std::cerr << "Usage: " << argv[0] << " <DataProcessServiceType>" << std::endl;
            }
            retDataPath = std::make_shared<CdpDataPath>("");
            break;
        case OCEAN_VMWARENATIVE_SERVICE:
            if (argc != paramNum) {
                std::cerr << "Usage: " << argv[0] << " <DataProcessServiceType> <VDDKVersion>" << std::endl;
                return NULL;
            }
            retDataPath = std::make_shared<VMwareNativeDataPathProcess>(argv[paramNum - 1]);
            break;
        default:
            break;
    }

    return retDataPath;
}
}

mp_int32 main(mp_int32 argc, mp_char **argv)
{
    mp_int32 iRet = MP_SUCCESS;

    mp_int32 iParams = 2;
    if (argc < iParams) {
        std::cerr << "Usage: " << argv[0] << " <DataProcessServiceType>" << std::endl;
        return MP_FAILED;
    }

    // ignore the SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);

#ifndef WIN32
    StackTracer stackTracer;
#endif

#if (defined LINUX) && (!defined ENABLE_TSAN)
    // tcmalloc 释放频率
    MallocExtension::instance()->SetMemoryReleaseRate(RELEASE_MEM_FRE);
#endif

    std::shared_ptr<DataPath> pDataPath = nullptr;
    pDataPath = HandleDataProcessType(argc, argv);
    if (pDataPath == nullptr) {
        return MP_FAILED;
    }

    if (argc > VERSION_NUM) {
        auto item = find(DATAPROCESS_VERSION_LIST.begin(), DATAPROCESS_VERSION_LIST.end(), argv[VERSION_NUM]);
        if (item == DATAPROCESS_VERSION_LIST.end()) {
            std::cerr << "invalid version: " << argv[VERSION_NUM] << std::endl;
            return MP_FAILED;
        }
        iRet = InitLog(argv[0], argv[VERSION_NUM]);
    } else {
        iRet = InitLog(argv[0], "");
    }
    if (iRet != MP_SUCCESS) {
        std::cout << "Dataprocess log file creation failed : " << std::endl;
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Begin to run data process service!");
    iRet = InitDataPath(pDataPath);

    return (iRet);
}
