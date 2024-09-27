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
package openbackup.data.access.client.sdk.api.storage;

import openbackup.data.access.client.sdk.api.storage.model.RepositoryBase;
import openbackup.data.access.client.sdk.api.storage.model.StorageInfo;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.system.base.common.model.storage.FullStorageInfo;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-16
 */
@FeignClient(name = "systemBaseService", url = "${pm-system-base.url}",
    configuration = CommonFeignConfiguration.class)
public interface StorageRestApi {
    /**
     * 测试接口
     *
     * @param startPage startPage
     * @param pageSize pageSize
     * @return 测试
     */
    @ExterAttack
    @GetMapping("/v1/storages")
    @ResponseBody
    PageListResponse<RepositoryBase> storage(@RequestParam("startPage") int startPage,
        @RequestParam("pageSize") int pageSize);

    /**
     * 查询存储信息
     *
     * @return StorageInfo
     */
    @ExterAttack
    @GetMapping("/v1/internal/storages/storage-info")
    @ResponseBody
    List<StorageInfo> storageInfo();

    /**
     * 查询存储全部信息
     *
     * @param storageId 存储库id
     * @return CompleteStorageInfoRes
     */
    @ExterAttack
    @GetMapping("/v1/internal/storages/{storageId}")
    @ResponseBody
    FullStorageInfo queryFullStorage(@PathVariable("storageId") String storageId);
}