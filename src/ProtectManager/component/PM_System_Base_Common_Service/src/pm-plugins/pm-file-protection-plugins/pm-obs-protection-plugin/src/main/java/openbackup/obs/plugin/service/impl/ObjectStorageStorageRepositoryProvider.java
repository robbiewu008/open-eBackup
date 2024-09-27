package openbackup.obs.plugin.service.impl;

import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryProvider;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.BooleanUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author w30044259
 * @since 2024-03-26
 */
@Component
@Slf4j
public class ObjectStorageStorageRepositoryProvider implements StorageRepositoryProvider {
    @Override
    public List<StorageRepository> buildBackupDataRepository(BackupObject backupObject) {
        // 并行备份暂时被屏蔽
        return Collections.emptyList();
    }

    private boolean isMultiNodeBackup(BackupObject backupObject) {
        JSONObject jsonObject = JSONObject.fromObject(backupObject.getProtectedObject().getExtParameters());
        Map multiBackSwitchMap = backupObject.getProtectedObject().getExtParameters();
        if (multiBackSwitchMap == null) {
            log.warn("isMultiNodeBackup is false. taskId is :{}", backupObject.getTaskId());
            return false;
        }
        Boolean isMultiBackupSwitch = jsonObject.getBoolean(BackupConstant.MULTI_NODE_BACKUP_SWITCH, false);
        return BooleanUtils.isTrue(isMultiBackupSwitch);
    }

    @Override
    public boolean applicable(BackupObject backupObject) {
        // 对象存储如果开启了并行备份，需要插件自己组装repository，未开走默认框架逻辑
        if (!StringUtils.equals(ResourceSubTypeEnum.OBJECT_STORAGE.getType(),
            backupObject.getProtectedObject().getSubType())) {
            return false;
        }
        return isMultiNodeBackup(backupObject);
    }
}
