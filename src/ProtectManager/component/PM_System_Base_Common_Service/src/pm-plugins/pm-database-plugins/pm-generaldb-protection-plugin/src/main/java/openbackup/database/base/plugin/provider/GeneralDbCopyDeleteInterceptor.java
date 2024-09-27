/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 通用数据库的副本删除逻辑
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-09
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

    @Override
    protected List<String> getShouldDeleteCopies(List<Copy> copies, Copy thisCopy) {
        List<String> copyUuids = new ArrayList<>();
        String switchConfBackupType = switchConfBackupType(thisCopy.getBackupType());
        Copy nextFullCopy = CopyUtil.getNextFullCopy(copies, thisCopy.getGn());
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
