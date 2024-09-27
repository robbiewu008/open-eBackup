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
package openbackup.access.framework.resource.controller.internal;

import openbackup.access.framework.resource.dto.AgentRegisterResponse;
import openbackup.access.framework.resource.dto.ProtectedEnvironmentDto;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.access.framework.resource.vo.DmcHostTrustData;
import openbackup.access.framework.resource.vo.DmcHostTrustError;
import openbackup.access.framework.resource.vo.DmcHostTrustVo;
import openbackup.access.framework.resource.vo.HostTrustVo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceExtendInfoKeyConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.auth.api.HcsTokenAPi;
import openbackup.system.base.sdk.auth.model.Condition;
import openbackup.system.base.sdk.auth.model.Constraint;
import openbackup.system.base.sdk.auth.model.IpQueryRequest;
import openbackup.system.base.sdk.auth.model.QueryIpResponse;
import openbackup.system.base.sdk.auth.model.RoleBo;
import openbackup.system.base.sdk.auth.model.Simple;
import openbackup.system.base.sdk.resource.enums.EnvironmentsOsTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.LinuxOsTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.alibaba.fastjson.JSON;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.time.DateUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.util.UriComponentsBuilder;

import java.net.URI;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import javax.validation.Valid;

/**
 * 资源注册内部接口
 *
 * @author w00616953
 * @since 2021-12-13
 */
@Slf4j
@RestController
@RequestMapping("/v2/internal")
public class EnvironmentInternalController {
    // 延迟时间，单位min
    private static final int DELAY_MINUTE = 5;

    // 更新
    private static final String UPDATE = "2";

    // 环境uuid格式常量
    private static final Pattern UUID_PATTERN = Pattern.compile("\\w{8}(-\\w{4}){3}-\\w{12}");

    // SanClient代理uuid格式正则校验
    private static final Pattern SAN_CLIENT_UUID_PATTERN = Pattern.compile("\\w{8}(-\\w{4}){3}-\\w{12}_sanclient");

    private static final String QUERY_IP_RESPONSE_URL = "/rest/csm-cmdb/v1/instances/CLOUD_VM_NOVA";

    private static final String SYS_ADMIN_ROLE_ID = "1";

    private final ProtectedEnvironmentService environmentService;

    private final ResourceService resourceService;

    private final UserService userService;

    @Autowired
    private HcsTokenAPi hcsTokenApi;

    @Autowired
    @Qualifier("hcsUserRestTemplate")
    private RestTemplate restTemplate;

    @Autowired
    private ProtectedResourceRepository resourceRepository;

    public EnvironmentInternalController(ProtectedEnvironmentService environmentService,
        ResourceService resourceService, UserService userService) {
        this.environmentService = environmentService;
        this.resourceService = resourceService;
        this.userService = userService;
    }

    /**
     * agent注册，注册受保护环境，分两种情况：
     * 1. 如果是新注册的环境，则直接注册
     * 2. 如果数据库存在有相同uuid的环境信息，则更新环境
     * (vm oracol 走python调用)
     *
     * @param environmentDto 受保护环境
     * @return 注册成功响应
     */
    @ExterAttack
    @PostMapping("/environments")
    public AgentRegisterResponse registerProtectedEnvironment(@RequestBody @Valid ProtectedEnvironmentDto environmentDto) {
        log.info("Register environment,uuid:{},name:{},endpoint:{},port:{},os type:{},sub type:{},createdTime:{}.",
            environmentDto.getUuid(), environmentDto.getName(), environmentDto.getEndpoint(), environmentDto.getPort(),
            environmentDto.getOsType(), environmentDto.getSubType(), environmentDto.getCreatedTime());
        checkEnvironmentUuid(environmentDto);
        ProtectedEnvironment environment = convertToProtectedEnvironment(environmentDto);

        setEnvironmentForHcs(environment);
        environmentService.updateInternalAgentAfterSystemDataRecovery(environment);
        resourceRepository.updateAgentShared(environment.getUuid(), environmentDto.getIsShared());
        // 1，agent升级， 2，agent卸载了未删除界面数据
        if (environmentService.hasSameEnvironmentInDb(environment)) {
            log.info("Update success, uuid: {}, endpoint: {}, port: {}", environment.getUuid(),
                environment.getEndpoint(), environment.getPort());
            return new AgentRegisterResponse(environment.getUuid());
        }
        environmentService.checkHasSameEndpointEnvironment(environment);
        environmentService.register(environment);
        log.info("Register success, uuid: {}, endpoint: {}, port: {}", environment.getUuid(), environment.getEndpoint(),
            environment.getPort());
        return new AgentRegisterResponse(environment.getUuid());
    }

    private void checkEnvironmentUuid(ProtectedEnvironmentDto environmentDto) {
        String uuid = environmentDto.getUuid();
        Pattern uuidPattern = ResourceSubTypeEnum.S_BACKUP_AGENT.getType().equals(environmentDto.getSubType())
            ? SAN_CLIENT_UUID_PATTERN
            : UUID_PATTERN;
        if (StringUtils.isBlank(uuid) || !uuidPattern.matcher(uuid).matches()) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Agent uuid is invalid.");
        }
    }

    private void setEnvironmentForHcs(ProtectedEnvironment environment) {
        String regionId = System.getenv("REGION_ID");
        // OP服务化场景才有regionId
        if (StringUtils.isEmpty(regionId)) {
            log.info("this is not for HCS. ");
            return;
        }

        Map<String, String> extendInfo = environment.getExtendInfo();
        if (extendInfo.containsKey("pushRegister")) {
            // 推送安装不查询
            log.info("this is not for pushRegister.");
            return;
        }
        if (!extendInfo.containsKey("nativeId") || !extendInfo.containsKey("projectId")) {
            log.info("not need to query hcs ip!");
            return;
        }

        log.info("start to set agent environment for hcs");
        // 1、从拓展信息中取出两个iD
        String nativeId = extendInfo.get("nativeId");
        String projectId = extendInfo.get("projectId");
        log.info("Set agent environment for hcs. The nativeId is {}, the projectId is {}", nativeId, projectId);

        // 2、调cmdb接口查询ip
        IpQueryRequest ipQueryRequest = new IpQueryRequest();
        Simple simple = new Simple();
        simple.setName("nativeId");
        simple.setValue(nativeId);
        Constraint constraint = new Constraint();
        constraint.setSimple(simple);
        Condition condition = new Condition();
        condition.setConstraint(Collections.singletonList(constraint));
        ipQueryRequest.setCondition(JSON.toJSONString(condition));
        String globalDomainName = System.getenv("GLOBAL_DOMAIN_NAME");
        String ocUrl = StringUtils.join("https://oc.", regionId, ".", globalDomainName, ":26335",
            QUERY_IP_RESPONSE_URL);
        log.info("Set agent environment for hcs. oc is {}, id:{}", ocUrl, environment.getUuid());
        String hcsToken = hcsTokenApi.getTokenByProjectId(null).getTokenStr();
        log.info("Set agent environment for hcs. start get hcs token, id:{}", environment.getUuid());
        QueryIpResponse queryIpResponse;
        try {
            queryIpResponse = getIpResponse(ocUrl, hcsToken, ipQueryRequest);
        } finally {
            StringUtil.clean(hcsToken);
        }
        String ip = queryIpResponse.getObjList().get(0).getExternalRelayIpAddress();
        log.info("Set agent environment for hcs. The ip is {}, id:{}", ip, environment.getUuid());

        // 3、塞入ip
        environment.setEndpoint(ip);
        log.info("Set agent environment for hcs. endpoint as:{}", ip);
    }

    private QueryIpResponse getIpResponse(String url, String token, IpQueryRequest ipQueryRequest) {
        HttpHeaders header = new HttpHeaders();
        header.add("X-Auth-Token", token);
        MultiValueMap<String, String> map = new LinkedMultiValueMap<>();
        map.add("condition", ipQueryRequest.getCondition());
        UriComponentsBuilder builder = UriComponentsBuilder.fromHttpUrl(url);
        URI uri = builder.queryParams(map).build().encode().toUri();
        return restTemplate.exchange(uri, HttpMethod.GET, new HttpEntity<>(map, header), QueryIpResponse.class)
            .getBody();
    }

    private ProtectedEnvironment convertToProtectedEnvironment(ProtectedEnvironmentDto environmentDto) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        BeanUtils.copyProperties(environmentDto, environment);
        buildEnvironmentUserId(environment, environmentDto);
        environment.setRootUuid(environmentDto.getUuid());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        if (VerifyUtil.isEmpty(environment.getPath())) {
            environment.setPath(environment.getEndpoint());
        }
        String osType = environment.getOsType();
        setOsTypeAndName(environment, osType);
        // agent(内置)注册后延迟5min再去开始执行扫描,避免nginx还未启动导致调接口失败
        Map<String, String> extendInfo = environment.getExtendInfo();
        String scenario = extendInfo.get(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO);
        if (scenario != null && scenario.equals(AgentTypeEnum.INTERNAL_AGENT.getValue())) {
            log.debug("internal agent: {} no need to check health.", environment.getUuid());
            setDelayTimeToScan(environment);
        }
        return environment;
    }

    private void setOsTypeAndName(ProtectedEnvironment environment, String osType) {
        if (StringUtils.isBlank(osType)) {
            return;
        }
        if (LinuxOsTypeEnum.isLinuxType(osType)) {
            environment.setOsType(EnvironmentsOsTypeEnum.LINUX.getOsType());
            environment.setOsName(EnvironmentsOsTypeEnum.LINUX.getOsType());
        }
        if (EnvironmentsOsTypeEnum.AIX.getOsType().equals(osType.toLowerCase(Locale.ENGLISH))) {
            environment.setOsType(EnvironmentsOsTypeEnum.AIX.getOsType());
            environment.setOsName(EnvironmentsOsTypeEnum.AIX.getOsType());
        }
        if (EnvironmentsOsTypeEnum.SOLARIS.getOsType().equals(osType.toLowerCase(Locale.ENGLISH))) {
            environment.setOsType(EnvironmentsOsTypeEnum.SOLARIS.getOsType());
            environment.setOsName(EnvironmentsOsTypeEnum.SOLARIS.getOsType());
        }
    }

    private void setDelayTimeToScan(ProtectedEnvironment environment) {
        Date now = new Date();
        Date afterDate = DateUtils.addMinutes(now, DELAY_MINUTE);
        String afterDateStr = new SimpleDateFormat(DateFormatConstant.DATE_TIME_WITH_MILLIS).format(afterDate);

        log.info("Set the Delayed Scan for environment, envId:{}, scan trigger time:{}", environment.getUuid(),
            afterDateStr);
        environment.setStartDate(afterDateStr);
    }

    private void buildEnvironmentUserId(ProtectedEnvironment environment, ProtectedEnvironmentDto environmentDto) {
        String userId = environmentDto.getUserId();
        if (StringUtils.isBlank(userId)) {
            return;
        }

        if (UPDATE.equals(environmentDto.getRegisterType())) {
            // 更新场景不修改userId
            log.info("Update agent is not need userId, agent id is {}", environmentDto.getUuid());
            environmentDto.setUserId(null);
            return;
        }

        UserInnerResponse userInnerResponse = userService.getUserInfoByUserId(userId);
        List<String> roleIds = userInnerResponse.getRolesSet()
            .stream()
            .map(RoleBo::getRoleId)
            .collect(Collectors.toList());
        if (roleIds.contains(SYS_ADMIN_ROLE_ID)) {
            environment.setUserId(null);
        } else {
            environment.setUserId(environmentDto.getUserId());
            environment.setAuthorizedUser(userInnerResponse.getUserName());
            log.info("set environment userId:{}, userName:{} success!", environment.getUserId(),
                environment.getAuthorizedUser());
        }
        // 设置注册用户id
        environment.getExtendInfo().put(ResourceExtendInfoKeyConstants.REGISTER_USER_ID, environmentDto.getUserId());
    }

    /**
     * 删除受保护环境，只是更新状态为离线
     *
     * @param envId 受保护环境ID
     */
    @ExterAttack
    @DeleteMapping("/environments/{envId}")
    public void deleteProtectedEnvironment(@PathVariable("envId") String envId) {
        ProtectedEnvironment env = environmentService.getEnvironmentById(envId);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(env.getSubType());
        environment.setType(env.getType());
        environment.setUuid(envId);
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
        if (ResourceSubTypeEnum.isSelectAgent(environment.getSubType())) {
            environment.setExtendInfoByKey(Constants.MANUAL_UNINSTALLATION, "1");
        }
        log.info("Update environment status to offline, uuid: {}", envId);
        environmentService.updateEnvironment(environment);
    }

    /**
     * 删除受保护环境
     *
     * @param envId 受保护环境ID
     */
    @ExterAttack
    @DeleteMapping("/environments/delete/{envId}")
    public void deleteProtectedEnvironmentForInternal(@PathVariable("envId") String envId) {
        // 删除数据库记录
        log.info("Delete environment internal, uuid: {}", envId);
        environmentService.deleteEnvironmentById(envId);
    }

    /**
     * 检查该主机是否受信
     *
     * @param endpoint endpoint
     * @return boolean
     */
    @ExterAttack
    @GetMapping("/environments/host/trust")
    public HostTrustVo checkHostBeTrust(@RequestParam("endpoint") String endpoint) {
        if (VerifyUtil.isEmpty(endpoint)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "endpoint is null or empty");
        }
        HostTrustVo hostTrustVo = new HostTrustVo();
        hostTrustVo.setIsTrusted(resourceService.checkHostIfBeTrustedByEndpoint(endpoint, true));
        return hostTrustVo;
    }

    /**
     * 检查dmc该主机是否受信
     *
     * @param endpoint endpoint
     * @return boolean
     */
    @ExterAttack
    @GetMapping("/environments/host/dmc/trust")
    public DmcHostTrustVo checkDmcHostBeTrust(@RequestParam("endpoint") String endpoint) {
        if (VerifyUtil.isEmpty(endpoint)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "endpoint is null or empty");
        }
        DmcHostTrustData dmcHostTrustData = new DmcHostTrustData();
        dmcHostTrustData.setIsTrusted(resourceService.checkHostIfBeTrustedByEndpoint(endpoint, true));
        return new DmcHostTrustVo(dmcHostTrustData, new DmcHostTrustError(0, "success"));
    }
}
