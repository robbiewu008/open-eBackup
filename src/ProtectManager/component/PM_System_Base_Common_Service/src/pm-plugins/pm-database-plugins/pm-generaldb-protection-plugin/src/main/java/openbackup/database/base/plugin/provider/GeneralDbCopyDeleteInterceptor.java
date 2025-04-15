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
package openbackup.database.base.plugin.provider;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import java.util.Collections;
import java.util.stream.Collectors;

/**
 * 通用数据库的副本删除逻辑
 *
 */
@Component
public class GeneralDbCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    private final GeneralDbProtectAgentService generalDbProtectAgentService;

    /**
     * GeneralDbCopyDeleteInterceptor构造器
     *
     * @param copyRestApi 副本rest api
     * @param resourceService 资源服务
     * @param generalDbProtectAgentService 通用数据库agent服务
     */
    public GeneralDbCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService,
        GeneralDbProtectAgentService generalDbProtectAgentService) {
        super(copyRestApi, resourceService);
        this.generalDbProtectAgentService = generalDbProtectAgentService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object);
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        ProtectedResource protectedResource = JsonUtil.read(copy.getResourceProperties(), ProtectedResource.class);
        AppConf appConf = GeneralDbUtil.getAppConf(
            protectedResource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF)).orElse(null);
        List<AppConf.Copy.Delete> deletes = Optional.ofNullable(appConf)
            .map(AppConf::getCopy)
            .map(AppConf.Copy::getDeletes)
            .orElse(Collections.emptyList());
        boolean isDeleteWithAgentFromConf = false;
        String switchConfBackupType = switchConfBackupType(copy.getBackupType());
        for (AppConf.Copy.Delete delete : deletes) {
            List<String> backupTypes = delete.getBackupType();
            if (VerifyUtil.isEmpty(backupTypes)) {
                continue;
            }
            if (GeneralDbUtil.isListContainsElemWithoutCase(backupTypes, switchConfBackupType)) {
                isDeleteWithAgentFromConf = isDeleteWithAgentFromConf || delete.getIsDeleteWithAgent();
            }
        }

        return isDeleteWithAgentFromConf && super.shouldSupplyAgent(task, copy);
    }

    @Override
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        ProtectedResource protectedResource = JsonUtil.read(copy.getResourceProperties(), ProtectedResource.class);
        List<Endpoint> agents = generalDbProtectAgentService.select(protectedResource);
        task.setAgents(agents);
    }

    /**
     * 转化为conf的备份类型
     *
     * @param backupType 备份类型，数字
     * @return 备份类型，conf
     */
    private String switchConfBackupType(int backupType) {
        return BackupTypeConstants.convert2BackupType(backupType);
    }

    private List<AppConf.Copy.Delete> getDeletesFromResourceProperties(String resourceProperties) {
        ProtectedResource protectedResource = JsonUtil.read(resourceProperties, ProtectedResource.class);
        AppConf appConf = GeneralDbUtil.getAppConf(
            protectedResource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF)).orElse(null);
        return Optional.ofNullable(appConf)
            .map(AppConf::getCopy)
            .map(AppConf.Copy::getDeletes)
            .orElse(Collections.emptyList());
    }

    /**
     * SAP HANA 删除全量副本时，要删除此副本到下一个全量副本之间的副本
     *
     * @param copies 此副本之后的所有副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsFullIsSAPHANA(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 前一个日志副本
        Copy previousLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
                thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation()).orElse(null);
        // 后一个日志副本
        Copy nextLogBackupCopy = copies.stream()
                .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
                .findFirst().orElse(null);
        // 如果之前没有日志副本或者之后没有日志副本
        if (previousLogBackupCopy == null || nextLogBackupCopy == null) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        long previousLogCopyEndTime = Long.parseLong(JSONObject.fromObject(previousLogBackupCopy.getProperties())
                .getString(DatabaseConstants.LOG_COPY_END_TIME_KEY));
        long nextLogCopyStartTime = Long.parseLong(JSONObject.fromObject(nextLogBackupCopy.getProperties())
                .getString(DatabaseConstants.LOG_COPY_BEGIN_TIME_KEY));
        // 如果后一个日志副本连不上前一个日志副本
        if (previousLogCopyEndTime < nextLogCopyStartTime) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        List<BackupTypeConstants> associatedTypes = new ArrayList<>(Arrays.asList(
                BackupTypeConstants.DIFFERENCE_INCREMENT, BackupTypeConstants.CUMULATIVE_INCREMENT));
        return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, associatedTypes);
    }

    /**
     * SAP HANA 删除增量副本时，要删除删除下一个全量副本前所有增量副本
     *
     * @param copies 此副本之后的所有副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsDifferenceIncrementIsSAPHANA(
        List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> differenceCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(differenceCopies, thisCopy, nextFullCopy);
    }

    /**
     * SAP HANA 删除差异副本时，要删除删除下一个全量副本前所有增量副本
     *
     * @param copies 此副本之后的所有副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    protected List<String> getCopiesCopyTypeIsCumulativeIncrementIsSAPHANA(
        List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> differenceCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(differenceCopies, thisCopy, nextFullCopy);
    }

    private List<String> getShouldDeleteCopiesIsSAPHANA(List<Copy> copies, Copy thisCopy, String backupType,
                                                          Copy nextFullCopy) {
        if (BackupTypeConstants.FULL.getBackupType().equals(backupType)) {
            return getCopiesCopyTypeIsFullIsSAPHANA(copies, thisCopy, nextFullCopy);
        } else if (BackupTypeConstants.DIFFERENCE_INCREMENT.getBackupType().equals(backupType)) {
            return getCopiesCopyTypeIsDifferenceIncrementIsSAPHANA(copies, thisCopy, nextFullCopy);
        } else if (BackupTypeConstants.CUMULATIVE_INCREMENT.getBackupType().equals(backupType)) {
            return getCopiesCopyTypeIsCumulativeIncrementIsSAPHANA(copies, thisCopy, nextFullCopy);
        } else {
            return Collections.emptyList();
        }
    }

    /**
     * getShouldDeleteCopies
     *
     * @param copies copies 本个副本之后的所有备份副本
     * @param thisCopy thisCopy 本个副本
     * @return List<String> 需要删除的集合
     */
    @Override
    protected List<String> getShouldDeleteCopies(List<Copy> copies, Copy thisCopy) {
        List<String> copyUuids = new ArrayList<>();
        String switchConfBackupType = switchConfBackupType(thisCopy.getBackupType());
        Copy nextFullCopy = CopyUtil.getNextFullCopy(copies, thisCopy.getGn());
        ProtectedResource protectedResource = JsonUtil.read(thisCopy.getResourceProperties(), ProtectedResource.class);
        AppConf appConf = GeneralDbUtil.getAppConf(protectedResource.getExtendInfoByKey(
                GeneralDbConstant.EXTEND_SCRIPT_CONF)).orElse(null);
        String databaseType = Optional.ofNullable(appConf).map(AppConf::getDatabaseType).orElse("");
        if (databaseType.equals(GeneralDbConstant.DATABASE_TYPE_DISPLAY_SAP_HANA)) {
            return getShouldDeleteCopiesIsSAPHANA(copies, thisCopy, switchConfBackupType, nextFullCopy);
        }
        List<AppConf.Copy.Delete> deletes = getDeletesFromResourceProperties(thisCopy.getResourceProperties());
        for (AppConf.Copy.Delete delete : deletes) {
            List<String> backupTypes = delete.getBackupType();
            if (VerifyUtil.isEmpty(backupTypes)) {
                continue;
            }
            if (GeneralDbUtil.isListContainsElemWithoutCase(backupTypes, switchConfBackupType)) {
                List<String> associatedTypes = Optional.ofNullable(delete.getAssociatedType())
                    .orElse(Collections.emptyList());
                copyUuids.addAll(getAssociatedTypeCopies(copies, thisCopy, nextFullCopy, associatedTypes));
                break;
            }
        }
        return copyUuids;
    }

    private List<String> getAssociatedTypeCopies(List<Copy> copies, Copy thisCopy, Copy nextFullCopy,
        List<String> associatedTypes) {
        if (VerifyUtil.isEmpty(associatedTypes)) {
            return Collections.emptyList();
        }

        List<Copy> confContainCopies = copies.stream().filter(copy -> {
            String switchConfBackupType = switchConfBackupType(copy.getBackupType());
            return GeneralDbUtil.isListContainsElemWithoutCase(associatedTypes, switchConfBackupType);
        }).collect(Collectors.toList());

        return CopyUtil.getCopyUuidsBetweenTwoCopy(confContainCopies, thisCopy, nextFullCopy);
    }
}
