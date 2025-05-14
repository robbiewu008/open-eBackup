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
package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;

import org.springframework.web.multipart.MultipartFile;

/**
 * 获取LLD配置信息和并返回
 *
 */
public interface InitializeNetworkBodyXls {
    /**
     * 根据过滤条件获取端口列表
     *
     * @param lld 过滤条件
     * @return 所有的端口信息
     */
    InitNetworkBody checkAndReturnInitXls(MultipartFile lld);
}
