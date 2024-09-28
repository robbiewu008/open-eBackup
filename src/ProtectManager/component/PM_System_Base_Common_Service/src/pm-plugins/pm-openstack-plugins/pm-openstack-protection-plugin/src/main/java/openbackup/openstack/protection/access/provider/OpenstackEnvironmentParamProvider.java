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
package openbackup.openstack.protection.access.provider;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentParamProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackAgentService;
import openbackup.openstack.protection.access.common.OpenstackCommonService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import openbackup.openstack.protection.access.constant.KeyStoneConstant;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.OpenstackEnvExtendInfo;
import openbackup.openstack.protection.access.keystone.KeyStoneService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import com.huawei.oceanprotect.system.base.user.entity.UserInfoEntity;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import openbackup.system.base.util.DefaultRoleHelper;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * OpenstackEnvironmentParamProvider
 *
 */
@Slf4j
@Component
public class OpenstackEnvironmentParamProvider extends UnifiedEnvironmentParamProvider {
    private final UserService userService;

    private final KeyStoneService keystoneService;

    private final OpenstackAgentService openstackAgentService;

    private final OpenstackQuotaService openstackQuotaService;

    private OpenstackCommonService openstackCommonService;

    /**
     * 构造器注入
     *
     * @param keystoneService       keystoneService
     * @param resourceService       resourceService
     * @param userService           userService
     * @param openstackAgentService openstackAgentService
     * @param openstackQuotaService openstackQuotaService
     */
    public OpenstackEnvironmentParamProvider(KeyStoneService keystoneService, ResourceService resourceService,
        UserService userService, OpenstackAgentService openstackAgentService,
        OpenstackQuotaService openstackQuotaService) {
        super(resourceService);
        this.keystoneService = keystoneService;
        this.userService = userService;
        this.openstackAgentService = openstackAgentService;
        this.openstackQuotaService = openstackQuotaService;
    }

    /**
     * 依赖注入
     *
     * @param openstackCommonService openstackCommonService
     */
    @Autowired
    public void setOpenstackCommonService(OpenstackCommonService openstackCommonService) {
        this.openstackCommonService = openstackCommonService;
    }

    @Override
    public void checkEnvironmentRepeat(ProtectedEnvironment env) {
        log.info("check environment repeat start, envName: {}", env.getName());
        checkEnvParams(env);
        // 填充证书内容
        fillEnvCertInfo(env);
        // 先检查连通性，否则获取环境扩展信息时无法获取错误原因
        openstackAgentService.checkConnectivity(env);
        OpenstackEnvExtendInfo extraEnvExtendInfo = queryExtraEnvExtendInfo(env);
        String serviceId = extraEnvExtendInfo.getServiceId();
        PageListResponse<ProtectedResource> registeredEnv = queryRegisteredEnv(env, serviceId);
        // 创建不允许重复
        if (registeredEnv.getTotalCount() > 0 && StringUtils.isBlank(env.getUuid())) {
            log.error("the Openstack env with endpoint(keystoneUrl:{}, serviceId:{}) has been registered.",
                env.getEndpoint(), serviceId);
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "Env has registered");
        }
        // 更新不允许修改为不同id的keyStone(名称可修改)
        if (registeredEnv.getTotalCount() == 0 && StringUtils.isNotBlank(env.getUuid())) {
            log.error("can not update env with different keystoneUrl:{}.", env.getEndpoint());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "can not change keystone.");
        }
        Map<String, String> envExtendInfo = env.getExtendInfo();
        envExtendInfo.put(OpenstackConstant.SERVICE_ID_KEY, serviceId);
        envExtendInfo.put(OpenstackConstant.REGISTER_SERVICE, extraEnvExtendInfo.getRegisterService());
        envExtendInfo.put(KeyStoneConstant.CPS_IP, extraEnvExtendInfo.getCpsIp());
        log.info("check environment repeat finished, envId: {}", env.getUuid());
    }

    private void checkEnvParams(ProtectedEnvironment env) {
        if (StringUtils.isBlank(env.getName()) || StringUtils.isBlank(env.getEndpoint())) {
            log.error("env name:[{}] or endpoint:[{}] is empty.", env.getName(), env.getEndpoint());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "env param is empty.");
        }
    }

    @Override
    public void checkAndPrepareParam(ProtectedEnvironment environment) {
        log.info("check and prepare OpenStack param start, env name: {}", environment.getName());
        // 检查平台数量
        checkEnvCount(environment);
        // 检查domain和openstack数量
        openstackCommonService.checkOpenStackAndDomainMaxNum();
        // 填充环境参数
        fillEnvProperties(environment);
        // 注册服务到openstack(云核场景)
        if (openstackQuotaService.isRegisterOpenstack(environment)) {
            log.info("ready to register openstack.");
            keystoneService.registerOpenstack(environment);
            authorizeQuotaUser(environment);
        }
        log.info("check and prepare OpenStack param finished, env_id: {}", environment.getUuid());
    }

    private void checkEnvCount(ProtectedEnvironment environment) {
        if (StringUtils.isNotBlank(environment.getUuid())) {
            return;
        }
        Map<String, String> envExtendInfo = environment.getExtendInfo();
        String serviceId = envExtendInfo.get(OpenstackConstant.SERVICE_ID_KEY);
        PageListResponse<ProtectedResource> response = queryRegisteredEnv(environment);
        Set<String> existKeystoneServices = response.getRecords().stream()
            .collect(Collectors.groupingBy(res -> res.getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY)))
            .keySet();
        log.info("openstack different environment count is:{}", existKeystoneServices.size());
        if (existKeystoneServices.size() >= OpenstackConstant.MAX_OPENSTACK_COUNT
            && !existKeystoneServices.contains(serviceId)) {
            throw new LegoCheckedException(CommonErrorCode.ENV_COUNT_OVER_LIMIT, new String[]{
                String.valueOf(OpenstackConstant.MAX_OPENSTACK_COUNT)}, "Environment count over limit.");
        }
    }

    @Override
    public boolean applicable(ProtectedEnvironment environment) {
        return ResourceSubTypeEnum.OPENSTACK_CONTAINER.equalsSubType(environment.getSubType());
    }

    private PageListResponse<ProtectedResource> queryRegisteredEnv(ProtectedEnvironment env, String serviceId) {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("type", env.getType());
        filterMap.put("subType", env.getSubType());
        filterMap.put(OpenstackConstant.SERVICE_ID_KEY, serviceId);
        return resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE, filterMap);
    }

    private PageListResponse<ProtectedResource> queryRegisteredEnv(ProtectedEnvironment env) {
        Map<String, Object> filterMap = new HashMap<>();
        filterMap.put("type", env.getType());
        filterMap.put("subType", env.getSubType());
        return resourceService.query(LegoNumberConstant.ZERO, OpenstackConstant.MAX_DOMAIN_COUNT, filterMap);
    }

    private void authorizeQuotaUser(ProtectedEnvironment environment) {
        UserInfoEntity quotaUser = userService.getUserInfoByName(OpenstackConstant.QUOTA_USER_NAME);
        if (Objects.isNull(quotaUser)) {
            throw new LegoCheckedException(CommonErrorCode.UNKNOWN_USER, "quota user not exists.");
        }
        boolean isDpAdmin = userService.getUserInfoByUserId(quotaUser.getUuid()).getRolesSet()
            .stream()
            .anyMatch(roleInfo -> Constants.Builtin.ROLE_DP_ADMIN.equals(roleInfo.getRoleName()));
        if (!isDpAdmin) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "quota user is not DataProtectAdmin");
        }
        checkAgentUser(environment, quotaUser.getUuid());
        environment.getExtendInfo().put(OpenstackConstant.QUOTA_USER_ID, quotaUser.getUuid());
        environment.setUserId(quotaUser.getUuid());
        environment.setAuthorizedUser(quotaUser.getUserName());
    }

    private void checkAgentUser(ProtectedEnvironment environment, String userId) {
        List<ProtectedResource> agents = environment.getDependencies().get(OpenstackConstant.AGENTS);
        agents.forEach(agent -> {
            String originalUserId = resourceService.getResourceById(false, agent.getUuid())
                .map(ProtectedResource::getUserId)
                .orElse(StringUtils.EMPTY);
            if (StringUtils.isBlank(originalUserId) && !StringUtils.equals(originalUserId, userId)) {
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "agent has authorized.");
            }
        });
    }

    private void fillEnvProperties(ProtectedEnvironment environment) {
        Map<String, String> envExtendInfo = environment.getExtendInfo();
        if (StringUtils.isBlank(environment.getUuid())) {
            String envUuid = UUID.nameUUIDFromBytes((envExtendInfo.get(OpenstackConstant.SERVICE_ID_KEY))
                .getBytes(StandardCharsets.UTF_8)).toString();
            environment.setUuid(envUuid);
        }
        environment.setPath(environment.getName());
        TokenBo.UserBo user = TokenBo.get().getUser();
        if (Objects.nonNull(user) && !DefaultRoleHelper.isAdmin(user.getId())) {
            environment.setUserId(user.getId());
            environment.setAuthorizedUser(user.getName());
        }
    }

    private void fillEnvCertInfo(ProtectedEnvironment env) {
        boolean shouldDecrypt = true;
        Optional<ProtectedResource> dbEnvironment = resourceService.getResourceById(shouldDecrypt, env.getUuid());
        if (!dbEnvironment.isPresent()) {
            log.info("create Openstack environment(name{}, id{}), no need to update certification.", env.getName(),
                env.getUuid());
            return;
        }
        Map<String, String> extendInfo = env.getExtendInfo();
        if (!OpenstackConstant.CERT_ENABLE.equals(extendInfo.get(OpenstackConstant.ENABLE_CERT))) {
            clearCertInfo(env);
            return;
        }
        Map<String, String> dbExtendInfo = dbEnvironment.get().getExtendInfo();
        Map<String, String> dbAuthExtendInfo = dbEnvironment.get().getAuth().getExtendInfo();
        Map<String, String> authExtendInfo = env.getAuth().getExtendInfo();
        if (isSameCert(extendInfo, dbExtendInfo)) {
            authExtendInfo.put(OpenstackConstant.CERTIFICATION, dbAuthExtendInfo.get(OpenstackConstant.CERTIFICATION));
            log.info("environment(id: {}) fill certification success.", env.getUuid());
        }
        if (isSameCrl(extendInfo, dbExtendInfo)) {
            authExtendInfo.put(OpenstackConstant.REVOCATION_LIST, dbAuthExtendInfo
                .get(OpenstackConstant.REVOCATION_LIST));
            log.info("environment(id: {}) fill revocation list success.", env.getUuid());
        }
    }

    private boolean isSameCrl(Map<String, String> extendInfo, Map<String, String> dbExtendInfo) {
        return Objects.equals(extendInfo.get(OpenstackConstant.CRL_NAME),
            dbExtendInfo.get(OpenstackConstant.CRL_NAME))
            && Objects.equals(extendInfo.get(OpenstackConstant.CRL_SIZE),
            dbExtendInfo.get(OpenstackConstant.CRL_SIZE));
    }

    private boolean isSameCert(Map<String, String> extendInfo, Map<String, String> dbExtendInfo) {
        return Objects.equals(extendInfo.get(OpenstackConstant.CERT_NAME),
            dbExtendInfo.get(OpenstackConstant.CERT_NAME))
            && Objects.equals(extendInfo.get(OpenstackConstant.CERT_SIZE),
            dbExtendInfo.get(OpenstackConstant.CERT_SIZE));
    }

    private void clearCertInfo(ProtectedEnvironment env) {
        Map<String, String> extendInfo = env.getExtendInfo();
        Map<String, String> authExtendInfo = env.getAuth().getExtendInfo();
        authExtendInfo.put(OpenstackConstant.CERTIFICATION, OpenstackConstant.EMPTY);
        authExtendInfo.put(OpenstackConstant.REVOCATION_LIST, OpenstackConstant.EMPTY);
        extendInfo.put(OpenstackConstant.CERT_NAME, OpenstackConstant.EMPTY);
        extendInfo.put(OpenstackConstant.CERT_SIZE, OpenstackConstant.EMPTY);
        extendInfo.put(OpenstackConstant.CRL_NAME, OpenstackConstant.EMPTY);
        extendInfo.put(OpenstackConstant.CRL_SIZE, OpenstackConstant.EMPTY);
    }

    private OpenstackEnvExtendInfo queryExtraEnvExtendInfo(ProtectedEnvironment environment) {
        AppEnvResponse envResponse = openstackAgentService.queryClusterInfo(environment);
        Map<String, String> extendInfo = Optional.ofNullable(envResponse)
            .map(AppEnvResponse::getExtendInfo)
            .orElseThrow(() ->
                new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "query service id error."));
        String serviceId = extendInfo.get(OpenstackConstant.SERVICE_ID_KEY);
        String isRegister = extendInfo.get(OpenstackConstant.REGISTER_SERVICE);
        String proxyIp = extendInfo.get(KeyStoneConstant.CPS_IP);
        log.info("openstack env extend info:  serviceId:{}, isRegister:{}, proxyIp:{}", serviceId,
            isRegister, proxyIp);
        if (StringUtils.isBlank(serviceId)) {
            log.error("service id is blank. serviceId:{}", serviceId);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "service id is blank.");
        }
        OpenstackEnvExtendInfo envExtendInfo = new OpenstackEnvExtendInfo();
        envExtendInfo.setServiceId(serviceId);
        envExtendInfo.setRegisterService(isRegister);
        envExtendInfo.setCpsIp(proxyIp);
        log.info("query openstack extra env extendInfo finished. serviceId:{}", serviceId);
        return envExtendInfo;
    }
}
