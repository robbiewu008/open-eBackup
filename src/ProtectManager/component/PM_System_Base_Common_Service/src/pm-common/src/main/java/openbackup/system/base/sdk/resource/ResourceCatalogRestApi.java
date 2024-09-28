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
package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.resource.model.ResourceCatalogSchema;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * get protected resource
 *
 */
@FeignClient(name = "ResourceCatalogService", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface ResourceCatalogRestApi {
    /**
     * 查询资源目录
     *
     * @return List<ResourceCatalogSchema>
     */
    @ExterAttack
    @GetMapping("/resource-catalogs")
    @ResponseBody
    List<ResourceCatalogSchema> queryResourceCatalog();

    /**
     * 隐藏资源目录
     *
     * @param catalogIds resource catalog uuid list
     */
    @PutMapping("/resource-catalogs/action/hidden")
    @ResponseBody
    void hiddenCatalog(@RequestBody List<String> catalogIds);

    /**
     * 展示资源目录
     *
     * @param catalogIds resource catalog uuid list
     */
    @PutMapping("/resource-catalogs/action/show")
    @ResponseBody
    void showCatalog(@RequestBody List<String> catalogIds);
}
