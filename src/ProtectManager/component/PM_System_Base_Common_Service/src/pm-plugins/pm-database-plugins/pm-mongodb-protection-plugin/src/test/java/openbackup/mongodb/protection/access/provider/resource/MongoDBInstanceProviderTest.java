/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mongodb.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.mock.MongoDBMockBean;

import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * mongodb单实例provider 测试类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public class MongoDBInstanceProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private final JsonSchemaValidator jsonSchemaValidator = Mockito.mock(JsonSchemaValidator.class);

    private final MongoDBBaseService mongoDBBaseService = Mockito.mock(MongoDBBaseService.class);

    private final MongoDBMockBean mongoDBMockBean = new MongoDBMockBean();

    private final MongoDBInstanceProvider mongoDBInstanceProvider = new MongoDBInstanceProvider(providerManager,
        pluginConfigManager, jsonSchemaValidator, mongoDBBaseService);

    /**
     * 用例场景：MongoDB单机注册下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBInstanceProvider.applicable(ResourceSubTypeEnum.MONGODB_SINGLE.getType()));
    }

    /**
     * 用例场景：MongoDB单机注册成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_success() {
        ProtectedEnvironment protectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(MongoDBConstants.AGENT_UUID, "uuid");
        given(mongoDBBaseService.getEnvironmentById("uuid")).willReturn(protectedEnvironment);
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(
            mongoDBMockBean.getMongoDBAppEnvResponse());
        mongoDBInstanceProvider.register(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：MongoDB单机注册成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test

    public void should_throw_LegoCheckedException_if_agent_uuid_not_exist_when_check() {
        ProtectedEnvironment protectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        Assert.assertThrows(LegoCheckedException.class, () -> mongoDBInstanceProvider.register(protectedEnvironment));
    }

    /**
     * 用例场景：MongoDB单机注册成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_app_env_agent_info_fail_exist_when_check() {
        ProtectedEnvironment protectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(MongoDBConstants.AGENT_UUID, "uuid");
        given(mongoDBBaseService.getEnvironmentById("uuid")).willReturn(protectedEnvironment);
        AppEnvResponse appEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        appEnvResponse.getExtendInfo().put("errorCode", "1677931336");
        appEnvResponse.getExtendInfo().put("errorParam","");
        appEnvResponse.setName("0");
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(appEnvResponse);
        Assert.assertThrows(LegoCheckedException.class, () -> mongoDBInstanceProvider.register(protectedEnvironment));
    }

    /**
     * 用例场景：MongoDB单机健康检查成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment protectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(MongoDBConstants.AGENT_UUID, "uuid");
        given(mongoDBBaseService.getEnvironmentById("uuid")).willReturn(protectedEnvironment);
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(
            mongoDBMockBean.getMongoDBAppEnvResponse());
        mongoDBInstanceProvider.validate(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：MongoDB单机健康检查成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_app_env_agent_info_fail_exist_when_health_check() {
        ProtectedEnvironment protectedEnvironment = mongoDBMockBean.getMongoDBProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(MongoDBConstants.AGENT_UUID, "uuid");
        given(mongoDBBaseService.getEnvironmentById("uuid")).willReturn(protectedEnvironment);
        AppEnvResponse mongoDBAppEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse.setName("0");
        given(mongoDBBaseService.getAppEnvAgentInfo(any(), any())).willReturn(mongoDBAppEnvResponse);
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBInstanceProvider.validate(protectedEnvironment));
    }
}