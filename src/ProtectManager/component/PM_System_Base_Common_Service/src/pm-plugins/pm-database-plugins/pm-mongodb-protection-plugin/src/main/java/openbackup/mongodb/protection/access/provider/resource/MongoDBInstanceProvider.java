/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mongodb.protection.access.provider.resource;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.enums.MongoDBClusterTypeEnum;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.mongodb.protection.access.util.MongoDBConstructionUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * mongodb单实例provider
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
@Component
@Slf4j
public class MongoDBInstanceProvider extends DatabaseEnvironmentProvider {
    private final JsonSchemaValidator jsonSchemaValidator;

    private final MongoDBBaseService mongoDBBaseService;

    /**
     * 构造方法
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param mongoDBBaseService mongodb 实际业务service
     */
    public MongoDBInstanceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        JsonSchemaValidator jsonSchemaValidator, MongoDBBaseService mongoDBBaseService) {
        super(providerManager, pluginConfigManager);
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.mongoDBBaseService = mongoDBBaseService;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start create mongodb instance parameters check. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.MONGODB_SINGLE.getType());
        environment.setExtendInfoByKey(MongoDBConstants.AGENT_CLUSTE_TYPE, MongoDBClusterTypeEnum.SINGLE.getType());
        mongoDBBaseService.checkMongoDBEnvironmentSize(environment, VerifyUtil.isEmpty(environment.getUuid()));
        String agentUuid = environment.getExtendInfoByKey(MongoDBConstants.AGENT_UUID);
        if (StringUtils.isEmpty(agentUuid)) {
            log.error("This MongoDB instance is not agent uuid");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB instance is not agent uuid");
        }
        ProtectedEnvironment hostEnv = mongoDBBaseService.getEnvironmentById(agentUuid);
        if ("windows".equals(hostEnv.getOsType())) {
            throw new LegoCheckedException(DatabaseErrorCode.ENV_OS_TYPE_ERROR,
                new String[] {hostEnv.getEndpoint(), hostEnv.getOsType()},
                "This agent do not need osType");
        }
        if (!VerifyUtil.isEmpty(environment.getAuth())
            && environment.getAuth().getAuthType() == Authentication.APP_PASSWORD) {
            mongoDBBaseService.checkKeyLength(environment.getAuth().getAuthKey(), environment.getAuth().getAuthPwd());
        }
        AppEnvResponse appEnvAgentInfo = mongoDBBaseService.getAppEnvAgentInfo(
            MongoDBConstructionUtils.getProtectedResource(environment), hostEnv);
        if (MongoDBConstants.FAILED_MARK.equals(appEnvAgentInfo.getName())) {
            if (StringUtils.isEmpty(appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_CODE))) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB instance connect agent failed");
            }
            String errorCode = appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_CODE);
            String errorParam = appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_PARAM);
            throw new LegoCheckedException(Long.parseLong(errorCode), errorParam.split(","),
                "This MongoDB instance connect agent failed");
        }
        // 注册成功更改环境信息
        updateEnvironment(environment, appEnvAgentInfo);
        log.info("End create mongodb instance success. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
    }

    private void updateEnvironment(ProtectedEnvironment environment, AppEnvResponse appEnvAgentInfo) {
        List<MongoClusterNodesExtendInfo> singleInfo = new ArrayList<>();
        singleInfo.add(
            MongoDBConstructionUtils.buildMongoClusterNodesExtendInfo(appEnvAgentInfo.getExtendInfo(), new NodeInfo()));
        environment.setVersion(appEnvAgentInfo.getExtendInfo().get(DatabaseConstants.VERSION));
        environment.setPath(appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.LOCAL_HOST));
        environment.setEndpoint(appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.LOCAL_HOST));
        environment.setVersion(appEnvAgentInfo.getExtendInfo().get(DatabaseConstants.VERSION));
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        environment.getExtendInfo().put(MongoDBConstants.CLUSTE_TYPE, MongoDBClusterTypeEnum.SINGLE.getType());
        environment.getExtendInfo().put(DatabaseConstants.VERSION, environment.getVersion());
        environment.getExtendInfo().put(MongoDBConstants.CLUSTER_NODES, JSONArray.fromObject(singleInfo).toString());
    }

    /**
     * MongoDB健康检查并更新状态和资源信息
     *
     * @param environment 资源
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("Start healthCheck MongoDB cluster online. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
        String agentUuid = environment.getExtendInfoByKey(MongoDBConstants.AGENT_UUID);
        AppEnvResponse appEnvAgentInfo = mongoDBBaseService.getAppEnvAgentInfo(
            MongoDBConstructionUtils.getProtectedResource(environment),
            mongoDBBaseService.getEnvironmentById(agentUuid));
        if (MongoDBConstants.FAILED_MARK.equals(appEnvAgentInfo.getName())) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB instance connect agent failed");
        }
        // 注册成功更改环境信息
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setExtendInfo(new HashMap<>());
        newEnv.setUuid(environment.getUuid());
        updateEnvironment(newEnv, appEnvAgentInfo);
        mongoDBBaseService.updateResourceService(newEnv);
        log.info("End healthCheck MongoDB online. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.MONGODB_SINGLE.equalsSubType(resourceSubType);
    }
}