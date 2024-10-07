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
#ifndef __RETRY_OPER_H__
#define __RETRY_OPER_H__

#include <string>
#include <functional>
#include "log/Log.h"

namespace VirtPlugin {
namespace Utils {

template<typename T>
class RetryOper {
public:
    RetryOper() {}
    virtual ~RetryOper() {}

    void SetOperName(const std::string &name)
    {
        opName = name;
    }

    void SetOperator(std::function<T()> op)
    {
        oper = op;
    }

    void SetFailedChecker(std::function<bool(T&)> c)
    {
        operFailed = c;
    }

    void SetRetryEnable(const bool &e)
    {
        enable = e;
    }

    /* attemps means the first try and retry times,
     * therefore, if attemps set to 0, then oper() will not be executed
     */
    void SetAttempts(const int &a)
    {
        attemps = a;
    }

    void SetInterval(const int &i)
    {
        interval = i;
    }

    void AddBlackList(const int32_t &e)
    {
        errnoBlackList.insert(e);
    }

    void ClearBlackList()
    {
        errnoBlackList.clear();
    }

    /* invoke result, oper result, errno */
    std::pair<std::optional<T>, int32_t> Invoke()
    {
        int32_t errNo = -1;
        if (oper == nullptr) {
            ERRLOG("func:[%s] is null", opName.c_str());
            return std::make_pair(std::nullopt, FAILED);
        }

        std::optional<T> ret = std::nullopt;
        /* attemps includes the first try
         * if failed, retry attemps-1 times
         */
        while (attemps > 0) {
            ret = oper();
            errNo = errno;
            /* if not enable retry OR operFailed not provided OR oper success, break retry loop */
            if (!enable || operFailed == nullptr || !operFailed(ret.value())) {
                break;
            }
            /* if errno in black list, not retry */
            if (errnoBlackList.find(errNo) != errnoBlackList.end()) {
                break;
            }
            WARNLOG("Operation[%s] failed, errno[%d]: %s", opName.c_str(), errNo, strerror(errNo));
            sleep(interval);
            attemps--;
            if (attemps == 0) {
                WARNLOG("Operation[%s] failed.", opName.c_str());
                return std::make_pair(ret, errNo);
            }
            WARNLOG("Operation[%s] failed, will retry, attemps left: %d", opName.c_str(), attemps);
        }
        return std::make_pair(ret, errNo);
    }

private:
    bool enable = true;
    int attemps = 4; /* means, excute one times, if failed retry 3 times */
    int interval = 3;
    std::string opName;
    std::function<T()> oper = nullptr;
    std::function<bool(T&)> operFailed = nullptr;
    std::set<int32_t> errnoBlackList; /* errno in the list will not retry */
};
} // end namespace Utils
} // end namespace VirtPlugin

#endif // __RETRY_OPER_H__