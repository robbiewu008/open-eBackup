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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import com.huawei.oceanprotect.k8s.protection.access.common.K8sQueryParam;
import com.huawei.oceanprotect.k8s.protection.access.common.util.YamlUtil;
import com.huawei.oceanprotect.k8s.protection.access.constant.K8sConstant;
import com.huawei.oceanprotect.k8s.protection.access.constant.K8sExtendInfoKey;
import com.huawei.oceanprotect.k8s.protection.access.constant.K8sKind;
import com.huawei.oceanprotect.k8s.protection.access.dto.Cluster;
import com.huawei.oceanprotect.k8s.protection.access.dto.Clusters;
import com.huawei.oceanprotect.k8s.protection.access.dto.Contexts;
import com.huawei.oceanprotect.k8s.protection.access.dto.KubeConfig;
import com.huawei.oceanprotect.k8s.protection.access.dto.TaskTimeOut;
import com.huawei.oceanprotect.k8s.protection.access.service.K8sCommonService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentListener;
import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.Base64Util;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.curator.shaded.com.google.common.collect.ImmutableList;
import org.apache.curator.shaded.com.google.common.collect.ImmutableSet;
import org.springframework.stereotype.Component;

import java.io.File;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * 功能描述: K8sEnvironmentProvider
 *
 */
@Slf4j
@AllArgsConstructor
@Component
public class K8sEnvironmentProvider implements EnvironmentProvider {
    private static final int K8S_CLUSTER_MAX_COUNT = 8;

    private static final int MAX_PAGE_SIZE = 500;
    private static final int MIN_JOB_NUMBER = 1;
    private static final int MAX_JOB_NUMBER = 8;
    private static final String SSL_DISABLED = "0";
    private static final Set<Character> K8S_SERVER_IP = ImmutableSet.of('0', '1', '2', '3', '4', '5', '6', '7', '8',
            '9', ':');
    private static final List<String> K8S_SYSTEM_NS = ImmutableList.of("kube-system",
            "kube-public", "kube-node-lease");
    private static final List<String> SSL_STATUS = ImmutableList.of("0", "1");
    private static final String ONLY_MATCH_NUMBER = "^[0-9][0-9]*$";
    private static final Pattern PATTERN = Pattern.compile(ONLY_MATCH_NUMBER);

    private final ResourceService resourceService;
    private final K8sCommonService commonService;
    private final MessageTemplate messageTemplate;
    private final ResourceExtendInfoService resourceExtendInfoService;
    private final UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;
    private final TokenVerificationService tokenVerificationService;

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.equalsSubType(subType);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        K8sQueryParam param = new K8sQueryParam();
        param.setKind(K8sKind.NAMESPACE);
        int pageNo = 0;
        List<ProtectedResource> resourceList = new ArrayList<>();
        PageListResponse<ProtectedResource> response;
        do {
            response = commonService.queryResource(pageNo, MAX_PAGE_SIZE, param, environment);
            resourceList.addAll(response.getRecords());
            pageNo++;
        } while (response.getRecords().size() >= MAX_PAGE_SIZE);

        List<ProtectedResource> validNamespace = resourceList.stream()
                .filter(this::isValidNamespace)
                .collect(Collectors.toList());

        for (ProtectedResource resource : validNamespace) {
            String envIdentity = environment.getUuid() + resource.getName();
            resource.setUuid(UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString());
            resource.setParentUuid(environment.getUuid());
            resource.setParentName(environment.getName());
            resource.setPath(environment.getPath() + File.separator + resource.getName());
            resource.setType(ResourceTypeEnum.KUBERNETES_COMMON.getType());
            resource.setSubType(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType());
        }
        deleteDatasetWhenNamespaceNotExist(validNamespace, environment);
        return validNamespace;
    }

    private void deleteDatasetWhenNamespaceNotExist(List<ProtectedResource> validNamespace, ProtectedEnvironment env) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("subType", ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.getType());
        conditions.put("rootUuid", env.getUuid());
        PageListResponse<ProtectedResource> oldDataset = resourceService.basicQuery(false, 0,
                    MAX_PAGE_SIZE, conditions);
        Set<String> uuids = validNamespace.stream().map(ProtectedResource::getUuid).collect(Collectors.toSet());

        String[] deleteDataset = oldDataset.getRecords().stream()
                .filter(dataset -> !uuids.contains(dataset.getParentUuid()))
                .map(ProtectedResource::getUuid)
                .distinct()
                .toArray(String[]::new);

        resourceService.delete(new ResourceDeleteParams(true, true, deleteDataset));
        log.info("Delete dataset when namespace not exist , delete dataset {}", String.join(",", deleteDataset));
    }

    private boolean isValidNamespace(ProtectedResource namespace) {
        return !K8S_SYSTEM_NS.contains(namespace.getName());
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
            BrowseEnvironmentResourceConditions environmentConditions) {
        K8sQueryParam param = JsonUtil.read(environmentConditions.getConditions(), K8sQueryParam.class);

        return commonService.queryResource(environmentConditions.getPageNo(), environmentConditions.getPageSize(),
                param, environment);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("K8s cluster check start, name: {}, uuid: {}.", environment.getName(), environment.getUuid());
        checkAuthParam(environment);
        checkJobNumOnSingleNode(environment);
        checkK8sClusterCount(environment);
        checkImageNameAndTag(environment);
        checkTaskOutTime(environment);
        checkScriptExecOutTime(environment);
        fillUserId(environment);
        parseAndFillIpFromConfig(environment);
        checkIpDontChange(environment);
        checkK8sClusterRepeat(environment);
        checkSsl(environment);
        checkClusterType(environment);
        commonService.checkConnectivity(environment);

        // 填充集群信息
        fillK8sClusterInfo(environment);
        deleteNodeSelecter(environment);
        checkVersion(environment);
        sendScanMessage(environment);
        log.info("K8s cluster check success, name: {}, uuid: {}.", environment.getName(), environment.getUuid());
    }

    private void checkClusterType(ProtectedEnvironment environment) {
        String clusterType = environment.getExtendInfo().get(K8sExtendInfoKey.CLUSTER_TYPE);
        if (VerifyUtil.isEmpty(clusterType)) {
            return;
        }
        if (!K8sExtendInfoKey.SUPPORT_CLUSTER_TYPE.contains(clusterType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s clusterType is illegal.");
        }
    }

    private void checkTaskOutTime(ProtectedEnvironment environment) {
        String taskTimeout = environment.getExtendInfo().get(K8sExtendInfoKey.TASK_TIMEOUT);
        TaskTimeOut timeout = JSONObject.toBean(taskTimeout, TaskTimeOut.class);
        if (VerifyUtil.isEmpty(timeout)) {
            return;
        }
        checkDay(timeout.getDays());
        checkHour(timeout.getHours());
        checkMinute(timeout.getMinutes());
        checkSeconds(timeout.getSeconds());
    }

    private void checkScriptExecOutTime(ProtectedEnvironment environment) {
        String consistentTimeout = environment.getExtendInfo().get(K8sExtendInfoKey.CONSISTENT_SCRIPT_EXEC_TIMEOUT);
        TaskTimeOut timeout = JSONObject.toBean(consistentTimeout, TaskTimeOut.class);
        if (VerifyUtil.isEmpty(timeout)) {
            return;
        }
        checkHour(timeout.getHours());
        checkMinute(timeout.getMinutes());
        checkSeconds(timeout.getSeconds());
        if (!VerifyUtil.isEmpty(timeout.getDays())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s TIMEOUT is illegal");
        }
    }

    private void checkDay(String days) {
        checkNum(days);
    }

    private void checkHour(String hours) {
        checkNum(hours);
        if (Integer.parseInt(hours) > 23) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s TIMEOUT is illegal");
        }
    }
    private void checkMinute(String minutes) {
        checkNum(minutes);
        checkMinuteOrSeconds(minutes);
    }

    private void checkSeconds(String seconds) {
        checkNum(seconds);
        checkMinuteOrSeconds(seconds);
    }

    private void checkNum(String days) {
        if (!PATTERN.matcher(days).matches()) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s TIMEOUT is illegal");
        }
    }

    private void checkMinuteOrSeconds(String time) {
        if (Integer.parseInt(time) > 59) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s TIMEOUT is illegal");
        }
    }

    private void checkSsl(ProtectedEnvironment env) {
        if (!SSL_STATUS.contains(env.getExtendInfo().get(K8sExtendInfoKey.IS_VERIFY_SSL))) {
            log.error("K8s cluster IS_VERIFY_SSL is illegal,Env name is {},Env id is {}", env.getName(), env.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s IS_VERIFY_SSL is illegal");
        }
        if (SSL_DISABLED.equals(env.getExtendInfo().get(K8sExtendInfoKey.IS_VERIFY_SSL)) || env.getAuth()
                .getAuthType() == Authentication.OTHER) {
            log.info("k8s cluster dont need check ssl,Env name is {},Env id is {}", env.getName(), env.getUuid());
            return;
        }
        if (VerifyUtil.isEmpty(env.getAuth().getExtendInfo().get(K8sExtendInfoKey.CERTIFICATE_AUTHORITY_DATA))) {
            log.error("K8s ssl_authority_data is empty,Env name is {},Env id is {}", env.getName(), env.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s ssl_authority_data is empty");
        }
    }

    private void fillUserId(ProtectedEnvironment environment) {
        if (!OpServiceUtil.isHcsService()) {
            log.info("Env id is {},it is not for HCS. Dont need to add User Id", environment.getUuid());
            return;
        }
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        if (token != null && token.getUser() != null) {
            environment.setUserId(token.getUser().getId());
            log.info("Env uuid is {},this env set userid is {}", environment.getUuid(), token.getUser().getId());
        }
    }

    private void deleteNodeSelecter(ProtectedEnvironment env) {
        Map<String, String> extendInfo = env.getExtendInfo();
        String selector = extendInfo.get(K8sExtendInfoKey.NODE_SELECTOR);
        if (!VerifyUtil.isEmpty(selector)) {
            log.info("dont delete selector! Environment uuid is {}, selector is {}", env.getUuid(), selector);
            return;
        }
        if (!VerifyUtil.isEmpty(env.getUuid())) {
            log.info("begin delete selector Environment uuid is {}, selector is {}", env.getUuid(), selector);
            resourceExtendInfoService.deleteByKeys(env.getUuid(), new String[]{K8sExtendInfoKey.NODE_SELECTOR});
        }
        env.getExtendInfo().remove(K8sExtendInfoKey.NODE_SELECTOR);
    }

    private void checkIpDontChange(ProtectedEnvironment environment) {
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("K8s cluster register dont need to check ip, k8s name: {}", environment.getName());
            return;
        }
        ProtectedResource env = resourceService.getBasicResourceById(environment.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "K8s not found."));
        if (!env.getEndpoint().equals(environment.getEndpoint())) {
            log.error("k8s cluster ip cant change ,old ip is {}, new ip is {} ,k8s name: {}", env.getEndpoint(),
                    environment.getEndpoint(), environment.getName());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s cluster ip cant change");
        }
    }

    private void sendScanMessage(ProtectedEnvironment environment) {
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("K8s cluster register dont need to scan again, k8s name: {}", environment.getName());
            return;
        }
        String envUuid = environment.getUuid();
        JSONObject messageData = new JSONObject();
        messageData.set("uuid", envUuid);
        messageTemplate.send(ProtectedEnvironmentListener.SCANNING_ENVIRONMENT_V2, messageData);
    }

    private void fillK8sClusterInfo(ProtectedEnvironment environment) {
        environment.setPath(Optional.ofNullable(environment.getPath()).orElse(environment.getEndpoint()));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());

        AppEnvResponse clusterInfo = commonService.queryClusterInfo(environment);
        // 设置version
        environment.setVersion(Optional.ofNullable(clusterInfo.getExtendInfo()).orElse(Collections.emptyMap())
                .get(K8sConstant.CLUSTER_VERSION));
    }

    private void checkVersion(ProtectedEnvironment environment) {
        String version = environment.getVersion();
        if (VerifyUtil.isEmpty(version) || version.compareTo(K8sConstant.K8S_MIN_VERSION) < 0) {
            log.error("K8s cluster register version is illegal, name: {}, version: {}.", environment.getName(),
                    environment.getVersion());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "k8s cluster version is illegal");
        }
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        try {
            commonService.addIpRule(environment.getEndpoint(), environment.getPort());
            unifiedConnectionCheckProvider.checkConnection(environment);
        } finally {
            commonService.deleteIpRule(environment.getEndpoint(), environment.getPort());
        }
    }

    private void checkAuthParam(ProtectedEnvironment env) {
        if (env.getAuth().getAuthType() == Authentication.TOKEN) {
            if (VerifyUtil.isEmpty(env.getEndpoint()) || VerifyUtil.isEmpty(env.getPort())
                    || VerifyUtil.isEmpty(env.getAuth().getExtendInfo().get(K8sExtendInfoKey.TOKEN))) {
                log.error("K8s cluster register param is correct, name: {}, endpoint: {}, port: {}.", env.getName(),
                        env.getEndpoint(), env.getPort());
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Endpoint or port is empty");
            }
        } else if (env.getAuth().getAuthType() == Authentication.OTHER) {
            if (VerifyUtil.isEmpty(env.getAuth().getExtendInfo().get(K8sExtendInfoKey.CONFIG))) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Extend info not correct");
            }
        } else {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Invalid auth type");
        }
    }

    private void checkK8sClusterCount(ProtectedEnvironment env) {
        if (!VerifyUtil.isEmpty(env.getUuid())) {
            log.info("Update K8s cluster(uuid: {}, name: {}), no need check count.", env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", env.getType());
        filter.put("subType", env.getSubType());
        int existedCount = resourceService.query(0, 1, filter).getTotalCount();
        if (existedCount >= K8S_CLUSTER_MAX_COUNT) {
            log.info("K8s cluster({}) count({}) over limit.", env.getName(), existedCount);
            throw new LegoCheckedException(CommonErrorCode.ENV_COUNT_OVER_LIMIT,
                    new String[]{String.valueOf(K8S_CLUSTER_MAX_COUNT)}, "K8s cluster count over limit {0}");
        }
    }

    private String findConfigServer(KubeConfig kubeConfig) {
        Contexts selectContext = Optional.ofNullable(kubeConfig.getContexts())
                .orElse(Collections.emptyList())
                .stream()
                .filter(context -> context.getName().equals(kubeConfig.getCurrentContext()))
                .findFirst()
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config"));
        return Optional.ofNullable(kubeConfig.getClusters())
                .orElse(Collections.emptyList())
                .stream()
                .filter(cluster -> cluster.getName().equals(selectContext.getContext().getCluster()))
                .findFirst()
                .map(Clusters::getCluster)
                .map(Cluster::getServer)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config"));
    }

    private KubeConfig toKubeConfig(String config) {
        KubeConfig kubeConfig;
        try {
            kubeConfig = JsonUtil.read(config, KubeConfig.class);
        } catch (DataMoverCheckedException e) {
            try {
                kubeConfig = YamlUtil.read(config, KubeConfig.class);
            } catch (LegoCheckedException e1) {
                log.error("Parse port from config file failed when chang to Java Class.");
                throw new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config");
            }
        }
        return kubeConfig;
    }

    private void parseAndFillIpFromConfig(ProtectedEnvironment env) {
        if (env.getAuth().getAuthType() == Authentication.TOKEN) {
            log.info("Register with ip, no need parse ip from config file, env name: {}.", env.getName());
            return;
        }
        String base64Config = env.getAuth().getExtendInfo().get(K8sExtendInfoKey.CONFIG);
        String config = Base64Util.decryptBase64ToString(base64Config);
        String server = findConfigServer(toKubeConfig(config));
        Pattern pattern = Pattern.compile(RegexpConstants.IP_V4V6_ADDRESS);
        Matcher matcher = pattern.matcher(server);
        if (!matcher.find()) {
            log.error("Parse endpoint and port from config file failed, env name: {}.", env.getName());
            throw new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config");
        }
        String tempIp = matcher.group();
        int index = config.indexOf(tempIp) + tempIp.length();
        StringBuilder builder = new StringBuilder(tempIp);
        while (index < config.length() && K8S_SERVER_IP.contains(config.charAt(index))) {
            builder.append(config.charAt(index));
            index++;
        }
        String[] split = builder.toString().split(":");
        if (split.length != 2) {
            log.error("Parse port from config file failed, env name: {}.", env.getName());
            throw new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config");
        }
        String endpoint = split[0].trim();
        env.setEndpoint(endpoint);
        int port;
        try {
            port = Integer.parseInt(split[1].trim());
            env.setPort(port);
        } catch (NumberFormatException e) {
            log.error("Parse port from config file failed, env name: {}.", env.getName());
            throw new LegoCheckedException(CommonErrorCode.KUBE_CONFIG_ERROR, "Invalid kube config");
        }
        log.info("Parse endpoint and port from config file success, endpoint: {}, port: {}.", endpoint, port);
    }

    private void checkK8sClusterRepeat(ProtectedEnvironment env) {
        if (!VerifyUtil.isEmpty(env.getUuid())) {
            log.info("Update K8s cluster(uuid: {}, name: {}), no need check repeat.", env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> filter = new HashMap<>();
        filter.put("endpoint", env.getEndpoint());
        filter.put("type", env.getType());
        filter.put("subType", env.getSubType());
        PageListResponse<ProtectedResource> registeredEnv = resourceService.query(0, 1, filter);
        if (registeredEnv.getTotalCount() > 0) {
            log.error("The env with endpoint({}) has been registered.", env.getEndpoint());
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "Env has registered");
        }
    }

    private void checkJobNumOnSingleNode(ProtectedEnvironment env) {
        Map<String, String> extendInfo = env.getExtendInfo();
        String jobNumOnSingleNode = extendInfo.get(K8sExtendInfoKey.JOB_NUM_ON_SINGLE_NODE);
        if (VerifyUtil.isEmpty(jobNumOnSingleNode)) {
            return;
        }
        try {
            int num = Integer.parseInt(jobNumOnSingleNode);
            if (num < MIN_JOB_NUMBER || num > MAX_JOB_NUMBER) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s jobNumOnSingleNode is illegal");
            }
        } catch (NumberFormatException e) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s jobNumOnSingleNode is illegal");
        }
    }

    private void checkImageNameAndTag(ProtectedEnvironment env) {
        Map<String, String> extendInfo = env.getExtendInfo();
        String imageNameAndTag = extendInfo.get(K8sExtendInfoKey.IMAGE_NAME_AND_TAG);
        log.info("Start to check image name and tag, uuid: {}, name: {}, imageAndTag: {}.",
                env.getUuid(), env.getName(), imageNameAndTag);
        if (VerifyUtil.isEmpty(imageNameAndTag)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s imageNameAndTag is empty");
        }
        String[] imageNameAndTags = imageNameAndTag.split(":");
        if (imageNameAndTags.length < 2 || VerifyUtil.isEmpty(imageNameAndTags[0])) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s imageNameAndTag is illegal");
        }
        String tag = imageNameAndTags[imageNameAndTags.length - 1];
        if (!tag.matches(K8sConstant.TAG_PATTERN)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s tag is illegal");
        }
    }
}