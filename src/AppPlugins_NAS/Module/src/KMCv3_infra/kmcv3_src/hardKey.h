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
#ifndef HARD_KEY_H
#define HARD_KEY_H

#include <string>

class KMCHWKey {
public:
    static KMCHWKey &GetInstance();
    bool SetKeyPath(const std::string& path, const std::string& bakPath);
    bool LoadKey();
    bool NewKey();
    std::string GetKey();
public:
    KMCHWKey(KMCHWKey const &) = delete;
    void operator=(KMCHWKey const &) = delete;
    virtual ~KMCHWKey() = default;

private:
    KMCHWKey() {}
    std::string m_keyPath;
    std::string m_bakKeyPath;

    std::string m_key;
};

#endif