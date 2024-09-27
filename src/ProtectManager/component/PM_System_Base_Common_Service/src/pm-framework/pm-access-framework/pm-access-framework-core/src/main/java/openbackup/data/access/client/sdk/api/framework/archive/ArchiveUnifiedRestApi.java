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
package openbackup.data.access.client.sdk.api.framework.archive;

import openbackup.data.protection.access.provider.sdk.archive.v2.ArchiveCopyMetadata;
import openbackup.data.protection.access.provider.sdk.archive.v2.ArchiveTask;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 归档统一框架api
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/13
 **/
@FeignClient(
        name = "archiveUnifiedRestApi",
        url = "${services.endpoints.protectengine.archive}",
        path = "/v2/internal/dme_archive",
        configuration = CommonFeignConfiguration.class)
public interface ArchiveUnifiedRestApi {
    /**
     * 下发DME创建归档任务
     *
     * @param archiveTask 创建归档任务对象
     */
    @PostMapping("/tasks/archive")
    void createArchiveTask(@RequestBody ArchiveTask archiveTask);

    /**
     * 更新副本元数据接口。将副本的json内容存入归档存储
     *
     * @param archiveCopyMetadata 更新元数据请求对象
     */
    @PutMapping("/snapmeta")
    void updateCopyMetadata(@RequestBody ArchiveCopyMetadata archiveCopyMetadata);

    /**
     * 终止归档任务
     *
     * @param taskId 任务id
     * @param requestId 请求id
     */
    @PutMapping("/task/{taskId}/abort")
    void abortTask(@PathVariable String taskId, @RequestParam("request_id") String requestId);
}
