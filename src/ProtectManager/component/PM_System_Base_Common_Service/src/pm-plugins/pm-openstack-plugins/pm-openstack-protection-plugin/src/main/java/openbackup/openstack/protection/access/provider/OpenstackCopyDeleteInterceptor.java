package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Optional;

/**
 * Openstack副本删除拦截器
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-14
 */
@Slf4j
@Component
public class OpenstackCopyDeleteInterceptor implements CopyDeleteInterceptor {
    private final OpenstackQuotaService quotaService;
    private final ResourceService resourceService;

    public OpenstackCopyDeleteInterceptor(OpenstackQuotaService quotaService, ResourceService resourceService) {
        this.quotaService = quotaService;
        this.resourceService = resourceService;
    }

    @Override
    public void finalize(Copy copy, TaskCompleteMessageBo taskMessage) {
        log.info("openstack copy delete post process, jobId:{}", taskMessage.getJobId());
        Optional.ofNullable(copy.getResourceId())
            .flatMap(resourceService::getResourceById)
            .ifPresent(res -> resourceService.getResourceById(res.getRootUuid())
                .filter(quotaService::isRegisterOpenstack)
                .ifPresent(env -> quotaService.updateUsedQuota(res.getParentUuid(), copy, UpdateQuotaType.REDUCE))
            );
    }

    @Override
    public void initialize(DeleteCopyTask task, CopyInfoBo copy) {
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(subType);
    }
}
