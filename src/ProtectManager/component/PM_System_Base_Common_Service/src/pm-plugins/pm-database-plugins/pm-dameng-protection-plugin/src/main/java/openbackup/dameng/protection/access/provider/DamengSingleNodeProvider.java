/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.dameng.protection.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.dameng.protection.access.util.DamengParamCheckUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

/**
 * dameng单节点注册
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-13
 */
@Slf4j
@Component
public class DamengSingleNodeProvider extends DatabaseEnvironmentProvider {
    private final DamengService damengService;

    private final ResourceService resourceService;

    /**
     * 构造方法
     *
     * @param providerManager provider管理器
     * @param pluginConfigManager 插件配置管理器
     * @param damengService dameng资源服务接口
     * @param resourceService 资源服务
     */
    public DamengSingleNodeProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        DamengService damengService, ResourceService resourceService) {
        super(providerManager, pluginConfigManager);
        this.damengService = damengService;
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check dameng single node: {}.", environment.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        DamengParamCheckUtil.checkPort(environment.getExtendInfoByKey(DatabaseConstants.PORT));
        DamengParamCheckUtil.checkAuthKey(environment.getAuth().getAuthKey());
        checkNodeExist(environment);
        updateEnvironment(environment);
        environment.setUuid(Optional.ofNullable(environment.getUuid()).orElse(UUIDGenerator.getUUID()));
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("The Dameng healthCheck start, uuid: {}.", environment.getUuid());
        String linkStatus = LinkStatusEnum.ONLINE.getStatus().toString();
        ActionResult actionResult = resourceService.check(environment)[0];
        if (actionResult.getCode() != ActionResult.SUCCESS_CODE) {
            log.error("The dameng single node environment:{} check connection failed.", environment.getUuid());
            linkStatus = LinkStatusEnum.OFFLINE.getStatus().toString();
        }
        updateLinkStatus(environment, linkStatus);
        return Optional.of(linkStatus);
    }

    private void updateLinkStatus(ProtectedEnvironment environment, String linkStatus) {
        NodeInfo nodeInfo = damengService.getNodeInfoFromNodes(environment).get(0);
        nodeInfo.getExtendInfo().put(DamengConstant.INSTANCESTATUS, linkStatus);
        List<NodeInfo> nodeInfoList = Collections.singletonList(nodeInfo);
        Map<String, String> envExtendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(DamengConstant.NODES, JSONObject.writeValueAsString(nodeInfoList));
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        newEnv.setLinkStatus(linkStatus);
        newEnv.setExtendInfo(envExtendInfo);
        resourceService.updateSourceDirectly(Collections.singletonList(newEnv));
    }

    private void updateEnvironment(ProtectedEnvironment environment) {
        Map<String, String> checkMessage = getCheckResult(environment);
        ProtectedEnvironment agentEnv = damengService.queryAgentEnvironment(environment);
        List<NodeInfo> nodeInfo = buildNodeInfo(environment, agentEnv);
        Map<String, String> extendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DamengConstant.NODES, JSONObject.writeValueAsString(nodeInfo));
        extendInfo.put(DamengConstant.DB_PATH, checkMessage.get(DamengConstant.DB_PATH));
        extendInfo.put(DamengConstant.DB_NAME, checkMessage.get(DamengConstant.DB_NAME));
        extendInfo.put(DamengConstant.VERSION, checkMessage.get(DamengConstant.VERSION));
        extendInfo.put(DamengConstant.BIG_VERSION, checkMessage.get(DamengConstant.BIG_VERSION));
        extendInfo.put(DamengConstant.DM_INI_PATH, checkMessage.get(DamengConstant.DM_INI_PATH));
        environment.setExtendInfo(extendInfo);
        environment.setEndpoint(agentEnv.getEndpoint());
        environment.setPath(agentEnv.getEndpoint());
        environment.setParentUuid(agentEnv.getUuid());
        environment.setLinkStatus(agentEnv.getLinkStatus());
    }

    private Map<String, String> getCheckResult(ProtectedEnvironment environment) {
        ResourceCheckContext checkContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment).checkConnection(environment);
        ActionResult actionResult = checkContext.getActionResults().get(IsmNumberConstant.ZERO);
        return JSONObject.fromObject(actionResult.getMessage()).toMap(String.class);
    }

    private List<NodeInfo> buildNodeInfo(ProtectedEnvironment environment, ProtectedEnvironment agentEnv) {
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setUuid(agentEnv.getUuid());
        nodeInfo.setName(agentEnv.getName());
        nodeInfo.setEndpoint(agentEnv.getEndpoint());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.INSTANCESTATUS, agentEnv.getLinkStatus());
        extendInfo.put(DatabaseConstants.PORT, environment.getExtendInfo().get(DatabaseConstants.PORT));
        extendInfo.put(DamengConstant.AUTH_TYPE, String.valueOf(environment.getAuth().getAuthType()));
        extendInfo.put(DamengConstant.ROLE, DamengConstant.PRIMARY);
        nodeInfo.setExtendInfo(extendInfo);
        return Collections.singletonList(nodeInfo);
    }

    private void checkNodeExist(ProtectedEnvironment environment) {
        ProtectedEnvironment agentEnv = damengService.queryAgentEnvironment(environment);
        Set<String> uuidAndPortSet = damengService.getExistingUuidAndPort(environment);
        String uuidAndPort = damengService.connectUuidAndPort(agentEnv.getUuid(),
            environment.getExtendInfo().get(DatabaseConstants.PORT));
        if (uuidAndPortSet.contains(uuidAndPort)) {
            log.error("The Dameng Single Node is already registered. uuid: {}", environment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                "The Dameng Single Node is already registered.");
        }
    }
}
