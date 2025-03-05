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
package openbackup.mongodb.protection.access.provider.copy;

import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.LOG;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Objects;
import java.util.Collections;

/**
 * MongoDB 副本删除拦截器
 *
 */
@Slf4j
@Component
public class MongoDBCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * Constructor
     *
     * @param copyRestApi 副本接口
     * @param resourceService resourceService
     */
    public MongoDBCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.MONGODB_CLUSTER.getType(),
                ResourceSubTypeEnum.MONGODB_SINGLE.getType()).contains(subType);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        if (!super.isResourceExists(task)) {
            return;
        }
        TaskEnvironment protectEnv = task.getProtectEnv();
        Map<String, String> envExtendInfo = Optional.of(protectEnv.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        if (ResourceSubTypeEnum.MONGODB_CLUSTER.getType().equals(task.getProtectObject().getSubType())) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        }
        protectEnv.setExtendInfo(envExtendInfo);
        task.setProtectEnv(protectEnv);
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    @Override
    protected List<String> getCopiesCopyTypeIsLog(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, nextFullCopy);
    }

    /**
     * 删除全量副本时，如果当前副本之前有日志副本，则只删除当前副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        Optional<Copy> latestLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
                thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation());
        if (latestLogBackupCopy.isPresent()) {
            log.warn("exist log copy before this copy, uuid is {}", latestLogBackupCopy.get().getUuid());
            return Collections.emptyList();
        }

        int format = CopyUtil.getFormat(thisCopy).orElse(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        if (Objects.equals(format, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat())) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        if (Objects.equals(format, CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat())) {
            return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, Collections.singletonList(LOG));
        }
        return Collections.emptyList();
    }
}
