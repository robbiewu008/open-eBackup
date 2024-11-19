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
package openbackup.cnware.protection.access.provider;

import com.huawei.oceanprotect.system.base.cert.common.constants.CertErrorCode;
import com.huawei.oceanprotect.system.base.cert.util.CertFileUtil;
import com.huawei.oceanprotect.system.base.cert.util.CertUtil;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.dto.ResourceScanParam;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.cnware.protection.access.util.CnwareUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.DefaultRoleHelper;
import openbackup.system.base.util.OptionalUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.security.cert.CRL;
import java.security.cert.CRLException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.TreeSet;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * CNware资源注册扫描
 *
 */
@Component
@Slf4j
public class CnwareEnvironmentProvider implements EnvironmentProvider {
    private final CnwareCommonService cnwareCommonService;
    private final ResourceService resourceService;
    private final AgentUnifiedService agentUnifiedService;
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;
    private final AgentUnifiedService agentService;

    /**
     * 构造器注入
     *
     * @param cnwareCommonService cnwareCommonService
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param envRetrievalsService envRetrievalsService
     * @param agentService agentService
     */
    public CnwareEnvironmentProvider(CnwareCommonService cnwareCommonService, ResourceService resourceService,
        AgentUnifiedService agentUnifiedService, ProtectedEnvironmentRetrievalsService envRetrievalsService,
        AgentUnifiedService agentService) {
        this.cnwareCommonService = cnwareCommonService;
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.envRetrievalsService = envRetrievalsService;
        this.agentService = agentService;
    }

    /**
     * detect object applicable
     *
     * @param subType object
     * @return detect result
     */
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CNWARE.equalsSubType(subType);
    }

    /**
     * 对环境信息进行检查，该接口用于注册受保护环境，修改受保护环境时对环境信息进行验证
     * 比如检查受保护环境与ProtectManager之间的连通性，认证信息是否合法。环境参数是否合法等等
     * 检查不通过抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException
     *
     * @param environment 受保护环境
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        // 校验同一受保护环境是否被重复注册
        checkEnvironmentRepeat(environment);

        // 查询已注册的CNware资源数量是否已经达到上限
        checkCnwareCount(environment);

        // 校验参数格式，并填充需要从其他模块获取的参数
        checkAndPrepareParam(environment);

        // 获取agent环境信息列表
        List<ProtectedEnvironment> agentEnvList = getAgentEnvironment(environment);

        // 检查连通性
        checkConnectivity(environment, agentEnvList);

        // CNware特性需要从agent中查询版本信息并回填到environment中
        updateEnvironment(environment, agentEnvList);

        // 更新环境状态
        updateStatus(environment);
    }

    private void checkAndPrepareParam(ProtectedEnvironment environment) {
        log.info("Check and prepare param start, uuid: {}, name: {}", environment.getUuid(), environment.getName());

        // 检查环境信息
        checkEnvironment(environment);

        // 检查注册时用户所填endpoint和port
        checkEndpointAndPort(environment);

        // 填充资源扫描频率，默认1小时，范围1~72小时
        fillScanInterval(environment);

        // 检查证书是否可用
        checkCnwareCertIfEnable(environment);
        fillCertEnableStatusForDisplay(environment);

        // OP服务化适配
        TokenBo.UserBo user = TokenBo.get().getUser();
        if (!DefaultRoleHelper.isAdmin(user.getId())) {
            environment.setUserId(user.getId());
            environment.setAuthorizedUser(user.getName());
        }
    }

    private void checkEnvironment(ProtectedEnvironment environment) {
        cnwareCommonService.checkEnvName(environment.getName());
    }

    private void checkEndpointAndPort(ProtectedEnvironment environment) {
        log.info("Start to check ip and port in environment: {}.", environment.getName());
        CnwareUtil.checkEndpoint(environment.getEndpoint());
        CnwareUtil.checkPort(environment.getPort());
    }

    private void fillScanInterval(ProtectedEnvironment environment) {
        Map<String, String> extendInfo = environment.getExtendInfo();
        int interval = Integer.parseInt(extendInfo.get(CnwareConstant.RESCAN_INTERVAL_IN_SEC));
        if (interval < CnwareConstant.SCAN_INTERVAL_MIN || interval > CnwareConstant.SCAN_INTERVAL_MAX) {
            log.info("Resource scan interval is out of range, set to the default value 1 hour. env name:{}, uuid:{}",
                environment.getName(), environment.getUuid());
            environment.setScanInterval(CnwareConstant.SCAN_INTERVAL_DEFAULT);
            return;
        }
        environment.setScanInterval(interval);
    }

    private void checkCnwareCertIfEnable(ProtectedEnvironment env) {
        Authentication auth = env.getAuth();
        Map<String, String> authExtendInfo = Optional.ofNullable(auth.getExtendInfo()).orElse(new HashMap<>());
        if (!CnwareConstant.ENABLE.equals(authExtendInfo.get(CnwareConstant.ENABLE_CERT))) {
            log.info("CNware({}) not enable cert, enableCert: {}.", env.getName(), authExtendInfo.get("enableCert"));
            return;
        }
        X509Certificate certificate = parseCertificate(authExtendInfo);
        Optional<X509CRL> optCrl = parseCrl(authExtendInfo);
        optCrl.ifPresent(crl -> checkCertAndCrlMatch(certificate, crl));
    }

    private X509Certificate parseCertificate(Map<String, String> authExtendInfo) {
        String certification = authExtendInfo.get("certification");
        if (VerifyUtil.isEmpty(certification)) {
            log.error("Certification is empty.");
            throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "certification is empty");
        }
        if (certification.length() > CnwareConstant.CERT_MAX_BYTE_SIZE) {
            log.error("Upload file is too large.");
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED,
                new String[] {"1MB"}, "upload file is too large.");
        }
        Certificate certificate;
        try (ByteArrayInputStream certStream =
            new ByteArrayInputStream(certification.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            certificate = certificateFactory.generateCertificate(certStream);
        } catch (CertificateException | IOException e) {
            throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "Invalid certification");
        }
        if (certificate instanceof X509Certificate) {
            return (X509Certificate) certificate;
        }
        throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "Certification is not X509");
    }

    private Optional<X509CRL> parseCrl(Map<String, String> authExtendInfo) {
        String revocationList = authExtendInfo.get("revocationList");
        if (VerifyUtil.isEmpty(revocationList)) {
            log.error("RevocationList is empty.");
            return Optional.empty();
        }
        if (revocationList.length() > CnwareConstant.CRL_MAX_BYTE_SIZE) {
            log.error("Upload file is too large.");
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED,
                new String[] {"5KB"}, "upload file is too large.");
        }
        CRL crl;
        try (ByteArrayInputStream revocationStream =
            new ByteArrayInputStream(revocationList.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            crl = certificateFactory.generateCRL(revocationStream);
        } catch (CRLException | IOException e) {
            throw new LegoCheckedException(CertErrorCode.CRL_FORMAT_INVALID, "Invalid crl");
        }
        if (crl instanceof X509CRL) {
            CertFileUtil.checkCrlIsValid((X509CRL) crl);
            return Optional.of((X509CRL) crl);
        }
        throw new LegoCheckedException(CertErrorCode.CRL_FORMAT_INVALID, "Crl is not X509");
    }

    private void checkCertAndCrlMatch(X509Certificate certificate, X509CRL crl) {
        String certName = certificate.getIssuerX500Principal().getName();
        String crlName = crl.getIssuerX500Principal().getName();
        if (!Objects.equals(certName, crlName)) {
            throw new LegoCheckedException(CertErrorCode.CRL_ISSUER_INCONSISTENT_CERT_ISSUER, "cert no match with crl");
        }
    }

    private void fillCertEnableStatusForDisplay(ProtectedEnvironment env) {
        Map<String, String> extendInfo = Optional.ofNullable(env.getExtendInfo()).orElse(new HashMap<>());
        Map<String, String> authExtendInfo = Optional.ofNullable(env.getAuth().getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(CnwareConstant.ENABLE_CERT, authExtendInfo.get(CnwareConstant.ENABLE_CERT));
        env.setExtendInfo(extendInfo);
    }

    private List<ProtectedEnvironment> getAgentEnvironment(ProtectedEnvironment environment) {
        log.info("Start to get agent info of environment: {}", environment.getUuid());
        List<ProtectedResource> agents = environment.getDependencies().get(CnwareConstant.AGENTS);
        if (VerifyUtil.isEmpty(agents)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Agent is not exists.");
        }
        List<ProtectedEnvironment> agentsEnvList = new ArrayList<>();
        for (ProtectedResource agent : agents) {
            try {
                ProtectedEnvironment agentEnv = cnwareCommonService.getEnvironmentById(agent.getUuid());
                if (VerifyUtil.isEmpty(agentEnv)) {
                    throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Get agent environment failed.");
                }
                agentsEnvList.add(agentEnv);
            } catch (LegoCheckedException e) {
                log.error("Query agent environment error, agentId:{}.", agent.getUuid(), e);
            }
        }
        return agentsEnvList;
    }

    private void checkConnectivity(ProtectedEnvironment environment, List<ProtectedEnvironment> agentEnvList) {
        cnwareCommonService.checkConnectivity(environment, agentEnvList);
    }

    private void updateEnvironment(ProtectedEnvironment environment, List<ProtectedEnvironment> agentEnvList) {
        // 设置版本号
        setVersion(environment, agentEnvList);
    }

    private void setVersion(ProtectedEnvironment environment, List<ProtectedEnvironment> agentEnvList) {
        for (ProtectedEnvironment agentEnv : agentEnvList) {
            try {
                AppEnvResponse appEnvResponse = cnwareCommonService.queryClusterInfo(environment, agentEnv);
                if (Objects.isNull(appEnvResponse) || MapUtils.isEmpty(appEnvResponse.getExtendInfo())) {
                    throw new LegoCheckedException(CommonErrorCode.VERSION_ERROR, "get cluster version failed.");
                }
                if (!appEnvResponse.getExtendInfo().get(CnwareConstant.PRODUCT_VERSION).isEmpty()) {
                    environment.setVersion(appEnvResponse.getExtendInfo().get(CnwareConstant.PRODUCT_VERSION));
                    return;
                }
            } catch (LegoCheckedException e) {
                log.error("Query CNware resource error, envid:{}.", environment.getUuid(), e);
            }
        }
        throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT,
            "CNware connection check failed.");
    }

    private void checkEnvironmentRepeat(ProtectedEnvironment env) {
        if (StringUtils.isNotBlank(env.getUuid())) {
            log.info("Update env(uuid: {}, name: {}), no need check repeat.", env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> condition = new HashMap<>();
        condition.put(CnwareConstant.SUBTYPE, env.getSubType());
        String envIp = env.getEndpoint();
        condition.put(CnwareConstant.ENDPOINT, envIp);
        PageListResponse<ProtectedResource> registeredEnv = resourceService.query(0, 1, condition);
        if (registeredEnv.getTotalCount() > 0) {
            log.info("The env with IP({}) has been registered.", envIp);
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "Env has registered");
        }
    }

    private void checkCnwareCount(ProtectedEnvironment env) {
        if (StringUtils.isNotBlank(env.getUuid())) {
            log.info("Update env(uuid: {}, name: {}), no need check whether exceeds the maximum in the environment.",
                env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> requestCondition = new HashMap<>();
        requestCondition.put(CnwareConstant.SUBTYPE, env.getSubType());
        PageListResponse<ProtectedResource> registeredCnwareEnv = resourceService.query(0, 1, requestCondition);
        if (registeredCnwareEnv.getTotalCount() >= CnwareConstant.CNWARE_MAX_COUNT) {
            log.error("Cnware check, count Exceeded the maximum count: {}, env(uuid: {}, name: {})",
                registeredCnwareEnv.getTotalCount(), env.getUuid(), env.getName());
            throw new LegoCheckedException(CommonErrorCode.ENV_COUNT_OVER_LIMIT,
                new String[] {String.valueOf(CnwareConstant.CNWARE_MAX_COUNT)},
                "The number of Cnware exceeds the maximum in the environment.");
        }
    }

    private void updateStatus(ProtectedEnvironment env) {
        env.setPath(Optional.ofNullable(env.getPath()).orElse(env.getEndpoint()));
        env.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 受保护环境健康状态检查，
     * 状态异常抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException，
     * 并返回具体的错误码
     *
     * @param environment 受保护环境
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        List<ProtectedEnvironment> agentEnvList = getAgentEnvironment(environment);
        checkConnectivity(environment, agentEnvList);
    }

    // 填充Resource的path
    private List<ProtectedResource> fillResourcePath(
        List<ProtectedResource> resourceList, ProtectedEnvironment environment) {
        log.info("CNware resource fillResourcePath start, environment id: {}", environment.getUuid());
        Map<String, ProtectedResource> resourceMap = new HashMap<>(resourceList.size() + CnwareConstant.ONE);
        resourceMap.put(environment.getUuid(), environment);
        for (ProtectedResource resource : resourceList) {
            resourceMap.put(resource.getUuid(), resource);
        }
        List<ProtectedResource> validResources = new ArrayList<>(resourceList.size());
        for (ProtectedResource resource : resourceList) {
            if (!resourceMap.containsKey(resource.getParentUuid())) {
                log.error("Resource parent does not exist, resource: {}.", buildPrintInfo(resource));
                resourceMap.remove(resource.getUuid());
                continue;
            }
            ProtectedResource parent = resourceMap.get(resource.getParentUuid());
            String parentPath = Optional.ofNullable(parent.getPath()).orElse(parent.getName());
            resource.setPath(parentPath + File.separator + resource.getName());
            validResources.add(resource);
        }
        return validResources;
    }

    private String buildPrintInfo(ProtectedResource resource) {
        return "uuid: " + resource.getUuid() + ", " + "name: " + resource.getName() + ", " + "type: "
            + resource.getType() + ", " + "subType: " + resource.getSubType() + ", " + "parentUuid: "
            + resource.getParentUuid() + ", " + "parentName: " + resource.getParentName();
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("Scan environment start, environment id: {}", environment.getUuid());
        ResourceScanParam resourceScanParam = new ResourceScanParam();
        resourceScanParam.setEnvironment(environment);
        return scanByAgent(this::scanResourceByAgent, resourceScanParam);
    }

    List<ProtectedResource> scanByAgent(Function<ResourceScanParam, List<ProtectedResource>> scanMethod,
        ResourceScanParam resourceScanParam) {
        List<ProtectedEnvironment> agents = getAllAgents(resourceScanParam.getEnvironment());
        LegoCheckedException legoCheckedException = new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR,
            "Resource scan failed.");
        for (ProtectedResource agent : agents) {
            try {
                resourceScanParam.setEndpoint(getAgentEndpoint(agent.getUuid()));
                return scanMethod.apply(resourceScanParam);
            } catch (LegoCheckedException exception) {
                log.error("Scan resources by agent:{} failed.",
                    agent.getUuid(), ExceptionUtil.getErrorMessage(exception));
                legoCheckedException = exception;
            }
        }
        throw legoCheckedException;
    }

    private List<ProtectedEnvironment> getAllAgents(ProtectedEnvironment env) {
        Map<ProtectedResource, List<ProtectedEnvironment>> map =
            envRetrievalsService.collectConnectableResources(env.getUuid());
        return map.values().stream().flatMap(List::stream).collect(Collectors.toList());
    }

    private Endpoint getAgentEndpoint(String agentId) {
        Optional<ProtectedResource> optResource = resourceService.getResourceById(agentId);
        return optResource.flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Agent is empty"));
    }

    private List<ProtectedResource> scanResourceByAgent(ResourceScanParam resourceScanParam) {
        Endpoint endpoint = resourceScanParam.getEndpoint();
        ProtectedEnvironment environment = resourceScanParam.getEnvironment();
        log.info("Start to scan CNware environment, agentId:{}, envId:{}", endpoint.getId(), environment.getUuid());

        // 扫描CNwareHostPool资源
        List<ProtectedResource> cnwareHostPoolResources = doScanResources(endpoint, Lists.newArrayList(),
            CnwareConstant.RES_TYPE_CNWARE_HOST_POOL, environment);

        // 扫描CNwareCluster资源
        List<ProtectedResource> cnwareClusterResources = doScanResources(endpoint, Lists.newArrayList(),
            CnwareConstant.RES_TYPE_CNWARE_CLUSTER, environment);

        // 扫描CNwareHost资源
        List<ProtectedResource> cnwareHostResources = doScanResources(endpoint, Lists.newArrayList(),
            CnwareConstant.RES_TYPE_CNWARE_HOST, environment);

        // 扫描CNwareVm资源
        List<ProtectedResource> cnwareVmResources = doScanResources(endpoint, Lists.newArrayList(),
            CnwareConstant.RES_TYPE_CNWARE_VM, environment);

        List<ProtectedResource> resources = Lists.newArrayList();
        resources.addAll(cnwareHostPoolResources);
        resources.addAll(cnwareClusterResources);
        resources.addAll(cnwareHostResources);
        resources.addAll(cnwareVmResources);
        resources = fillResourcePath(Lists.newArrayList(resources), environment);
        resources = updateVmCountInClusterAndHost(resources);
        log.info("finish scan CNware environment, agentId:{}, envId:{}", endpoint.getId(), environment.getUuid());
        return resources;
    }

    private List<ProtectedResource> doScanResources(Endpoint endpoint, List<Application> applications,
        String condition, ProtectedEnvironment env) {
        log.info("Cnware doScanResources start, environment id: {}", env.getUuid());
        int page = CnwareConstant.ZERO;
        int size = CnwareConstant.PAGE_SIZE;
        ListResourceV2Req request = new ListResourceV2Req();
        request.setPageSize(size);
        request.setPageNo(page);
        request.setAppEnv(BeanTools.copy(env, AppEnv::new));
        request.setApplications(applications);
        Map<String, String> conditions = new HashMap<>();
        conditions.put(CnwareConstant.RESOURCE_TYPE_KEY, condition);
        conditions.put(CnwareConstant.IS_TREE, String.valueOf(CnwareConstant.ZERO));
        request.setConditions(JSON.toJSONString(conditions));
        PageListResponse<ProtectedResource> response;
        List<ProtectedResource> scanResources = Lists.newArrayList();
        do {
            request.setPageNo(page);
            response = agentService.getDetailPageList(env.getSubType(), endpoint.getIp(), endpoint.getPort(), request);
            List<ProtectedResource> protectedResources = response.getRecords();
            if (CollectionUtils.isEmpty(protectedResources)) {
                break;
            }
            page++;
            scanResources.addAll(protectedResources);

            // 将最后一条记录id作为下次查询的标记
            if (CnwareConstant.RES_TYPE_CNWARE_VM.equals(condition)) {
                conditions.put(CnwareConstant.SCAN_MARKER_KEY,
                    protectedResources.get(protectedResources.size() - 1).getUuid());
                request.setConditions(JSON.toJSONString(conditions));
            }
        } while (response.getRecords().size() == size);

        // 根据uuid去重
        scanResources = scanResources.stream()
            .peek(resource -> resource.setRootUuid(env.getUuid()))
            .collect(Collectors.collectingAndThen(Collectors.toCollection(
                () -> new TreeSet<>(Comparator.comparing(ResourceBase::getUuid))), Lists::newArrayList));
        log.info("scan resources success, condition:{}, total:{}", condition, scanResources.size());
        return scanResources;
    }

    private List<ProtectedResource> updateVmCountInClusterAndHost(List<ProtectedResource> resources) {
        List<ProtectedResource> cnwareClusterResources = resources.stream().filter(
            res -> CnwareConstant.RES_TYPE_CNWARE_CLUSTER.equals(res.getSubType())).collect(Collectors.toList());
        List<ProtectedResource> cnwareVmResources = resources.stream().filter(
            res -> CnwareConstant.RES_TYPE_CNWARE_VM.equals(res.getSubType())).collect(Collectors.toList());
        for (ProtectedResource cnwareClusterResource : cnwareClusterResources) {
            int vmNumnberInCluster = 0;
            String path = cnwareClusterResource.getPath();
            for (ProtectedResource cnwareVmResource : cnwareVmResources) {
                if (cnwareVmResource.getPath().contains(path)) {
                    vmNumnberInCluster++;
                }
            }
            cnwareClusterResource.getExtendInfo().put(CnwareConstant.VM_NUMBER, String.valueOf(vmNumnberInCluster));
        }
        List<ProtectedResource> cnwareHostResources = resources.stream().filter(
            res -> CnwareConstant.RES_TYPE_CNWARE_HOST.equals(res.getSubType())).collect(Collectors.toList());
        for (ProtectedResource cnwareHostResource : cnwareHostResources) {
            int vmNumnberInHost = 0;
            String path = cnwareHostResource.getPath();
            for (ProtectedResource cnwareVmResource : cnwareVmResources) {
                if (cnwareVmResource.getPath().contains(path)) {
                    vmNumnberInHost++;
                }
            }
            cnwareHostResource.getExtendInfo().put(CnwareConstant.VM_NUMBER, String.valueOf(vmNumnberInHost));
        }
        List<ProtectedResource> cnwareHostPoolResources = resources.stream().filter(
            res -> CnwareConstant.RES_TYPE_CNWARE_HOST_POOL.equals(res.getSubType())).collect(Collectors.toList());
        List<ProtectedResource> resourceList = new ArrayList<>();
        resourceList.addAll(cnwareHostPoolResources);
        resourceList.addAll(cnwareClusterResources);
        resourceList.addAll(cnwareHostResources);
        resourceList.addAll(cnwareVmResources);
        return resourceList;
    }
}
