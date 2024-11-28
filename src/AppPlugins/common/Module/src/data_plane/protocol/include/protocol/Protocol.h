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
#ifndef DP_PROTOCOL_H
#define DP_PROTOCOL_H

// std
#include <string>

// Vendors
#include <boost/optional.hpp>

#include <protocol/Message.h>
#include <protocol/fuse/FuseMessage.h>

namespace Module {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11
// Entry data that were obtained via lookup, getattr and setattr would be cached in system for this specified time
constexpr double ENTRY_LIFE_SPAN = 5.0;
// Attributes that were obtained via lookup, getattr and setattr would be cached in system for this specified time
constexpr double ATTRIBUTES_LIFE_SPAN = 5.0;

}
}

#endif
