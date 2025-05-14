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
package com.huawei.oceanprotect.system.base.initialize;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;

import lombok.extern.slf4j.Slf4j;

/**
 * 线程更新性能监控
 *
 * @version: [OceanProtect X8000 2.1.0]
 * @since 2022-08-26
 */
@Slf4j
public class UpdatePerformanceThread extends Thread {
    private final ClusterStorageService clusterStorageService;

    /**
     * 线程更新性能监控
     *
     * @param clusterStorageService 存储提供接口
     */
    public UpdatePerformanceThread(ClusterStorageService clusterStorageService) {
        super.setName("UpdatePerformanceThread");
        this.clusterStorageService = clusterStorageService;
    }

    /**
     * 初始化执行
     */
    @Override
    public void run() {
        clusterStorageService.updatePerformance(true, true);
    }
}
