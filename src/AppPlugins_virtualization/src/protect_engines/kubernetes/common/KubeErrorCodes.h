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
#pragma once
#include "KubeMacros.h"
KUBERNETES_PLUGIN_NAMESPACE_BEGIN
    const int SUCCESS = 0;
    const int FAILED = -1;
    const int PASS = 2;
    const int ERR_PARAM = 1677929218;

    /**
     * 执行注册Kubernetes集群操作，无法访问Kubernetes集群对应的存储，操作失败。
     */
    const int STORAGE_AUTH_FAILED = 0x5E025068;

    /**
     * 执行注册Kubernetes集群操作时，由于输入存储IP地址信息与Kubernetes集群配套存储不匹配，操作失败。
     */
    const int STORAGE_NOT_MATCH = 0x5E025086;

    /**
     * 执行注册Kubernetes集群操作时，由于存储IP地址错误或网络连接超时，操作失败。
     */
    const int STORAGE_NETWORK_FAILED = 0x640333AF;

    /**
     * 执行注册Kubernetes集群操作时，由于配置文件有误或者集群状态异常，操作失败。
     */
    const int CONNECT_FAILED = 0x5E025069;

    // OceanStor 存储错误码
    const int OPERATION_ERROR = 1677929219;
    const int SNAP_NOT_EXIST = 1077937880;
    const int USERNAME_OR_PASSWORD_WRONG_V6 = 1077949061;
    const int USERNAME_OR_PASSWORD_WRONG_V3 = 1077987870;

KUBERNETES_PLUGIN_NAMESPACE_END
