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
