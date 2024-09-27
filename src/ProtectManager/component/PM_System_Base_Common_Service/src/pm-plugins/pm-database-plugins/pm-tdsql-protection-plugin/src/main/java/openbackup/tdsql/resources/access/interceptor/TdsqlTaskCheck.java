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
package openbackup.tdsql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.Group;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.provider.TdsqlClusterProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.DataNodeEquator;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections4.CollectionUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

/**
 * 功能描述 校验TDSQL任务
 *
 * @author z30047175
 * @since 2023-05-30
 */
@Slf4j
@Component
public class TdsqlTaskCheck {
    private final TdsqlService tdsqlService;

    private final TdsqlClusterProvider tdsqlClusterProvider;

    /**
     * 构造器
     *
     * @param tdsqlService tdsqlService
     * @param tdsqlClusterProvider tdsqlClusterProvider
     */
    public TdsqlTaskCheck(TdsqlService tdsqlService, TdsqlClusterProvider tdsqlClusterProvider) {
        this.tdsqlService = tdsqlService;
        this.tdsqlClusterProvider = tdsqlClusterProvider;
    }

    /**
     * 校验生产环境的变化
     *
     * @param clusterId 集群id
     * @param instanceId 实例id
     * @return TDSQLInstance
     */
    public TdsqlInstance checkEnvChange(String clusterId, String instanceId) {
        // 获取备份实例
        ProtectedResource instanceResource = tdsqlService.getResourceById(instanceId);
        String backupInstanceJson = instanceResource.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        TdsqlInstance backupInstance = JsonUtil.read(backupInstanceJson, TdsqlInstance.class);

        // 获取扫描实例
        ProtectedEnvironment cluster = tdsqlService.getEnvironmentById(clusterId);
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setResourceType(ResourceSubTypeEnum.TDSQL_CLUSTER.getType());
        conditions.setConditions(JsonUtil.json(backupInstance));
        AtomicReference<TdsqlInstance> browseInstance = new AtomicReference<>(new TdsqlInstance());
        PageListResponse<ProtectedResource> detailPageList = tdsqlClusterProvider.browse(cluster, conditions);
        if (detailPageList == null || CollectionUtils.isEmpty(detailPageList.getRecords())) {
            log.error("cluster instance is empty");
            throw new LegoCheckedException(TdsqlConstant.INCONSISTENT_NODES, "cluster instance is empty");
        }
        detailPageList.getRecords().forEach(protectedResource -> {
            String clusterInstanceInfo = protectedResource.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
            TdsqlInstance read = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
            if (read.getId().equals(backupInstance.getId())) {
                browseInstance.set(read);
            }
        });

        if (checkDifference(backupInstance, browseInstance.get())) {
            log.error("checkDifference in checkEnvChange failed");
            updateInstanceStatus(instanceResource);
            throw new LegoCheckedException(TdsqlConstant.INCONSISTENT_NODES,
                "The number of backup tasks is inconsistent with that in the production environment.");
        }
        return backupInstance;
    }

    /**
     * 判断生产环境是否变化+主备切换
     *
     * @param backupInstance 备份实例
     * @param browseInstance 扫描出来的实例
     * @return true or false
     */
    public boolean checkDifference(TdsqlInstance backupInstance, TdsqlInstance browseInstance) {
        // 判断节点个数是否相同
        ProtectedEnvironment backupEnvironment = new ProtectedEnvironment();
        backupEnvironment.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(backupInstance));
        ProtectedEnvironment browseEnvironment = new ProtectedEnvironment();
        browseEnvironment.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(browseInstance));
        if (tdsqlService.getInstanceDataNodes(backupEnvironment).size() != tdsqlService.getInstanceDataNodes(
            browseEnvironment).size()) {
            return true;
        }

        Map<String, List<DataNode>> backupDataMap = backupInstance.getGroups()
            .stream()
            .collect(Collectors.toMap(Group::getSetId, Group::getDataNodes));

        Map<String, List<DataNode>> browseDataMap = browseInstance.getGroups()
            .stream()
            .collect(Collectors.toMap(Group::getSetId, Group::getDataNodes));

        if (!backupDataMap.keySet().equals(browseDataMap.keySet())) {
            return true;
        }

        // 判断节点ip+port是否相同,主备信息不加入判断条件
        DataNodeEquator equator = new DataNodeEquator();
        for (String setId : backupDataMap.keySet()) {
            List<DataNode> backupDataList = backupDataMap.get(setId);
            List<DataNode> browseDataList = browseDataMap.get(setId);
            if (!CollectionUtils.isEqualCollection(backupDataList, browseDataList, equator)) {
                return true;
            }
        }
        return false;
    }

    /**
     * 若参数与生产环境不一致，需要置实例状态为离线
     *
     * @param instanceResource 实例
     */
    private void updateInstanceStatus(ProtectedResource instanceResource) {
        tdsqlService.updateInstanceLinkStatus(instanceResource, LinkStatusEnum.OFFLINE.getStatus().toString());
    }
}
