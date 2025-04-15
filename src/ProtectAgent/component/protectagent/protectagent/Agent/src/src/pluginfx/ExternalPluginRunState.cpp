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
#include "pluginfx/ExternalPluginRunState.h"
#include "pluginfx/ExternalPlugin.h"
#include "common/Log.h"

mp_uint32 ExPluginStateBase::PluginClosed()
{
    INFOLOG("Receive closed notification.");
    CHECK_POINTER_NULL(m_context);
    mp_uint32 iRet = m_context->KillProcess();
    if (iRet != MP_SUCCESS) {
        m_context->ChangeStatus(EX_PLUGIN_STATUS::CLOSING);
        return iRet;
    }
    m_context->ChangeStatus(EX_PLUGIN_STATUS::CLOSED);
    return iRet;
}

mp_uint32 PluginIdleState::PluginStarting()
{
    INFOLOG("Receive start req in idle status.");
    CHECK_POINTER_NULL(m_context);
    if (m_context->ExecStartPlugin() != MP_SUCCESS) {
        ERRLOG("Start plugin failed.");
        return MP_FAILED;
    }
    m_context->ChangeStatus(EX_PLUGIN_STATUS::STARTING);
    return MP_SUCCESS;
}

mp_uint32 PluginStartingState::PluginRegistered()
{
    INFOLOG("Receive registered notification in starting status.");
    CHECK_POINTER_NULL(m_context);
    m_context->ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
    return MP_SUCCESS;
}

mp_uint32 PluginStartingState::PluginClosing()
{
    INFOLOG("Receive close req in starting status.");
    CHECK_POINTER_NULL(m_context);
    if (m_context->ExecStopPlugin() != MP_SUCCESS) {
        ERRLOG("Stop plugin failed.");
        return MP_FAILED;
    }
    m_context->ChangeStatus(EX_PLUGIN_STATUS::CLOSING);
    return MP_SUCCESS;
}

mp_uint32 PluginRegisterdState::PluginStarting()
{
    INFOLOG("Receive start req in registered status.");
    return MP_SUCCESS;
}

mp_uint32 PluginRegisterdState::PluginClosing()
{
    INFOLOG("Receive close req in registered status.");
    CHECK_POINTER_NULL(m_context);
    m_context->ChangeStatus(EX_PLUGIN_STATUS::CLOSING);
    return m_context->ExecStopPlugin();
}