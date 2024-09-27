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
package openbackup.database.base.plugin.utils;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;

import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 数据库扫描公共工具类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-30
 */
@Slf4j
public final class DatabaseScannerUtils {
    private DatabaseScannerUtils() {
    }

    /**
     * 查询主机下面的数据库服务实例（前提是按照规范定义数据结构：extendInfo需要维护维护主机hostId）
     *
     * @param environmentId 环境id
     * @param subtype 资源子类型
     * @param resourceService 受保护资源服务
     * @return 环境下面数据库服务实例资源列表
     */
    public static List<ProtectedResource> getInstancesByEnvironment(String environmentId, String subtype,
        ResourceService resourceService) {
        // 查询主机下面数据库服务实例
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, subtype);
        conditions.put(DatabaseConstants.HOST_ID, environmentId);
        PageListResponse<ProtectedResource> queryRes = resourceService.query(0, DatabaseConstants.PAGE_SIZE, conditions,
            "");
        List<ProtectedResource> instances = queryRes.getRecords();
        int totalCount = queryRes.getTotalCount();
        if (totalCount == 0) {
            log.info("The environment {} does not have instances, subtype {}.", environmentId, subtype);
            return new ArrayList<>();
        }
        if (totalCount > instances.size()) {
            instances = resourceService.query(0, totalCount, conditions, "").getRecords();
        }
        instances.forEach(item -> {
            Optional<ProtectedResource> resource = resourceService.getResourceById(item.getUuid());
            resource.ifPresent(protectedResource -> item.setDependencies(protectedResource.getDependencies()));
        });
        return instances;
    }
}
