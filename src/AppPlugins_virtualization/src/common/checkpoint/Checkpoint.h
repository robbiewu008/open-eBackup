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
#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include <string>
#include <memory>
#include <json/json.h>

#include "common/Macros.h"
#include "common/Structs.h"

#include "repository_handlers/RepositoryHandler.h"
#include "common/utils/Utils.h"
#include "log/Log.h"

VIRT_PLUGIN_NAMESPACE_BEGIN


template<class T>
class Checkpoint {
public:
    void SetHandle(std::shared_ptr<RepositoryHandler> handler, std::string repoPath);
    void SetJobId(const std::string& mainJobId, const std::string& subJobId);
    void SetMainJobPath(const std::string& mainJobId);
    bool Create();
    bool Clear();
    bool CreateCheckpointDirectory();
    bool Exist();
    bool Set(T& data);
    bool Get(T& data);

private:
    std::shared_ptr<RepositoryHandler> m_handler {nullptr};
    std::string m_repoPath;
    std::string m_file;
    std::string m_dir;
    std::string m_dataStr;
};

namespace VirtCheck {
    const std::string MODULE_NAME = "Checkpoint";
}

template<class T>
void Checkpoint<T>::SetHandle(std::shared_ptr<RepositoryHandler> handler, std::string repoPath)
{
    m_handler = handler;
    m_repoPath = repoPath;
}

template<class T>
void Checkpoint<T>::SetJobId(const std::string& mainJobId, const std::string& subJobId)
{
    std::string mainJobPath = "main_job_" + mainJobId;
    std::string subJobPath = "sub_job_" + subJobId + ".json";
    m_dir = m_repoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + mainJobPath;
#ifndef WIN32
    m_file = m_dir + "/" + subJobPath;
#else
    m_file = m_dir + "\\" + subJobPath;
#endif
    INFOLOG("Set checkpoint file successfully: %s", m_file.c_str());
}

template<class T>
void Checkpoint<T>::SetMainJobPath(const std::string& mainJobId)
{
    std::string mainJobPath = "main_job_" + mainJobId;
    m_dir = m_repoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + mainJobPath;
    INFOLOG("Set checkpoint main job path successfully: %s", m_dir.c_str());
}

template<class T>
bool Checkpoint<T>::Create()
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null");
        return false;
    }
    if (!m_handler->IsDirectory(m_dir)) {
        ERRLOG("Directory not exist: %s ", m_dir.c_str());
        return false;
    }
    if (!m_handler->Exists(m_dir)) {
        ERRLOG("Dir not exist: %s ", m_dir.c_str());
        if (!m_handler->CreateDirectory(m_dir)) {
            ERRLOG("Create dir failed: %s ", m_dir.c_str());
            return false;
        }
    } else {
        WARNLOG("Directory already exists: %s, will not create again", m_dir.c_str());
    }
    if (m_handler->Exists(m_file)) {
        INFOLOG("File: %s already exist", m_file.c_str());
        return true;
    }
    int res = Utils::RetryOpWithT<int32_t>(std::bind(&RepositoryHandler::Open, m_handler, m_file, "w+"), SUCCESS,
                                        "Open");
    if (res == FAILED) {
        m_handler->Close();
        ERRLOG("Create checkpoint main directory %s failed.", m_dir.c_str());
        return false;
    }
    if (m_handler->Close() != SUCCESS) {
        ERRLOG("Close file failed: %s ", m_file.c_str());
        return false;
    }

    INFOLOG("Create checkpoint file successfully: %s", m_file.c_str());
    return true;
}

template<class T>
bool Checkpoint<T>::CreateCheckpointDirectory()
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null.");
        return false;
    }
    if (m_handler->IsDirectory(m_dir)) {
        INFOLOG("Directory already exist: %s", m_dir.c_str());
        return true;
    }

    int res = Utils::RetryOpWithT<bool>(std::bind(&RepositoryHandler::CreateDirectory, m_handler, m_dir), true,
                                        "CreateDirectory");
    if (res == FAILED) {
        ERRLOG("Create checkpoint main directory %s failed.", m_dir.c_str());
        return false;
    }
    INFOLOG("Create checkpoint main directory successfully: %s", m_dir.c_str());
    return true;
}

template<class T>
bool Checkpoint<T>::Clear()
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null.");
        return false;
    }
    if (!m_handler->IsDirectory(m_dir)) {
        INFOLOG("Checkpoint dir: %s is not exist, no need delete.", m_dir.c_str());
        return true;
    }

    if (!m_handler->RemoveAll(m_dir)) {
        ERRLOG("Remove all checkpoint file failed.");
        return false;
    }
    INFOLOG("Remove all checkpoint file successfully.");
    return true;
}

template<class T>
bool Checkpoint<T>::Exist()
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null.");
        return false;
    }
    return m_handler->Exists(m_file);
}

template<class T>
bool Checkpoint<T>::Set(T& data)
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null.");
        return false;
    }
    if (!Module::JsonHelper::StructToJsonString(data, m_dataStr)) {
        ERRLOG("Convert struct to string failed.");
        return false;
    }

    int res = Utils::RetryOpWithT<int>(std::bind(&Utils::SaveToFile, m_handler, m_file, m_dataStr), SUCCESS,
                                       "SaveToFile");
    if (res == FAILED) {
        return false;
    }
    return true;
}

template<class T>
bool Checkpoint<T>::Get(T& data)
{
    if (m_handler == nullptr) {
        ERRLOG("Handler is null.");
        return false;
    }
    int res = Utils::RetryOpWithT<int>(std::bind(&Utils::LoadFileToStruct<T>, m_handler, m_file, std::ref(data)),
                                       SUCCESS, "LoadFileToStruct");
    if (res == FAILED) {
        return false;
    }
    return true;
}

VIRT_PLUGIN_NAMESPACE_END

#endif