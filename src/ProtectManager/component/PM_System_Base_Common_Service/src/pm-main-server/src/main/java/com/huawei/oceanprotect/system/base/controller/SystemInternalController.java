/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import openbackup.system.base.sdk.system.model.EsnInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 获取esn
 *
 * @author l00617376
 * @version OceanCyber 300 1.1.0
 * @since 2023-09-26
 */
@Slf4j
@RestController
public class SystemInternalController {
    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 查询集群esn
     *
     * @return 集群esn
     */
    @GetMapping("/v1/internal/system/esn")
    @ExterAttack
    public EsnInfo queryEsnInternal() {
        return new EsnInfo(clusterBasicService.getCurrentClusterEsn());
    }
}
