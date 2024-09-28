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
package openbackup.data.access.framework.protection.service;

import com.huawei.oceanprotect.client.resource.manager.constant.DataProtocolEnum;
import com.huawei.oceanprotect.client.resource.manager.entity.AgentLanFree;
import com.huawei.oceanprotect.client.resource.manager.entity.SanClientConfig;
import com.huawei.oceanprotect.client.resource.manager.service.AgentLanFreeSanClientService;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.SanClientInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.enums.ClientProtocolTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.CommUtils;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述 sanClient服务类
 *
 */
@Component
public class SanClientService {
    /**
     * 是否下发san任务的标志
     */
    public static final String IS_SANCLIENT = "sanclientInvolved";

    /**
     * 多节点执行
     */
    public static final String ADVANCE_PARAMS_KEY_MULTI_POST_JOB = "multiPostJob";

    private static final String WWPN = "wwpn";

    private final AgentLanFreeSanClientService sanClientService;
    private final ResourceService resourceService;
    private final CopyManagerService copyManagerService;
    private EncryptorUtil encryptorUtil;
    private AgentLanFreeSanClientService agentLanFreeSanClientService;

    public SanClientService(AgentLanFreeSanClientService sanClientService, ResourceService resourceService,
        CopyManagerService copyManagerService) {
        this.sanClientService = sanClientService;
        this.resourceService = resourceService;
        this.copyManagerService = copyManagerService;
    }

    @Autowired
    public void setEncryptorUtil(EncryptorUtil encryptorUtil) {
        this.encryptorUtil = encryptorUtil;
    }

    /**
     * 获取未配置san的代理主机uuid列表
     *
     * @param agents 通用代理主机
     * @return 绑定san的代理主机uuids
     */
    public String[] getAgentsNotConfiguredSanclient(List<Endpoint> agents) {
        return Optional.ofNullable(agents)
                .map(items -> items.stream()
                        .filter(item -> !sanClientService.isBindedSanClient(item.getId()))
                        .map(Endpoint::getIp)
                        .toArray(String[]::new))
                .orElse(new String[] {});
    }

    /**
     * 填充san任务需要的代理参数
     *
     * @param agent 填充san信息的代理
     */
    public void fillAgentParams(Endpoint agent) {
        SanClientConfig sanClientConfig = getSanClientConfig(agent.getId());
        validateSanClientConfig(sanClientConfig, agent.getIp());
        agent.setWwpns(CommUtils.parseJsonStr2Array(sanClientConfig.getClientWwpns(), WWPN));
        setSanClients(agent, sanClientConfig);
    }

    private void fillAgentParamsNotCheckDataProtocol(Endpoint agent) {
        SanClientConfig sanClientConfig = sanClientService.getAgentLanFreeSanClientByResourceId(agent.getId());
        if (VerifyUtil.isEmpty(sanClientConfig)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "SanClient config not exist, Illegal");
        }
        boolean isFC = DataProtocolEnum.FC.value().equals(sanClientConfig.getDataProtocol());
        if (isFC) {
            String clientWwpns = sanClientConfig.getClientWwpns();
            agent.setWwpns(CommUtils.parseJsonStr2Array(clientWwpns, WWPN));
        } else {
            List<String> iqns = JSONArray.fromObject(sanClientConfig.getClientIqns()).toBean(String.class);
            List<String> decryptIqns = iqns.stream().map(iqn -> encryptorUtil.getDecryptPwd(iqn) == null
                                ? iqn : encryptorUtil.getDecryptPwd(iqn)).collect(Collectors.toList());
            agent.setIqns(decryptIqns);
        }
        setSanClients(agent, sanClientConfig);
    }

    private SanClientConfig getSanClientConfig(String agentId) {
        SanClientConfig sanClientConfig = sanClientService.getAgentLanFreeSanClientByResourceId(agentId);
        if (VerifyUtil.isEmpty(sanClientConfig)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "SanClient config not exist, Illegal");
        }
        return sanClientConfig;
    }

    private void validateSanClientConfig(SanClientConfig sanClientConfig, String ip) {
        if (!DataProtocolEnum.FC.value().equals(sanClientConfig.getDataProtocol())) {
            throw new LegoCheckedException(CommonErrorCode.SANCLIENT_CONFIG_FC_ERROR,
                    new String[] {ip}, "Data protocol of Aix agent {0} in Backup task is not FC");
        }
    }

    private void setSanClients(Endpoint agent, SanClientConfig sanClientConfig) {
        List<String> sanResourceIds = JSONArray.fromObject(sanClientConfig.getSanclientResourceIds())
                .toBean(String.class);
        List<SanClientInfo> sanClientInfos = sanResourceIds.stream()
                .filter(id -> !VerifyUtil.isEmpty(findAgentByUuid(id)))
                .map(id -> {
                    AgentLanFree sanClient = sanClientService.getAgentLanFreeByResourceId(id);
                    if (VerifyUtil.isEmpty(sanClient)) {
                        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                                "SanClient not exist, Illegal");
                    }
                    Endpoint endpoint = findAgentByUuid(id);

                    SanClientInfo sanClientInfo = new SanClientInfo();
                    sanClientInfo.setId(id);
                    sanClientInfo.setIp(endpoint.getIp());
                    sanClientInfo.setPort(endpoint.getPort());
                    if (DataProtocolEnum.FC.value().equals(sanClientConfig.getDataProtocol())) {
                        sanClientInfo
                                .setSanClientWwpns(CommUtils.parseJsonStr2Array(sanClient.getSanClientWwpns(), WWPN));
                    } else {
                        List<String> iqns = JSONArray.fromObject(sanClient.getSanClientIqns()).toBean(String.class);
                        List<String> decryptIqns =
                                iqns.stream().map(iqn -> encryptorUtil.getDecryptPwd(iqn)).collect(Collectors.toList());
                        sanClientInfo.setIqns(decryptIqns);
                    }

                    String fcWwpns = sanClient.getWwpns();
                    if (!VerifyUtil.isEmpty(JSONArray.fromObject(fcWwpns))) {
                        sanClientInfo.setWwpns(CommUtils.parseJsonStr2Array(fcWwpns, WWPN));
                        sanClientInfo.setFcPorts(CommUtils.parseJsonStr2Array(sanClient.getFcPorts(), WWPN));
                        sanClientInfo.setOpenLanFreeSwitch(Boolean.TRUE);
                    }
                    return sanClientInfo;
                }).collect(Collectors.toList());
        if (VerifyUtil.isEmpty(sanClientInfos)) {
            throw new LegoCheckedException(CommonErrorCode.ALL_SANCLIENT_IS_OFFLINE,
                    "All sanClients is offline, Illegal");
        }
        agent.setSanClients(sanClientInfos);
    }

    private Endpoint findAgentByUuid(String resUuid) {
        return resourceService.getResourceById(resUuid)
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .filter(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus()))
                .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()))
                .orElse(null);
    }

    /**
     * 恢复、校验、删除任务公用配置san信息
     *
     * @param copyId 副本id
     * @param agents 填充san信息的代理
     * @param params 任务高级参数
     * @param dataLayout 数据布局
     */

    public void configSanClient(String copyId, List<Endpoint> agents, Map<String, String> params,
        BaseDataLayout dataLayout) {
        boolean isSanCopy = copyManagerService.checkSanCopy(Optional.ofNullable(copyId).orElse(""));
        if (!isSanCopy) {
            params.put(IS_SANCLIENT, Boolean.FALSE.toString());
            return;
        }
        params.put(IS_SANCLIENT, Boolean.TRUE.toString());
        params.put(ADVANCE_PARAMS_KEY_MULTI_POST_JOB, Boolean.TRUE.toString());
        dataLayout.setClientProtocolType(ClientProtocolTypeEnum.DATA_TURBO.getClientProtocolType());
        if (VerifyUtil.isEmpty(agents)) {
            return;
        }
        String[] notConfiguredSanClientAgents = getAgentsNotConfiguredSanclient(agents);
        if (!VerifyUtil.isEmpty(notConfiguredSanClientAgents)) {
            throw new LegoCheckedException(CommonErrorCode.AIX_PARTIAL_ASSOCIATED_SANCLIENT_ERROR,
                    notConfiguredSanClientAgents, "Aix agents {0} not config sanClient");
        }
        agents.forEach(this::fillAgentParamsNotCheckDataProtocol);
    }

    /**
     * 副本删除任务，删除副本在ubc执行，需要发送空的agent列表。
     *
     * @param agents 任务的agents
     * @param copyId 副本id
     */

    public void configCopyDeleteAgent(String copyId, List<Endpoint> agents) {
        if (VerifyUtil.isEmpty(agents)) {
            return;
        }
        boolean isSanCopy = copyManagerService.checkSanCopy(Optional.ofNullable(copyId).orElse(""));
        if (!isSanCopy) {
            return;
        }
        agents.clear();
    }

    /**
     * 清理agent和agent中SanClient的iqns
     *
     * @param agents 任务的agents
     * @param copyId 副本id
     */

    public void cleanAgentIqns(List<Endpoint> agents, String copyId) {
        boolean isSanCopy = copyManagerService.checkSanCopy(Optional.ofNullable(copyId).orElse(""));
        if (!isSanCopy || VerifyUtil.isEmpty(agents)) {
            return;
        }
        // 清理agent里的iqns
        agents.stream()
                .map(Endpoint::getIqns)
                .forEach(this::cleanIqns);
        // 清理SanClient里的iqns
        agents.stream()
                .map(Endpoint::getSanClients)
                .forEach(this::cleanSanClientIqns);
    }

    private void cleanIqns(List<String> iqns) {
        if (VerifyUtil.isEmpty(iqns)) {
            return;
        }
        iqns.forEach(StringUtil::clean);
    }

    private void cleanSanClientIqns(List<SanClientInfo> sanClientInfos) {
        sanClientInfos.stream()
                .map(SanClientInfo::getIqns)
                .forEach(this::cleanIqns);
    }

    /**
     * 判断该任务是否副本类型不一致
     *
     * @param task 任务
     * @return 该副本是否可以执行校验或恢复，仅当两个都存在，或者两个都不存在，可以进行恢复
     */
    public boolean checkSanCopyAndLanFree(BaseTask task) {
        String[] notConfiguredSanClientAgents = getAgentsNotConfiguredSanclient(task.getAgents());
        boolean isSanCopy = copyManagerService.checkSanCopy(Optional.ofNullable(task.getCopyId()).orElse(""));
        return (isSanCopy && VerifyUtil.isEmpty(notConfiguredSanClientAgents))
                || (!isSanCopy && notConfiguredSanClientAgents.length == task.getAgents().size());
    }
}
