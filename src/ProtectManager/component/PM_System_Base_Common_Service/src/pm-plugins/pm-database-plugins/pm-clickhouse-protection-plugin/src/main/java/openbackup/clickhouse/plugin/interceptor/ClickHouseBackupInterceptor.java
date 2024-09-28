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
package openbackup.clickhouse.plugin.interceptor;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * ClickHouse备份拦截器
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class ClickHouseBackupInterceptor extends AbstractDbBackupInterceptor {
    private final ClickHouseService clickHouseService;

    private final ResourceService resourceService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final ClickHouseAgentProvider clickHouseAgentProvider;

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(subType);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // PM 默认创建了data仓的StorageRepository对象, DME在收到data仓的时候，默认给创建了meta仓和data仓
        // 目录副本格式1-INNER_DIRECTORY
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        Map<String, String> protectEnvExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        protectEnvExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());

        // 默认为单文件系统，多文件系统需要在高级参数中添加advanceParams.put("multiFileSystem", "true")
        backupTask.addParameter(ClickHouseConstant.ADVANCE_PARAMS_KEY_MULTI_FILE_SYSTEM, Boolean.TRUE.toString());
        backupTask.addParameter(ClickHouseConstant.ADVANCE_PARAMS_KEY_MULTI_POST_JOB, Boolean.TRUE.toString());
        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        return backupTask;
    }

    @Override
    protected void supplyAgent(BackupTask backupTask) {
        ProtectedResource resource = BeanTools.copy(backupTask.getProtectObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        backupTask.setAgents(clickHouseAgentProvider.getSelectedAgents(agentSelectParam));
    }

    @Override
    protected void checkConnention(BackupTask backupTask) {
        // do nothing
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        // 获取所有节点
        TaskResource protectObject = backupTask.getProtectObject();
        String clusterId = protectObject.getRootUuid();
        log.info("ClickHouse clusterId:{} backup supplyNodes.", clusterId);
        ProtectedResource clusterResource = resourceService.getResourceById(clusterId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        List<ProtectedResource> nodeResources = clusterResource.getDependencies().get(ResourceConstants.CHILDREN);
        AuthParamUtil.convertKerberosAuth(nodeResources, kerberosService, encryptorService);

        // 获取要保护的表
        List<ProtectedResource> protectedTables = null;
        boolean isBackupDatabase = isBackupDatabaseObject(backupTask);
        if (!isBackupDatabase) {
            // 表集中保护的表
            ProtectedResource tableSetResource = resourceService.getResourceById(protectObject.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
            protectedTables = tableSetResource.getDependencies().get(ResourceConstants.CHILDREN);
        }

        // key为节点，value为当前节点要备份的表
        Map<ProtectedResource, List<ProtectedResource>> nodeTables = Maps.newHashMap();

        // 单独把ReplicatedMergeTree类型的表拧出来计算, key为表名，value的key为分片号，value的value为备份的节点
        Map<String, Map<String, List<ProtectedResource>>> tableShardNodes = Maps.newHashMap();

        // 不存在的表
        Set<String> doesNotExistTableNames = Sets.newHashSet();
        List<List<ProtectedResource>> tablesInDatabases = Lists.newArrayList();
        ProtectedEnvironment cluster = protectedEnvironmentService.getEnvironmentById(clusterId);
        for (ProtectedResource nodeInfo : nodeResources) {
            // 获取agent信息
            ProtectedEnvironment agent = protectedEnvironmentService.getEnvironmentById(
                clickHouseService.selectAgent(nodeInfo).getId());
            // 检查库是否存在
            String databaseName = isBackupDatabase ? protectObject.getName() : protectObject.getParentName();
            checkDatabaseExist(nodeInfo, agent, databaseName);
            List<ProtectedResource> tablesInDatabase = clickHouseService.queryClusterDetail(agent, nodeInfo,
                ClickHouseConstant.QUERY_TYPE_VALUE_TABLE, databaseName, null).getRecords();
            tablesInDatabase = tablesInDatabase.stream()
                .map(item -> convertToTable(cluster, item))
                .collect(Collectors.toList());
            if (isBackupDatabase) {
                tablesInDatabases.add(tablesInDatabase);
                protectedTables = tablesInDatabase;
            } else {
                fillInDoesNotExistTableNames(protectedTables, doesNotExistTableNames, tablesInDatabase);
            }
            nodeTables.put(nodeInfo, protectedTables);
            for (ProtectedResource table : protectedTables) {
                fillInTableShardNodes(tableShardNodes, nodeInfo, table);
            }
        }
        dealWithDoesNotExistTable(protectedTables, isBackupDatabase, nodeTables, doesNotExistTableNames,
            tablesInDatabases);

        // 随机分配表对应的备份节点
        Map<String, List<ProtectedResource>> tableNodeMap = getTableNodeMap(tableShardNodes);

        // 依次查询所有节点需要备份的表，转换为TaskEnvironment对象
        List<TaskEnvironment> protectEnvNodes = nodeResources.stream()
            .map(nodeResource -> convertToTaskEnvironment(protectObject, isBackupDatabase, nodeTables.get(nodeResource),
                tableNodeMap, nodeResource))
            .collect(Collectors.toList());
        backupTask.getProtectEnv().setNodes(protectEnvNodes);
    }

    private void checkDatabaseExist(ProtectedResource nodeInfo, ProtectedEnvironment agent, String databaseName) {
        log.info("ClickHouse databaseName:{} backup checkDatabaseExist.", databaseName);
        List<ProtectedResource> databaseResources = clickHouseService.queryClusterDetail(agent, nodeInfo,
            ClickHouseConstant.QUERY_TYPE_VALUE_DATABASE, null, null).getRecords();
        if (CollectionUtils.isEmpty(databaseResources)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "database not exist");
        }
        Set<String> databaseResourceNames = databaseResources.stream()
            .map(item -> item.getExtendInfoByKey(ClickHouseConstant.DB_NAME))
            .collect(Collectors.toSet());
        if (!databaseResourceNames.contains(databaseName)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "database not exist");
        }
    }

    private void dealWithDoesNotExistTable(List<ProtectedResource> protectedTables, boolean isBackupDatabaseObject,
        Map<ProtectedResource, List<ProtectedResource>> nodeTables, Set<String> doesNotExistTableNames,
        List<List<ProtectedResource>> tablesInDatabases) {
        if (isBackupDatabaseObject) {
            List<Set<String>> tableNamesInDatabases = tablesInDatabases.stream()
                .map(item -> item.stream().map(it -> it.getName()).collect(Collectors.toSet()))
                .collect(Collectors.toList());
            // 并集
            HashSet<String> union = new HashSet<>(tableNamesInDatabases.get(0));
            // 交集
            HashSet<String> intersection = new HashSet<>(tableNamesInDatabases.get(0));
            for (int i = 1; i < tableNamesInDatabases.size(); i++) {
                union.addAll(tableNamesInDatabases.get(i));
                intersection.retainAll(tableNamesInDatabases.get(i));
            }
            union.removeAll(intersection);
            doesNotExistTableNames.addAll(union);
        }
        if (CollectionUtils.isNotEmpty(doesNotExistTableNames)) {
            if (isBackupDatabaseObject) {
                for (ProtectedResource node : nodeTables.keySet()) {
                    List<ProtectedResource> protectedTablesTemp = nodeTables.get(node);
                    putBackupFlag(protectedTablesTemp, doesNotExistTableNames);
                }
            } else {
                putBackupFlag(protectedTables, doesNotExistTableNames);
            }
        }
        log.info("ClickHouse backup doesNotExistTableNames:{}.", doesNotExistTableNames);
    }

    private void putBackupFlag(List<ProtectedResource> protectedTables, Set<String> doesNotExistTableNames) {
        for (ProtectedResource protectedTable : protectedTables) {
            if (doesNotExistTableNames.contains(protectedTable.getName())) {
                protectedTable.getExtendInfo()
                    .put(ClickHouseConstant.BACKUP_TABLES_BACKUP_DATA_KEY,
                        ClickHouseConstant.BACKUP_DATA_TABLE_DOES_NOT_EXIST);
            }
        }
    }

    private ProtectedResource convertToTable(ProtectedEnvironment cluster, ProtectedResource item) {
        String tableName = item.getName();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEnvironment(cluster);
        protectedResource.setName(tableName);
        protectedResource.setType(ClickHouseConstant.TABLE_TYPE);
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResource.setVersion(cluster.getVersion());
        protectedResource.setRootUuid(cluster.getUuid());
        protectedResource.setPath(StringUtils.defaultIfEmpty(cluster.getName(), tableName));
        protectedResource.setAuthorizedUser(cluster.getAuthorizedUser());
        protectedResource.setUserId(cluster.getUserId());
        protectedResource.setExtendInfo(item.getExtendInfo());
        return protectedResource;
    }

    private void fillInDoesNotExistTableNames(List<ProtectedResource> protectedTables,
        Set<String> doesNotExistTableNames, List<ProtectedResource> tablesInDatabase) {
        Set<String> protectedTableNames = protectedTables.stream()
            .map(item -> item.getName())
            .collect(Collectors.toSet());
        Set<String> tablesInDatabaseNames = tablesInDatabase.stream()
            .map(item -> item.getName())
            .collect(Collectors.toSet());
        protectedTableNames.removeAll(tablesInDatabaseNames);
        doesNotExistTableNames.addAll(protectedTableNames);
    }

    private Map<String, List<ProtectedResource>> getTableNodeMap(
        Map<String, Map<String, List<ProtectedResource>>> tableShardNodes) {
        Map<String, List<ProtectedResource>> tableNodeMap = Maps.newHashMap();
        for (String table : tableShardNodes.keySet()) {
            Map<String, List<ProtectedResource>> shardNodes = tableShardNodes.get(table);
            for (String shard : shardNodes.keySet()) {
                List<ProtectedResource> nodes = shardNodes.get(shard);
                Collections.shuffle(nodes);
                List<ProtectedResource> nodeList = tableNodeMap.get(table);
                if (CollectionUtils.isEmpty(nodeList)) {
                    tableNodeMap.put(table, Lists.newArrayList(nodes.get(0)));
                } else {
                    nodeList.add(nodes.get(0));
                }
            }
        }
        return tableNodeMap;
    }

    private void fillInTableShardNodes(Map<String, Map<String, List<ProtectedResource>>> tableShardNodes,
        ProtectedResource nodeInfo, ProtectedResource table) {
        Map<String, String> tableExtendInfo = table.getExtendInfo();
        if (StringUtils.equalsIgnoreCase(ClickHouseConstant.REPLICATED_MERGE_TREE_ENGINE,
            MapUtils.getString(tableExtendInfo, ClickHouseConstant.TABLE_ENGINE))) {
            String tableName = table.getName();
            String shardNum = MapUtils.getString(nodeInfo.getExtendInfo(), ClickHouseConstant.SHARD_NUM);
            Map<String, List<ProtectedResource>> shardNodes = tableShardNodes.get(tableName);
            if (MapUtils.isEmpty(shardNodes)) {
                Map<String, List<ProtectedResource>> shardNodesTemp = Maps.newHashMap();
                shardNodesTemp.put(shardNum, Lists.newArrayList(nodeInfo));
                tableShardNodes.put(tableName, shardNodesTemp);
            } else {
                List<ProtectedResource> nodesTemp = shardNodes.get(shardNum);
                if (CollectionUtils.isEmpty(nodesTemp)) {
                    shardNodes.put(shardNum, Lists.newArrayList(nodeInfo));
                } else {
                    nodesTemp.add(nodeInfo);
                }
            }
        }
    }

    private TaskEnvironment convertToTaskEnvironment(TaskResource protectObject, boolean isBackupDatabaseObject,
        List<ProtectedResource> tables, Map<String, List<ProtectedResource>> tableNodeMap,
        ProtectedResource nodeResource) {
        // 获取agent信息
        ProtectedResource agentResource = nodeResource.getDependencies().get(DatabaseConstants.AGENTS).get(0);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(nodeResource, taskEnvironment);

        Map<String, String> extendInfo = taskEnvironment.getExtendInfo();
        extendInfo.put(ClickHouseConstant.AGENT_ID, agentResource.getUuid());
        extendInfo.put(ClickHouseConstant.BACKUP_OBJECT_KEY, isBackupDatabaseObject
            ? ClickHouseConstant.BACKUP_OBJECT_DATABASE_TYPE
            : ClickHouseConstant.BACKUP_OBJECT_TABLE_SET_TYPE);

        // 转换为backupTable
        JSONObject backupTables = new JSONObject();
        tables.forEach(protectedTable -> {
            String backupDataFlag = getBackupDataFlag(tableNodeMap, nodeResource, protectedTable);
            if (!(isBackupDatabaseObject && StringUtils.equals(backupDataFlag,
                ClickHouseConstant.BACKUP_DATA_TABLE_DOES_NOT_EXIST))) {
                backupTables.put(protectedTable.getName(), backupDataFlag);
            }
        });
        extendInfo.put(ClickHouseConstant.BACKUP_TABLES_KEY, backupTables.toString());
        extendInfo.put(ClickHouseConstant.BACKUP_DATABASE_KEY,
            isBackupDatabaseObject ? protectObject.getName() : protectObject.getParentName());
        return taskEnvironment;
    }

    private String getBackupDataFlag(Map<String, List<ProtectedResource>> tableNodeMap, ProtectedResource nodeResource,
        ProtectedResource protectedTable) {
        String backupDataFlag = ClickHouseConstant.BACKUP_DATA_NO_BACKUP;
        if (protectedTable.getExtendInfo().containsKey(ClickHouseConstant.BACKUP_TABLES_BACKUP_DATA_KEY)) {
            backupDataFlag = MapUtils.getString(protectedTable.getExtendInfo(),
                ClickHouseConstant.BACKUP_TABLES_BACKUP_DATA_KEY);
        } else {
            if (StringUtils.endsWithIgnoreCase(protectedTable.getExtendInfoByKey(ClickHouseConstant.TABLE_ENGINE),
                ClickHouseConstant.MERGE_TREE_ENGINE)) {
                if (StringUtils.equalsIgnoreCase(ClickHouseConstant.REPLICATED_MERGE_TREE_ENGINE,
                    protectedTable.getExtendInfoByKey(ClickHouseConstant.TABLE_ENGINE))) {
                    List<ProtectedResource> backupNodes = tableNodeMap.get(protectedTable.getName());
                    Optional<ProtectedResource> protectedResourceOptional = backupNodes.stream()
                        .filter(item ->
                            StringUtils.equals(MapUtils.getString(item.getExtendInfo(), ClickHouseConstant.IP),
                                MapUtils.getString(nodeResource.getExtendInfo(), ClickHouseConstant.IP))
                                && StringUtils.equals(MapUtils.getString(item.getExtendInfo(), ClickHouseConstant.IP),
                                MapUtils.getString(nodeResource.getExtendInfo(), ClickHouseConstant.IP)))
                        .findFirst();
                    backupDataFlag = protectedResourceOptional.isPresent()
                        ? ClickHouseConstant.BACKUP_DATA_NO_BACKUP
                        : ClickHouseConstant.BACKUP_DATA_BACKUP;
                } else {
                    backupDataFlag = ClickHouseConstant.BACKUP_DATA_BACKUP;
                }
            }
        }
        return backupDataFlag;
    }

    /**
     * 是否备份的数据库
     *
     * @param backupTask backupTask
     * @return true：数据库/false：表集
     */
    private boolean isBackupDatabaseObject(BackupTask backupTask) {
        // 识别是备份库还是表集
        String resourceType = backupTask.getProtectObject().getType();
        if (ClickHouseConstant.DATABASE_TYPE.equals(resourceType)) {
            return true;
        } else if (ClickHouseConstant.TABLE_SET_TYPE.equals(resourceType)) {
            return false;
        } else {
            throw new LegoCheckedException("illegal resource type: " + resourceType);
        }
    }
}
