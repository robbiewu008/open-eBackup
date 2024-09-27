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
package openbackup.data.access.client.sdk.api.framework.dee;

import openbackup.data.access.client.sdk.api.framework.dee.model.DeleteCopyIndexRequest;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexTask;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 统一备份框架DEE相关接口定义
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-29
 */
@FeignClient(name = "deeTaskRestApi", url = "${services.endpoints.protectengine.dee}",
    configuration = CommonFeignConfiguration.class)
public interface DeeUnifiedRestApi {
    /**
     * DEE创建副本索引接口
     *
     * @param indexTask 索引任务参数
     */
    @PostMapping("/v1/internal/indexes")
    void createIndexTask(@RequestBody CopyIndexTask indexTask);

    /**
     * DEE删除资源或副本索引接口
     *
     * @param requestId 请求Id
     * @param resourceId 资源Id
     * @param copyId 副本Id
     * @param chainId 备份链Id
     * @param userId 用户Id
     */
    @DeleteMapping("/v1/internal/indexes")
    void deleteIndexTask(@RequestParam("requestId") String requestId, @RequestParam("resourceId") String resourceId,
        @RequestParam(value = "copyId", required = false, defaultValue = "") String copyId,
        @RequestParam(value = "chainId", required = false, defaultValue = "") String chainId,
        @RequestParam("userId") String userId);

    /**
     * 删除指定副本索引
     *
     * @param deleteCopyIndexRequest 删除副本索引请求
     */
    @DeleteMapping("/v2/internal/indexes")
    void deleteIndexTask(@RequestBody DeleteCopyIndexRequest deleteCopyIndexRequest);
}
