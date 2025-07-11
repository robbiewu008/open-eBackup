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
package openbackup.system.base.sdk.accesspoint.restapi;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.accesspoint.Initialize;
import openbackup.system.base.sdk.accesspoint.model.InitializeParam;
import openbackup.system.base.sdk.accesspoint.model.InitializeResult;
import openbackup.system.base.sdk.accesspoint.model.StandardBackupVolInitInfo;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

import java.util.List;

/**
 * JobCenter Client Service
 *
 */
@FeignClient(name = "InitializeRestApi", url = "${service.url.pm-dm-access-point}/v1/ab/internal/initialize",
    configuration = CommonFeignConfiguration.class)
public interface InitializeRestApi extends Initialize {
    /**
     * 初始化备份存储
     *
     * @param initializeParam 备份存储参数
     * @return 初始化结果
     */
    @Override
    @PutMapping("/backStorage")
    InitializeResult initializeBackStorage(@RequestBody InitializeParam initializeParam);

    /**
     * 获取当前节点挂载的卷信息
     *
     * @return 卷信息列表
     */
    @Override
    @GetMapping("/volumeInfos")
    List<StandardBackupVolInitInfo> queryVolumeInfo();
}
