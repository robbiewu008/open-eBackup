package openbackup.db2.protection.access.provider.copy;

import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.Map;

/**
 * {@link Db2CopyVerifyInterceptorProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-21
 */
public class Db2CopyVerifyInterceptorProviderTest {
    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final Db2Service db2Service = PowerMockito.mock(Db2Service.class);

    private final CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    Db2CopyVerifyInterceptorProvider provider = new Db2CopyVerifyInterceptorProvider(instanceResourceService, db2Service,
            copyManagerService, copyRestApi);

    /**
     * 用例场景：框架调用到db2的副本校验
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_copy_verify_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：db2副本校验拦截器设置参数
     * 前置条件：参数正常
     * 检查点：参数设置正确
     */
    @Test
    public void execute_interceptor_success() {
        TaskResource taskResource = new TaskResource();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType());
        taskEnvironment.setExtendInfo(extendInfo);
        Mockito.when(copyManagerService.buildTaskResource(Mockito.any())).thenReturn(taskResource);
        Mockito.when(copyManagerService.buildTaskEnvironment(Mockito.any())).thenReturn(taskEnvironment);
        Mockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(new Copy());
        CopyVerifyTask copyVerifyTask = provider.interceptor(mockTask());
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), copyVerifyTask.getTargetEnv().getExtendInfo().get(
            DatabaseConstants.DEPLOY_TYPE));
    }

    private CopyVerifyTask mockTask() {
        CopyVerifyTask task = new CopyVerifyTask();
        task.setTaskId(UUIDGenerator.getUUID());
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUIDGenerator.getUUID());
        task.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        task.setTargetEnv(taskEnvironment);
        return task;
    }
}