package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.springframework.beans.BeanUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * 保护资源信息的Mock类，模拟测试用例中{@code ProtectedResource}对象的各种情况
 *
 * @description:
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/7
 **/
public class ProtectedResourceMocker {
    /**
     * 模拟基础的环境信息，只包含id,ip,port属性
     *
     * @param uuid 资源uuid
     * @param ip ip地址
     * @param port 端口号
     * @return 资源信息{@code ProtectedResource}
     */
    public static ProtectedEnvironment mockEndPointInfo(String uuid, String ip, int port) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(uuid);
        environment.setEndpoint(ip);
        environment.setPort(port);
        environment.setOsType("windows");
        return environment;
    }

    /**
     * 模拟基础的环境信息
     *
     * @return 资源信息
     */
    public static ProtectedEnvironment mockTaskEnv() {
        final ProtectedResource protectedResource = mockCommonResource();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        BeanUtils.copyProperties(protectedResource, environment);
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8080);
        environment.setRootUuid(UUID.randomUUID().toString());
        environment.setPath(File.separator + "parent" + File.separator + "children");
        environment.setLinkStatus("1");
        Authentication auth = new Authentication();
        auth.setAuthType(2);
        auth.setAuthKey("username");
        auth.setAuthPwd("password1111");
        environment.setAuth(auth);
        return environment;
    }

    /**
     * 模拟基础的环境信息
     *
     * @return 资源信息
     */
    public static ProtectedResource mockTaskResource() {
        final ProtectedResource protectedResource = mockCommonResource();
        protectedResource.setParentUuid(UUID.randomUUID().toString());
        protectedResource.setParentName("test_resource_parent");
        return protectedResource;
    }

    /**
     * 模拟基础的环境信息
     *
     * @return 资源信息
     */
    private static ProtectedResource mockCommonResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUID.randomUUID().toString());
        resource.setName("test_resource");
        resource.setType("Database");
        resource.setSubType("Oracle");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("key1", "value1");
        extendInfo.put("key2", "value2");
        resource.setExtendInfo(extendInfo);
        return resource;
    }

    /**
     * 模拟基础的环境信息
     *
     * @param parentId 上级id
     * @return 资源信息
     */
    public static ProtectedResource mockParentResource(String parentId) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid(parentId);
        return protectedResource;
    }

    /**
     * 模拟基础的环境信息
     *
     * @return 资源信息
     */
    public static ProtectedResource mockEnvironmentDependencies() {
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedResource> resourceList = new ArrayList<>();
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setEndpoint("1");
        env.setPort(8080);
        env.setUuid("uuid");
        env.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        resourceList.add(env);

        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put("agents", resourceList);
        protectedResource.setDependencies(dependencies);
        return protectedResource;
    }
}
