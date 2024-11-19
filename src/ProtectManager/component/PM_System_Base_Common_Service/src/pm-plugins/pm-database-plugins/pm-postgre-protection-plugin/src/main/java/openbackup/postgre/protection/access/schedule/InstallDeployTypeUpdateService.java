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
package openbackup.postgre.protection.access.schedule;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigConstants;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.system.base.query.PageQueryOperator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.MapUtils;
import org.apache.curator.shaded.com.google.common.collect.ImmutableMap;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * InstallDeployType更新service
 *
 */
@Slf4j
@Component
public class InstallDeployTypeUpdateService implements CommandLineRunner {
    private final ResourceService resourceService;

    private final ResourceExtendInfoService resourceExtendInfoService;

    /**
     * 有参构造
     *
     * @param resourceService 资源服务
     * @param resourceExtendInfoService 扩展信息服务
     *
     */
    public InstallDeployTypeUpdateService(ResourceService resourceService,
        ResourceExtendInfoService resourceExtendInfoService) {
        this.resourceService = resourceService;
        this.resourceExtendInfoService = resourceExtendInfoService;
    }

    /**
     * 服务启动后，更新Pgsql的InstallDeployType
     *
     * @param args 程序启动参数
     * @throws Exception exception
     */
    @Override
    public void run(String... args) throws Exception {
        log.info("Start to update InstallDeployType jobs...");
        ResourceQueryParams queryParams = new ResourceQueryParams();
        queryParams.setShouldIgnoreOwner(true);
        Map<String, Object> conditionMap = ImmutableMap.of(PluginConfigConstants.SUB_TYPE,
            Lists.newArrayList(Collections.singletonList(PageQueryOperator.IN.getValue()),
                ResourceSubTypeEnum.POSTGRE_CLUSTER.getType(), ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()));
        queryParams.setConditions(conditionMap);
        PageListResponse<ProtectedResource> pageListResponse = resourceService.query(queryParams);
        if (pageListResponse.getTotalCount() > 0) {
            List<ProtectedResource> result = pageListResponse.getRecords();
            for (ProtectedResource resource : result) {
                Map<String, String> extendInfo = resourceExtendInfoService.queryExtendInfo(resource.getUuid(),
                    PostgreConstants.INSTALL_DEPLOY_TYPE);
                if (MapUtils.isEmpty(extendInfo)) {
                    log.info("update resource uuid: {}", resource.getUuid());
                    resourceExtendInfoService.saveOrUpdateExtendInfo(resource.getUuid(),
                        Collections.singletonMap(PostgreConstants.INSTALL_DEPLOY_TYPE, PostgreConstants.PGPOOL));
                }
            }
        }
    }
}
