/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;

import lombok.extern.slf4j.Slf4j;

/**
 * 线程更新性能监控
 *
 * @author swx1010572
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
