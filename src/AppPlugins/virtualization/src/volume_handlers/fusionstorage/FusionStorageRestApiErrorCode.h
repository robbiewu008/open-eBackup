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
#ifndef FUSIONSTORAGE_RESTAPI_ERRORCODE_H
#define FUSIONSTORAGE_RESTAPI_ERRORCODE_H

#include <string>

enum class FusionStorageRestApiErrorCode {
    // 正常状态
    RESTAPI_OK = 0,
    // 错误的API请求参数
    RESTAPI_ERR_WRONG_REQUEST_PARAMETER = 1577209949,
    // 登录target失败
    RESTAPI_ERR_LOGIN_TARGET_FAILED = 1577209948,
    // 获得WWN失败
    RESTAPI_ERR_GET_WWN_FAILED = 1577210079,
    // 建立host-lun映射失败
    RESTAPI_ERR_CREATE_MAPPING_FAILED = 1577210093,
    // 获得磁盘路径
    RESTAPI_ERR_GET_DISKPATH_FAILED = 1577210104,
    // 启动器不存在
    RESTAPI_ERR_INITIATOR_IS_NOT_EXIST = 1577213518,

    RESTAPI_ERR_UNKNOWN = 0x7FFFFFFF
};

#endif