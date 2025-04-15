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
#ifndef OPEN_LIB_MGR_H
#define OPEN_LIB_MGR_H

#include <mutex>
#include <iostream>
#include "log/Log.h"
#include "common/Dlib.h"

#ifdef WIN32
#define AGENT_EXPORT __declspec(dllexport)
class AGENT_EXPORT OpenLibMgr {
#else
class OpenLibMgr {
#endif
public:
    OpenLibMgr(const OpenLibMgr&) = delete;
    OpenLibMgr& operator=(const OpenLibMgr&) = delete;

    static OpenLibMgr& GetInstance();

    bool InitLibHandle(const std::string& libName)
    {
        m_handle = Module::DlibOpen(libName);
        if (m_handle == nullptr) {
            return false;
        }
        return true;
    }

    bool InitLibHandleEx(const std::string& libName, bool bLocal)
    {
        INFOLOG("InitLibHandleEx.");
        m_handle = Module::DlibOpenEx(libName, bLocal);
        if (m_handle == nullptr) {
            ERRLOG("InitLibHandleEx nullptr failed.");
            return false;
        }
        return true;
    }

    template<typename T>
    T* GetObj(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_lock);
#ifdef WIN32
        return (T*)Module::DlibDlsym((Module::handle_t)m_handle, name.c_str());
#else
        return (T*)Module::DlibDlsym(m_handle, name.c_str());
#endif
    }

    void CloseLib()
    {
#ifdef WIN32
        Module::DlibClose((Module::handle_t)m_handle);
#else
        Module::DlibClose(m_handle);
#endif
    }
private:
    static OpenLibMgr instance;
    OpenLibMgr() {};
    ~OpenLibMgr() {};
    void* m_handle;
    std::mutex m_lock;
};

#endif
