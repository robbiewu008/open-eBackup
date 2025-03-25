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
#ifndef JOB_STATE_H
#define JOB_STATE_H

#include <functional>
#include <string>
#include <vector>
#include <type_traits>
#include "common/Types.h"
#include "common/Log.h"
#include "taskmanager/externaljob/Job.h"

namespace AppProtect {
using StateAction = std::function<mp_int32()>;
template<typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
class JobStateAction {
public:
    JobStateAction(const std::string& name, const T& curState, const T& failState, const T& succState)
        : m_name(name), m_curState(curState), m_failState(failState), m_succState(succState)
    {}
    JobStateAction() = delete;
    virtual ~JobStateAction(){};

    T Transition()
    {
        INFOLOG("%s from %d to %d, Begin to transition", m_name.c_str(), m_curState, m_succState);
        if (OnEnter == nullptr && OnTransition == nullptr && OnExit == nullptr) {
            ERRLOG("OnEnter and OnTransition and OnExit is null");
            return m_failState;
        }

        m_iRet = OnEnter == nullptr ? MP_SUCCESS : OnEnter();
        if (m_iRet != MP_SUCCESS) {
            if (OnFailed != nullptr) {
                OnFailed();
            }
            ERRLOG("%s from %d to %d, OnEnter failed, error %d", m_name.c_str(), m_curState, m_succState, m_iRet);
            return m_failState;
        }

        m_iRet = OnTransition == nullptr ? MP_SUCCESS : OnTransition();
        if (m_iRet != MP_SUCCESS) {
            if (OnFailed != nullptr) {
                OnFailed();
            }
            ERRLOG("%s from %d to %d, OnTransition failed, error %d", m_name.c_str(), m_curState, m_succState, m_iRet);
            return m_failState;
        }

        m_iRet = OnExit == nullptr ? MP_SUCCESS : OnExit();
        if (m_iRet != MP_SUCCESS) {
            if (OnFailed != nullptr) {
                OnFailed();
            }
            ERRLOG("%s from %d to %d, OnExit failed, error %d", m_name.c_str(), m_curState, m_succState, m_iRet);
            return m_failState;
        }

        INFOLOG("%s from %d to %d, transition successfully", m_name.c_str(), m_curState, m_succState);
        return m_succState;
    }

    mp_int32 GetLastResult()
    {
        return m_iRet;
    }

    T GetCurState()
    {
        return m_curState;
    }

    T GetFailState()
    {
        return m_failState;
    }

    T GetSuccState()
    {
        return m_succState;
    }

    // state action of enter state
    StateAction OnEnter = nullptr;
    // state action of exit state
    StateAction OnExit = nullptr;
    // state action of transiton state
    StateAction OnTransition = nullptr;

    StateAction OnFailed = nullptr;

protected:
    // state name
    std::string m_name;
    // state list
    T m_curState;
    T m_failState;
    T m_succState;
    mp_int32 m_iRet;
};

}  // namespace AppProtect
#endif
