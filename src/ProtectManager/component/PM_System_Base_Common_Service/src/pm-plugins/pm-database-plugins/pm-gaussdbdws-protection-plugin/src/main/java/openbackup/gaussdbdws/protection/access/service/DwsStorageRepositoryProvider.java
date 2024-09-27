package openbackup.gaussdbdws.protection.access.service;

import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryProvider;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.ImmutableList;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * DWS
 *
 * @author w30044259
 * @since 2024-03-26
 */
@Component
@AllArgsConstructor
@Slf4j
public class DwsStorageRepositoryProvider implements StorageRepositoryProvider {
    private static final ImmutableList<ResourceSubTypeEnum> DWS_SUBTYPES =
        ImmutableList.of(ResourceSubTypeEnum.GAUSSDB_DWS, ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE,
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE, ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA);

    private final BackupStorageApi backupStorageApi;

    private final StorageRepositoryCreateService storageRepositoryCreateService;

    @Override
    public List<StorageRepository> buildBackupDataRepository(BackupObject backupObject) {
        log.info("Dws parallel is enabled.Job_id: {}", backupObject.getRequestId());
        return storageRepositoryCreateService.createRepositoryByStorageUnitGroup(backupObject.getPolicy()
            .getExtParameters()
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY)
            .textValue());
    }

    @Override
    public boolean applicable(BackupObject object) {
        // DWS开启并行存储后，需要单独处理
        if (DWS_SUBTYPES.stream()
            .noneMatch(subType -> subType.equalsSubType(object.getProtectedObject().getSubType()))) {
            return false;
        }
        JsonNode extParameters = object.getPolicy().getExtParameters();
        if (!isBoundStorageUnitGroup(extParameters)) {
            return false;
        }
        return backupStorageApi.getDetail(extParameters.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)
            .get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY)
            .textValue()).isHasEnableParallelStorage();
    }

    private boolean isBoundStorageUnitGroup(JsonNode extParameters) {
        if (!extParameters.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)) {
            return false;
        }
        JsonNode storageInfo = extParameters.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        return storageInfo.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY)
            && BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE
                .equals(storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY).textValue());
    }
}
