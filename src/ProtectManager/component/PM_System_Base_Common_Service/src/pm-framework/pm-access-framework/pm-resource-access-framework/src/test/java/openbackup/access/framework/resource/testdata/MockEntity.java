/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.access.framework.resource.testdata;

import openbackup.access.framework.resource.dto.ProtectedEnvironmentDto;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * mock数据类型
 *
 * @author w00616953
 * @since 2021-12-17
 */
public class MockEntity {
    public static ProtectedEnvironmentDto mockProtectedEnvironmentDto() {
        ProtectedEnvironmentDto protectedEnvironmentDto = new ProtectedEnvironmentDto();
        protectedEnvironmentDto.setUuid("decf8540-dbeb-42c3-870c-fe377b415485");
        protectedEnvironmentDto.setType("Host");
        protectedEnvironmentDto.setSubType("UBackupAgent");
        protectedEnvironmentDto.setEndpoint("8.40.97.197");
        protectedEnvironmentDto.setPort(59538);
        protectedEnvironmentDto.setName("testName");
        protectedEnvironmentDto.setOsType("RedHat");
        protectedEnvironmentDto.setUsername("testUserName");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "1");
        protectedEnvironmentDto.setExtendInfo(extendInfo);
        return protectedEnvironmentDto;
    }

    public static PageListResponse<ProtectedResource> mockEmptyPageListResponse() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        response.setRecords(Collections.emptyList());

        return response;
    }

    public static PageListResponse<ProtectedResource> mockPageListResponse() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource = mockProtectedResource();
        response.setTotalCount(1);
        response.setRecords(Arrays.asList(resource));

        return response;
    }

    public static ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("decf8540-dbeb-42c3-870c-fe377b415485");
        resource.setType("Host");
        resource.setSubType("UBackupAgent");
        resource.setName("testName");

        ProtectedEnvironment environment = mockProtectedEnvironment();
        resource.setEnvironment(environment);

        return resource;
    }

    public static ProtectedEnvironment mockProtectedEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("test1");
        environment.setName("TestEnv");
        environment.setEndpoint("8.40.97.197");
        environment.setPort(59538);
        Map<String, String> extendInfo = new LinkedHashMap<>();
        extendInfo.put("scenario", "0");
        environment.setExtendInfo(extendInfo);
        environment.setOsType("RedHat");
        environment.setUsername("testUserName");
        environment.setPath(environment.getName());
        return environment;
    }

    public static ProtectedEnvironment mockEnvironmentWithDependency() {
        ProtectedEnvironment env = mockProtectedEnvironment();
        env.setType("Container");
        env.setSubType("Kubernetes");
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource agent = mockAgentResource();
        dependency.put("agents", Collections.singletonList(agent));
        env.setDependencies(dependency);
        env.setUserId("Test-User-Id");
        env.setAuthorizedUser("Test-User-Name");
        return env;
    }

    public static ProtectedEnvironment mockAgentResource() {
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("1ee1b71f72294f15b55a4555ff30195e");
        agent.setEndpoint("127.0.0.1");
        agent.setPort(8888);
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return agent;
    }
}
