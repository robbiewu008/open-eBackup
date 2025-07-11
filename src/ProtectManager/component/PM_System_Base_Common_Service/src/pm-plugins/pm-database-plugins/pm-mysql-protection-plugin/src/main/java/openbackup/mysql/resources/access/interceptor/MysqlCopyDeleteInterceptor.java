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
package openbackup.mysql.resources.access.interceptor;

import static openbackup.data.access.framework.copy.mng.util.CopyUtil.getCopiesBetweenTwoCopy;
import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.LOG;

import com.alibaba.fastjson.JSONObject;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Collectors;

/**
 * MySQL副本删除
 *
 */
@Slf4j
@Component
public class MysqlCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * Constructor
     *
     * @param copyRestApi     copyRestApi
     * @param resourceService resourceService
     */
    public MysqlCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    /**
     * 适用于MySQL单实例，集群实例，数据库
     *
     * @param subType 子类型
     * @return 是否适用于
     */
    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType(),
                ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(),
                ResourceSubTypeEnum.MYSQL_DATABASE.getType()).contains(subType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        log.info("mysql full copies:{}, thisCopy:{}, nextFullCopy:{}", copies, thisCopy, nextFullCopy);
        if (isEAppCopy(thisCopy)) {
            return Lists.newArrayList(thisCopy.getUuid());
        }
        List<Copy> copyList = copyRestApi.queryCopiesByResourceId(thisCopy.getResourceId());
        log.info("copyList:{}", copyList);
        List<String> filterCopyUuids = copyList.stream()
            .filter(copy -> StringUtils.equals(copy.getGeneratedBy(), thisCopy.getGeneratedBy()))
            .filter(copy -> {
                if (nextFullCopy == null) {
                    return copy.getGn() > thisCopy.getGn();
                }
                return copy.getGn() > thisCopy.getGn() && copy.getGn() < nextFullCopy.getGn();
            })
            .sorted(Comparator.comparingInt(Copy::getGn))
            .map(Copy::getUuid)
            .collect(Collectors.toList());
        log.info("filterCopies:{}", filterCopyUuids);
        return filterCopyUuids;
    }

    private boolean isContainsMorePreviousCopy(Copy thisCopy, int backupType) {
        List<Copy> copies = copyRestApi.queryCopiesByResourceId(thisCopy.getResourceId());
        log.info("queryCopies:{}", copies);
        return copies.stream()
                .anyMatch(copy -> copy.getBackupType() == backupType
                        && copy.getGn() < thisCopy.getGn());
    }

    /**
     * 删除增量副本时，要删除此副本到下一个全量副本之间的所有副本
     *
     * @param copies       本个副本之后的所有备份副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        log.info("mysql incr copies:{}, thisCopy:{}, nextFullCopy:{}", copies, thisCopy, nextFullCopy);
        if (isEAppCopy(thisCopy)) {
            return Lists.newArrayList(thisCopy.getUuid());
        }
        if (isContainsMorePreviousCopy(thisCopy, BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType())) {
            return Lists.newArrayList(thisCopy.getUuid());
        }
        List<Copy> copiesBetweenTwoCopy = getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        log.info("copiesBetweenTwoCopy:  {}", copiesBetweenTwoCopy);
        List<Copy> associatedLogs = getAssociatedLogs(copiesBetweenTwoCopy);
        log.info("associatedLogs:  {}", associatedLogs);
        return associatedLogs.stream()
                .map(Copy::getUuid)
                .collect(Collectors.toList());
    }

    private static List<Copy> getAssociatedLogs(List<Copy> copies) {
        copies.sort(Comparator.comparingInt(Copy::getGn));
        List<Copy> logs = new ArrayList<>();
        for (Copy copy : copies) {
            BackupTypeConstants copyBackupType = BackupTypeConstants.getBackupTypeByAbBackupType(
                    copy.getBackupType());
            if (StringUtils.equals(copyBackupType.getBackupType(), LOG.getBackupType())) {
                logs.add(copy);
                continue;
            }
            break;
        }
        return logs;
    }

    /**
     * 删除差异副本时，要删除此副本到下一个全量副本之间的所有副本
     *
     * @param copies       本个副本之后的所有备份副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        log.info("mysql diff copies:{}, thisCopy:{}, nextFullCopy:{}", copies, thisCopy, nextFullCopy);
        if (isEAppCopy(thisCopy)) {
            return Lists.newArrayList(thisCopy.getUuid());
        }
        if (isContainsMorePreviousCopy(thisCopy, BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType())) {
            return Lists.newArrayList(thisCopy.getUuid());
        }
        List<Copy> copiesBetweenTwoCopy = getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        log.info("copiesBetweenTwoCopy:  {}", copiesBetweenTwoCopy);
        List<Copy> associatedLogs = getAssociatedLogs(copiesBetweenTwoCopy);
        log.info("associatedLogs:  {}", associatedLogs);
        return associatedLogs.stream()
                .map(Copy::getUuid)
                .collect(Collectors.toList());
    }

    /**
     * 不下发Agents
     *
     * @param copy 副本信息
     * @param task 删除任务
     * @return 是否下发
     */
    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    private boolean isEAppCopy(Copy thisCopy) {
        JSONObject resourceProperties = JSONObject.parseObject(thisCopy.getResourceProperties());
        JSONObject extendInfo = resourceProperties.getJSONObject(DatabaseConstants.EXTEND_INFO);
        if (MapUtils.isEmpty(extendInfo)) {
            return false;
        }
        return MapUtils.isNotEmpty(extendInfo) && MysqlConstants.EAPP.equals(
                extendInfo.getString(DatabaseConstants.CLUSTER_TYPE));
    }
}
