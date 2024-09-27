package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobCommonProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * ExchangeDatabaseJobProvider
 * <p>
 * 拦截exchange database备份，添加区分数据库属于单机和DAG参数
 *
 * @author w30032137
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-17
 */
@Slf4j
@Component
@AllArgsConstructor
public class ExchangeDatabaseJobProvider implements JobCommonProvider {
    private final ResourceService resourceService;

    @Override
    public boolean applicable(String subtype) {
        return ResourceSubTypeEnum.EXCHANGE_DATABASE.equalsSubType(subtype);
    }

    @Override
    public void intercept(Job insertJob) {
        if (isAgentLessBackup(insertJob)) {
            ProtectedResource resource = resourceService.getResourceById(insertJob.getSourceId())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                    "Protected resource not exist. uuid: " + insertJob.getSourceId()));
            String isGroup = resource.getEnvironment().getExtendInfo().get("isGroup");
            JSONObject extendStr = JSONObject.fromObject(insertJob.getExtendStr());
            extendStr.put("isGroup", isGroup);
            insertJob.setExtendStr(extendStr.toString());
        }
    }

    private boolean isAgentLessBackup(Job insertJob) {
        return JobTypeEnum.BACKUP.getValue().equals(insertJob.getType())
            && ResourceSubTypeEnum.EXCHANGE_DATABASE.equalsSubType(insertJob.getSourceSubType());
    }
}
