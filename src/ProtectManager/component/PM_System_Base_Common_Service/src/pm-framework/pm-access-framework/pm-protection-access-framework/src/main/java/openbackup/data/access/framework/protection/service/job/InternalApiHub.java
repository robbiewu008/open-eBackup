package openbackup.data.access.framework.protection.service.job;

import openbackup.data.access.framework.protection.service.lock.ResourceLockService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.protection.QosCommonRestApi;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 任务流程中依赖其它模块api操作的集成器.
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/30
 **/
@Slf4j
@Component
public class InternalApiHub {
    private final CopyRestApi copyRestApi;
    private final JobService jobService;
    private final ResourceLockService lockService;
    private final QosCommonRestApi qosCommonRestApi;
    private final ResourceService resourceService;

    /**
     * 内部api集合器构造函数
     *
     * @param copyRestApi 副本服务api
     * @param jobService 任务服务
     * @param lockService 资源锁服务
     * @param qosCommonRestApi qos服务api
     * @param resourceService 资源对象服务
     */
    public InternalApiHub(CopyRestApi copyRestApi, JobService jobService, ResourceLockService lockService,
        QosCommonRestApi qosCommonRestApi, ResourceService resourceService) {
        this.copyRestApi = copyRestApi;
        this.jobService = jobService;
        this.lockService = lockService;
        this.qosCommonRestApi = qosCommonRestApi;
        this.resourceService = resourceService;
    }

    public CopyRestApi getCopyRestApi() {
        return copyRestApi;
    }

    public JobService getJobService() {
        return jobService;
    }

    public ResourceLockService getLockService() {
        return lockService;
    }

    public QosCommonRestApi getQosCommonRestApi() {
        return qosCommonRestApi;
    }

    public ResourceService getResourceService() {
        return resourceService;
    }
}
