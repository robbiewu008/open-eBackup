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
#ifndef __ALARM_INFO_XML_PARSER_H__
#define __ALARM_INFO_XML_PARSER_H__

#include <map>
#include <vector>
#include "tinyxml2.h"
#include "common/Types.h"
#include "common/CMpThread.h"
#include "common/Path.h"

class AGENT_API AlarmInfoXmlParser {
public:
    static AlarmInfoXmlParser& GetInstance()
    {
        return m_instance;
    }
    ~AlarmInfoXmlParser()
    {
        CMpThread::DestroyLock(&m_cfgValueMutexLock);
    }

    mp_int32 Init(const mp_string& strCfgFilePath);
    mp_string GetRectification(const mp_string& strAlarmID);
    mp_string GetFaultTitle(const mp_string& strAlarmID);
    mp_int32 GetFaultType(const mp_string& strAlarmID);
    mp_int32 GetFaultLevel(const mp_string& strAlarmID);
    mp_string GetAdditionInfo(const mp_string& strAlarmID);
    mp_int32 GetFaultCategory(const mp_string& strAlarmID);

private:
    AlarmInfoXmlParser()
    {
        CMpThread::InitLock(&m_cfgValueMutexLock);
        m_pDoc = NULL;
        m_lastTime = 0;
    }
    mp_int32 Load();
    mp_bool IsModified();
    tinyxml2::XMLElement* GetAlarmElement(const mp_string& strAlarmID);

private:
    static AlarmInfoXmlParser m_instance;
    mp_string m_strCfgFilePath;
    mp_time m_lastTime;
    thread_lock_t m_cfgValueMutexLock;
    tinyxml2::XMLDocument* m_pDoc;
};

#endif  // __ALARM_INFO_XML_PARSER_H__
