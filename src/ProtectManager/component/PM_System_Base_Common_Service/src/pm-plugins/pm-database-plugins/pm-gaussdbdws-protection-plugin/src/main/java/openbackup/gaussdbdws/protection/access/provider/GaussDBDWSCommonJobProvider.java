package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.backup.constant.BackupConstant;
import com.huawei.oceanprotect.job.sdk.JobCommonProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * GaussDBDWSCommonJobProvider
 *
 * @author l00626951
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-20
 */
@Slf4j
@Component
@AllArgsConstructor
public class GaussDBDWSCommonJobProvider implements JobCommonProvider {
    private BackupStorageApi backupStorageApi;

    @Override
    public boolean applicable(String subtype) {
        return ResourceSubTypeEnum.GAUSSDB_DWS.equalsSubType(subtype) || ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA
                .equalsSubType(subtype) || ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.equalsSubType(subtype);
    }

    @Override
    public void intercept(Job insertJob) {
        if (isDWSBackup(insertJob)) {
            JSONObject extendStr = JSONObject.fromObject(insertJob.getExtendStr());
            JSONObject triggerPolicy = extendStr.getJSONObject("triggerPolicy");
            JSONArray policyList = triggerPolicy.getJSONArray("policy_list");
            JSONObject storageInfo = policyList.getJSONObject(0).getJSONObject("ext_parameters")
                    .getJSONObject(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
            String storageType = storageInfo.getString(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY);
            if (BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE.equals(storageType)) {
                String storageId = storageInfo.getString(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY);
                NasDistributionStorageDetail storageUnitGroup = backupStorageApi.getDetail(storageId);
                if (storageUnitGroup.isHasEnableParallelStorage()) {
                    storageInfo.put("storageUnitGroupName", storageUnitGroup.getName());
                    insertJob.setExtendStr(extendStr.toString());
                }
            }
        }
    }

    private boolean isDWSBackup(Job insertJob) {
        return JobTypeEnum.BACKUP.getValue().equals(insertJob.getType()) && (ResourceSubTypeEnum.GAUSSDB_DWS
                .equalsSubType(insertJob.getSourceSubType()) || ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA
                .equalsSubType(insertJob.getSourceSubType()) || ResourceSubTypeEnum.GAUSSDB_DWS_TABLE
                .equalsSubType(insertJob.getSourceSubType()));
    }
}
