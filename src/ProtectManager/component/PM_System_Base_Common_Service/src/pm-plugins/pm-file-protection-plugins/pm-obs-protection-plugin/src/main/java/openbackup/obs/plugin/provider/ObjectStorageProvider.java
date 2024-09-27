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
package openbackup.obs.plugin.provider;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigConstants;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.obs.plugin.common.ObjectStorageCommonTool;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.obs.plugin.service.ObjectStorageAgentService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.query.PageQueryOperator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.ObjectStorageTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.validator.routines.UrlValidator;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Pattern;

/**
 * HCS-OBS存储
 *
 * @author w00607005
 * @since 2023-11-15
 */
@Slf4j
@Component
public class ObjectStorageProvider implements EnvironmentProvider {
    /**
     * 存在一个特殊字符正则
     */
    private static final Pattern SPECIAL_CHARACTERS_PATTERN =
        Pattern.compile("[!@#$%^&*()\\-=_+\\[\\]{};':\"\\\\|,.<>/?]");

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private ObjectStorageAgentService agentService;

    /**
     * 监测对象适用，该provider适用于对象存储
     *
     * @param resourceSubType 设备类型
     * @return detect 检查结果
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return StringUtils.equals(ResourceSubTypeEnum.OBJECT_STORAGE.getType(), resourceSubType);
    }

    /**
     * 环境检查
     *
     * @param environment 受保护环境
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("ObjectStorage check.");
        // 名称不能为空，名称只能输入字母，数字和下划线，长度不能超过32
        checkName(environment);
        if (environment.getAuth() == null || environment.getAuth().getExtendInfo() == null) {
            log.error("auth is null or extendInfo from auth is null.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "auth illegal.");
        }
        // Endpoint，最多128个字符（已有注解校验），符合正则表达式，一般为ip地址、域名
        checkEndpoint(environment);
        if (environment.getExtendInfo() == null) {
            log.error("extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "extendInfo illegal.");
        }
        // AK、SK不可为空，AK长度为1～256，SK长度为1～256
        checkAkAndSk(environment);
        // 代理服务器URL（含端口）、最多2048个字符，符合URL正则表达式，一般为http，https协议，用户、密码长度1~1024
        checkProxy(environment);
        // 证书文件不大于1M
        checkCert(environment);
        // 校验类型
        checkStorageType(environment);

        // 区分添加和更新场景
        if (StringUtils.isEmpty(environment.getUuid())) {
            // Endpoint不可达，则返回“对象存储网络无法连通”；
            // 认证信息不正确，则返回“对象存储认证失败”；
            // 证书校验失败，则返回“证书校验失败”；
            // 填写的“AK”和“SK”需要具有管理员权限以及 list 权限（用于获取 bucket 列表）
            String tenantId = checkConnection(environment);
            // 整系统最多支持添加2000个对象存储
            checkEnvironmentNum();
            // 校验通过后对隐私数据进行加密，更新场景需要校验不可修改的字段
            encryptData(environment);
            // 拼接uuid
            String uuid = environment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE)
                + SymbolConstant.UNDERLINE + tenantId;
            // 设置uuid
            environment.setUuid(uuid);
            // 校验uuid是否重复，避免重复注册
            checkEnvExist(environment);
        } else {
            Optional<ProtectedResource> equipment = resourceService.getResourceById(environment.getUuid());
            // 数据库中是否存在相同设备
            if (!equipment.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "env not exist.");
            }
            // 校验不可以修改项
            updateCheck(environment, equipment.get());
            // 检查连通性
            updateCheckConnection(environment, equipment.get());
            // 加密敏感数据
            encryptData(environment);
        }
        // auth中需要展示的非敏感信息存入资源扩展信息
        setResourceExtendInfo(environment);
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private void setResourceExtendInfo(ProtectedEnvironment environment) {
        String ak = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_AK);
        environment.getExtendInfo().put(EnvironmentConstant.KEY_AK, ak);
        environment.getExtendInfo()
            .put(EnvironmentConstant.KEY_USE_HTTPS,
                environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_USE_HTTPS));
        environment.getExtendInfo()
            .put(EnvironmentConstant.KEY_PROXY_ENABLE,
                environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_ENABLE));
        environment.getExtendInfo()
            .put(EnvironmentConstant.KEY_PROXY_HOST_NAME,
                environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_HOST_NAME));
        environment.getExtendInfo()
            .put(EnvironmentConstant.KEY_PROXY_USER_NAME,
                environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_NAME));
    }

    private void checkEnvExist(ProtectedEnvironment environment) {
        Optional<ProtectedResource> equipment = resourceService.getResourceById(environment.getUuid());
        if (equipment.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "env is exist.");
        }
    }

    private static void checkStorageType(ProtectedEnvironment environment) {
        if (!ObjectStorageTypeEnum.isObjectStorageType(
            Integer.parseInt(environment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE)))) {
            log.error("storageType invalid, storageType: {}",
                environment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE));
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "storageType illegal.");
        }
    }

    private static boolean isEnableProxy(ProtectedEnvironment environment) {
        String proxyEnable = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_ENABLE);
        return StringUtils.equals(proxyEnable, EnvironmentConstant.PROXY_ENABLE_VALUE);
    }

    private static boolean isUpdateSk(ProtectedEnvironment environment) {
        if (StringUtils.isEmpty(environment.getUuid())) {
            return false;
        }
        return StringUtils.isEmpty(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
    }

    private static boolean isUpdateSk(ProtectedEnvironment environment, ProtectedResource originEnvironment) {
        if (StringUtils.isEmpty(environment.getUuid())) {
            return false;
        }
        return Objects.equals(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK),
            originEnvironment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
    }

    private static boolean isUpdateProxyUserPwd(ProtectedEnvironment environment) {
        if (StringUtils.isEmpty(environment.getUuid())) {
            return false;
        }
        return StringUtils.isEmpty(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
    }

    private boolean isUpdateProxyUserPwd(ProtectedEnvironment environment, ProtectedResource originEnvironment) {
        if (StringUtils.isEmpty(environment.getUuid())) {
            return false;
        }
        return Objects.equals(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD),
            originEnvironment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
    }

    private void updateCheckConnection(ProtectedEnvironment environment, ProtectedResource equipment) {
        String encryptSk =
            isUpdateSk(environment, equipment) ? equipment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK)
                : encryptorService.encrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));

        if (isEnableProxy(environment)) {
            String encryptedPwd = isUpdateProxyUserPwd(environment, equipment)
                ? equipment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD) : encryptorService
                    .encrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
            environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_PWD, encryptedPwd);
        }
        environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_SK, encryptSk);
        checkConnection(environment);
    }

    private void encryptData(ProtectedEnvironment environment) {
        // 加密sk
        String encryptedSk =
            encryptorService.encrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
        environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_SK, encryptedSk);

        // 开启代理需要加密代理密码
        if (isEnableProxy(environment)) {
            String encryptedPwd = encryptorService
                .encrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
            environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_PWD, encryptedPwd);
        }
    }

    private void checkEnvironmentNum() {
        int total =
            resourceService
                .query(0, EnvironmentConstant.MAX_OBJECT_STORAGE_NUM,
                    ImmutableMap
                        .of(PluginConfigConstants.SUB_TYPE,
                            Lists.newArrayList(Collections.singletonList(PageQueryOperator.EQ.getValue()),
                                ResourceSubTypeEnum.OBJECT_STORAGE.getType())))
                .getTotalCount();
        if (total >= EnvironmentConstant.MAX_OBJECT_STORAGE_NUM) {
            throw new DataProtectionAccessException(CommonErrorCode.NUMBER_OF_DEVICES_EXCEEDS_THRESHOLD,
                new String[] {String.valueOf(EnvironmentConstant.MAX_OBJECT_STORAGE_NUM)},
                "The number of device exceeds threshold!");
        }
    }

    private String checkConnection(ProtectedEnvironment environment) {
        // 任一连通即为连通
        ActionResult[] results = agentService.checkConnection(environment);
        log.info("object storage healthCheckWithResultStatus, results.length: {}", results.length);
        for (ActionResult result : results) {
            if (result.getCode() == ActionResult.SUCCESS_CODE) {
                return environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_AK);
            }
        }
        log.error("check connection failed, name: {}", environment.getName());
        throw new LegoCheckedException(Long.parseLong(results[0].getBodyErr()),
            StringUtils.isEmpty(results[0].getMessage()) ? "check connection failed." : results[0].getMessage());
    }

    private void updateCheck(ProtectedEnvironment environment, ProtectedResource equipment) {
        // 类型不可修改
        if (!StringUtils.equals(environment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE),
            equipment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE))) {
            throw new DataProtectionAccessException(CommonErrorCode.ILLEGAL_PARAM,
                new String[] {environment.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE)});
        }

        // endpoint不可修改
        if (!StringUtils.equals(environment.getEndpoint(), equipment.getEndpoint())) {
            throw new DataProtectionAccessException(CommonErrorCode.ILLEGAL_PARAM,
                new String[] {environment.getEndpoint()});
        }

        // 协议不可修改
        if (!StringUtils.equals(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_USE_HTTPS),
            equipment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_USE_HTTPS))) {
            throw new DataProtectionAccessException(CommonErrorCode.ILLEGAL_PARAM,
                new String[] {environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_USE_HTTPS)});
        }
    }

    private static void checkCert(ProtectedEnvironment environment) {
        String certStr = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_CERTIFICATION);
        if (StringUtils.isEmpty(certStr)) {
            return;
        }
        long certSize = certStr.getBytes(StandardCharsets.UTF_8).length;
        if (certSize > 1024 * 1024L) {
            log.error("crt size out of limit.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "certificate illegal.");
        }
    }

    private static void checkProxy(ProtectedEnvironment environment) {
        String proxyHostName = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_HOST_NAME);
        // 代理用户密码可以为空
        String proxyUserName = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_NAME);
        String proxyUserPwd = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD);
        if (isEnableProxy(environment)) {
            if (StringUtils.isEmpty(proxyHostName)) {
                log.error("proxyHostName is empty.");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "proxyHostName illegal.");
            }
            if (StringUtils.isNotEmpty(proxyUserName) && proxyUserName.length() > 1024) {
                log.error("proxyUserName length out of limit.");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "proxyUserName illegal.");
            }
            if (proxyUserName == null) {
                environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_NAME, "");
            }

            if (StringUtils.isNotEmpty(proxyUserPwd) && proxyUserPwd.length() > 1024) {
                log.error("proxyUserPwd length out of limit.");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "proxyUserPwd illegal.");
            }

            UrlValidator urlValidator = new UrlValidator(new String[] {"http", "https"});
            if (proxyHostName.length() > 2048 || !urlValidator.isValid(proxyHostName)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "proxyHostName illegal.");
            }
        } else {
            // 不启用代理，则数据置空
            environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_HOST_NAME, "");
            environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_NAME, "");
            environment.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_PWD, "");
        }
    }

    private static void checkAkAndSk(ProtectedEnvironment environment) {
        String ak = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_AK);
        String sk = environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK);
        if (StringUtils.isEmpty(ak) || ak.length() > 256 || SPECIAL_CHARACTERS_PATTERN.matcher(ak).matches()) {
            log.error("ak is invalid.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "ak illegal.");
        }

        // 新增场景SK不允许为空，更新场景允许传空值到后端
        if (!isUpdateSk(environment)) {
            if (StringUtils.isEmpty(sk) || sk.length() > 256 || SPECIAL_CHARACTERS_PATTERN.matcher(sk).matches()) {
                log.error("sk is invalid.");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "sk illegal.");
            }
        } else {
            if (sk.length() > 256 || SPECIAL_CHARACTERS_PATTERN.matcher(sk).matches()) {
                log.error("sk is invalid.");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "sk illegal.");
            }
        }
    }

    private static void checkEndpoint(ProtectedEnvironment environment) {
        String endpoint = environment.getEndpoint();
        ObjectStorageCommonTool.checkEndPoint(endpoint);
        environment.getAuth().getExtendInfo().put(EnvironmentConstant.ENDPOINT, endpoint);
    }

    private static void checkName(ProtectedEnvironment environment) {
        String name = environment.getName();
        if (StringUtils.isEmpty(name)) {
            log.error("env name is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "name illegal.");
        }
        if (!EnvironmentConstant.NAME_PATTERN.matcher(name).matches()) {
            log.error("env name not match pattern");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "name illegal.");
        }
    }

    /**
     * 健康检查
     *
     * @param environment 受保护环境
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        checkConnection(environment);
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        checkEnvLinkStatus(environment);

        ProtectedResource protectedResource = getProtectedResource(environment, environmentConditions.getPageNo(),
            environmentConditions.getPageSize(), ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        environment.getAuth()
            .getExtendInfo()
            .put(EnvironmentConstant.KEY_SK,
                encryptorService.decrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK)));
        if (isEnableProxy(environment)) {
            environment.getAuth()
                .getExtendInfo()
                .put(EnvironmentConstant.KEY_PROXY_USER_PWD, encryptorService
                    .decrypt(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD)));
        }

        try {
            return agentService.getDetail(environment, protectedResource,
                environment.getExtendInfoByKey(AgentKeyConstant.AGENTS_KEY), ResourceSubTypeEnum.OBJECT_STORAGE);
        } finally {
            StringUtil.clean(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
            StringUtil.clean(environment.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
        }
    }

    private void checkEnvLinkStatus(ProtectedEnvironment environment) {
        ProtectedEnvironment env = resourceService.getResourceByIdIgnoreOwner(environment.getUuid())
            .filter(e -> e instanceof ProtectedEnvironment)
            .map(e -> (ProtectedEnvironment) e)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Object storage " + environment.getUuid() + " not exist!"));
        if (!String.valueOf(LinkStatusEnum.ONLINE.getStatus())
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env))) {
            log.error("Failed to obtain object storage bucket, the object storage is offline, uuid:{}", env.getUuid());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "The object storage" + environment.getUuid() + " is offline.");
        }
    }

    private ProtectedResource getProtectedResource(ProtectedEnvironment environment, int pageNo, int pageSize,
        String resourceType) {
        ProtectedResource protectedResource = BeanTools.copy(environment, ProtectedResource::new);
        protectedResource.setType(resourceType);
        protectedResource.setSubType(resourceType);
        protectedResource.setExtendInfoByKey("pageNo", String.valueOf(pageNo));
        protectedResource.setExtendInfoByKey("pageSize", String.valueOf(pageSize));
        return protectedResource;
    }
}
