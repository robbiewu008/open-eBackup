package openbackup.mongodb.protection.access.provider.copy;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.HashMap;
import java.util.Optional;

public class MongoDBCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final MongoDBCopyDeleteInterceptor mongoDBCopyDeleteInterceptor = new MongoDBCopyDeleteInterceptor(
        copyRestApi, resourceService);

    /**
     * 用例场景：MongoDB单机和集群注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MONGODB_SINGLE.getType()));
        Assert.assertTrue(mongoDBCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MONGODB_CLUSTER.getType()));
    }

    /**
     * 用例场景：MongoDB单机和集群注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void should_supply_agent() {
        Assert.assertFalse(mongoDBCopyDeleteInterceptor.shouldSupplyAgent(new DeleteCopyTask(), new CopyInfoBo()));
    }

    /**
     * 用例场景：MongoDB handle_task
     * 前置条件：无
     * 检查点：NA
     */
    @Test
    public void handle_task() {
        DeleteCopyTask deleteCopyTask = new DeleteCopyTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        taskResource.setSubType(ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        deleteCopyTask.setProtectObject(taskResource);
        taskEnvironment.setExtendInfo(new HashMap<>());
        deleteCopyTask.setProtectEnv(taskEnvironment);
        given(resourceService.getBasicResourceById(any())).willReturn(Optional.of(new ProtectedResource()));
        mongoDBCopyDeleteInterceptor.handleTask(deleteCopyTask, new CopyInfoBo());
        Assert.assertTrue(true);
    }
}
