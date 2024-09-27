package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * 任务资源的mock工具类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/11
 **/
public class TaskResourceMocker {

    public static TaskResource mockFullInfo(){
        TaskResource resource = new TaskResource();
        resource.setUuid(UUID.randomUUID().toString());
        resource.setName("test_resource");
        resource.setType("Database");
        resource.setSubType("Oracle");
        resource.setParentName("parent_test_resource");
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setRootUuid(UUID.randomUUID().toString());
        resource.setPath("/parent/children");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("key1", "value1");
        extendInfo.put("key2", "value2");
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
