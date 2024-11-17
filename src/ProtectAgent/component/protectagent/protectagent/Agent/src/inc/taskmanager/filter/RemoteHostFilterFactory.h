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
#ifndef FILTER_REMOTE_HOST_FILTER_FACTORY_H
#define FILTER_REMOTE_HOST_FILTER_FACTORY_H

#include "taskmanager/filter/RemoteHostFilter.h"

typedef enum {
    /* logic port type filter */
    PORT_TYPE_FILTER,
    /* unreachable host filter */
    UNREACHABLE_FILTER
} FilterType;

class RemoteHostFilterFactory {
public:
    RemoteHostFilterFactory();
    ~RemoteHostFilterFactory();
    static std::shared_ptr<RemoteHostFilter> CreateFilter(FilterType type);
    static FilterAction CreateFilterAction(FilterType type);
};
#endif