package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import org.springframework.beans.BeanUtils;

/**
 * 恢复应用插件的模拟类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/15
 **/
public class MockRestoreInterceptorProvider implements RestoreInterceptorProvider {
    @Override
    public boolean applicable(String object) {
        return true;
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        TaskResource mockResource = new TaskResource();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskResource(), mockResource);
        task.setTargetObject(mockResource);
        return task;
    }
}
