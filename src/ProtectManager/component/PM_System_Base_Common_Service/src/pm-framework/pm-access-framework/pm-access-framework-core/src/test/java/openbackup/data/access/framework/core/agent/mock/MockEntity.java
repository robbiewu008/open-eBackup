/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.framework.core.agent.mock;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.PluginsDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportApplicationDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.SupportPluginDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import java.util.Arrays;
import java.util.UUID;

/**
 * 单元测试使用的数据结构
 *
 * @author w00616953
 * @since 2021-12-07
 */
public class MockEntity {
    public static ProtectedEnvironment mockProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();

        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8080);
        return protectedEnvironment;
    }

    public static HostDto mockHostDto() {
        HostDto hostDto = new HostDto();
        hostDto.setName("host dto");
        return hostDto;
    }

    public static PluginsDto mockPluginsDto() {
        PluginsDto pluginsDto = new PluginsDto();

        SupportPluginDto firstPlugin = mockSupportPluginDto();
        SupportPluginDto secondPlugin = mockSupportPluginDto();

        secondPlugin.setSupportApplications(Arrays.asList(mockSupportApplicationDto(), mockSupportApplicationDto()));
        secondPlugin.setPluginName("name_test");

        pluginsDto.setUuid(UUID.randomUUID().toString());
        pluginsDto.setSupportPlugins(Arrays.asList(firstPlugin, secondPlugin));

        return pluginsDto;
    }

    public static SupportPluginDto mockSupportPluginDto() {
        SupportPluginDto supportPluginDto = new SupportPluginDto();

        supportPluginDto.setPluginName("name");
        supportPluginDto.setPluginVersion("version1");

        SupportApplicationDto application1 = mockSupportApplicationDto();
        SupportApplicationDto application2 = mockSupportApplicationDto();
        application2.setApplication("type2");
        SupportApplicationDto application3 = mockSupportApplicationDto();
        application3.setApplication("type3");

        supportPluginDto.setSupportApplications(Arrays.asList(application1, application2, application3));
        return supportPluginDto;

    }

    public static SupportApplicationDto mockSupportApplicationDto() {
        SupportApplicationDto application = new SupportApplicationDto();
        application.setApplication("type1");

        return application;
    }
}
