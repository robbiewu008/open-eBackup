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
#include "common/TimeOut.h"
#include "common/Log.h"
#include "common/CMpTime.h"

CTimeOut::CTimeOut(mp_uint32 uiSecTimeOut)
{
    CMpTime::Now(m_tStart);
    m_uiSecTimeout = uiSecTimeOut;
}

CTimeOut::~CTimeOut()
{
}

mp_uint32 CTimeOut::Remaining()
{
    COMMLOG(OS_LOG_DEBUG, "Begin calculate remainning time.");
    if (m_uiSecTimeout == 0) {
        COMMLOG(OS_LOG_DEBUG, "Remainning time is 0.");
        return 0;
    }

    if (TIMEOUT_INFINITE == m_uiSecTimeout) {
        COMMLOG(OS_LOG_DEBUG, "Remainning time is infinite.");
        return m_uiSecTimeout;
    }

    mp_time tNow;
    mp_uint32 uiRemain = 0;
    CMpTime::Now(tNow);
    mp_uint32 uiElapsed = tNow - m_tStart;
    if (uiElapsed >= m_uiSecTimeout) {
        COMMLOG(OS_LOG_DEBUG, "End calculate remainning time, uiRemain %d.", uiRemain);
        return 0;
    }

    uiRemain = m_uiSecTimeout - uiElapsed;
    COMMLOG(OS_LOG_DEBUG, "End calculate remainning time, uiRemain %d.", uiRemain);
    return uiRemain;
}

