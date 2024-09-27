package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.common.util.StorageRepositoryUtil;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Unified Replication Provider
 *
 * @author l00272247
 * @since 2022-01-30
 */
@Slf4j
@Component
public class UnifiedReplicationProvider extends AdvanceReplicationProvider {
    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    /**
     * build copy info
     *
     * @param copy copy
     * @param backupId backupId
     * @return copy properties
     */
    @Override
    protected JSONObject buildCopyProperties(CopyInfoBo copy, String backupId) {
        JSONObject properties = super.buildCopyProperties(copy, backupId);
        DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(backupId);
        properties.put(CopyPropertiesKeyConstant.SNAPSHOTS, copyInfo.getSnapshots());
        List<BaseStorageRepository> repositories = copyInfo.getRepositories().stream()
                .map(this::castStorageRepositoryToBaseStorageRepository).collect(Collectors.toList());
        if (copyInfo.getExtendInfo() != null) {
            properties.putAll(copyInfo.getExtendInfo());
        }
        properties.put(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositories);
        properties.put(CopyPropertiesKeyConstant.KEY_EXTEND_INFO, copyInfo.getExtendInfo());
        // 添加副本格式
        properties.put(CopyPropertiesKeyConstant.KEY_FORMAT, copyInfo.getFormat());
        // 添加副本校验状态
        properties.put(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS, copyInfo.getCopyVerifyStatus().getVerifyStatus());
        // 添加副本逻辑大小/缩减前数据量
        properties.put(CopyPropertiesKeyConstant.SIZE, copyInfo.getSize());
        return properties;
    }

    private BaseStorageRepository castStorageRepositoryToBaseStorageRepository(StorageRepository repository) {
        repository.setProtocol(StorageRepositoryUtil.getRepositoryProtocol(repository));
        return BeanTools.copy(repository, BaseStorageRepository::new);
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return false;
    }
}
