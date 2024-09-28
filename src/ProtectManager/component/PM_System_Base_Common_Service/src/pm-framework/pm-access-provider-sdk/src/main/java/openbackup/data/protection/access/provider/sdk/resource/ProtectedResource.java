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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.Authentication;

import lombok.EqualsAndHashCode;
import lombok.Getter;

import org.hibernate.validator.constraints.Length;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.validation.constraints.Max;

/**
 * 受保护资源
 *
 */
@EqualsAndHashCode(callSuper = true)
public class ProtectedResource extends ResourceBase {
    /**
     * 受保护状态
     */
    @Max(Integer.MAX_VALUE)
    protected Integer protectionStatus;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;

    /**
     * 资源所属的用户
     */
    @Length(max = 255)
    private String userId;

    /**
     * 资源授权的用户名称
     */
    @Length(max = 255)
    private String authorizedUser;

    /**
     * agent对应的ip
     */
    @Length(max = 255)
    private String endpoint;

    /**
     * agent对应端口
     */
    @Max(Integer.MAX_VALUE)
    private Integer port;

    private boolean isInGroup;

    @Getter
    private String resourceGroupName;

    @Getter
    private String resourceGroupId;

    /**
     * 资源的认证信息
     */
    private Authentication auth;

    private ProtectedObject protectedObject;

    private ProtectedAgentExtend protectedAgentExtend;

    private ResourceDesesitization resourceDesesitization;

    private ProtectedEnvironment environment;

    /**
     * 标签
     */
    @Getter
    private List<Label> labelList;

    /**
     * 依赖
     */
    private Map<String, List<ProtectedResource>> dependencies;

    public void setLabelList(List<Label> labelList) {
        this.labelList = labelList;
    }

    public ResourceDesesitization getResourceDesesitization() {
        return resourceDesesitization;
    }

    public void setResourceDesesitization(ResourceDesesitization resourceDesesitization) {
        this.resourceDesesitization = resourceDesesitization;
    }

    public Map<String, List<ProtectedResource>> getDependencies() {
        return dependencies;
    }

    public void setDependencies(Map<String, List<ProtectedResource>> dependencies) {
        this.dependencies = dependencies;
    }

    public Integer getProtectionStatus() {
        return protectionStatus;
    }

    public void setProtectionStatus(Integer protectionStatus) {
        this.protectionStatus = protectionStatus;
    }

    public Map<String, String> getExtendInfo() {
        return extendInfo;
    }

    public void setExtendInfo(Map<String, String> extendInfo) {
        this.extendInfo = extendInfo;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getAuthorizedUser() {
        return authorizedUser;
    }

    public void setAuthorizedUser(String authorizedUser) {
        this.authorizedUser = authorizedUser;
    }

    public ProtectedObject getProtectedObject() {
        return protectedObject;
    }

    public void setProtectedObject(ProtectedObject protectedObject) {
        this.protectedObject = protectedObject;
    }

    public ProtectedAgentExtend getProtectedAgentExtend() {
        return protectedAgentExtend;
    }

    public void setProtectedAgentExtend(ProtectedAgentExtend protectedAgentExtend) {
        this.protectedAgentExtend = protectedAgentExtend;
    }

    public ProtectedEnvironment getEnvironment() {
        return environment;
    }

    public void setEnvironment(ProtectedEnvironment environment) {
        this.environment = environment;
    }

    /**
     * get extend info by key
     *
     * @param key key
     * @return value
     */
    public String getExtendInfoByKey(String key) {
        return extendInfo != null ? extendInfo.get(key) : null;
    }

    public Authentication getAuth() {
        return auth;
    }

    public void setAuth(Authentication auth) {
        this.auth = auth;
    }

    /**
     * set extend info by key
     *
     * @param key key
     * @param value value
     * @return this object
     */
    public ProtectedResource setExtendInfoByKey(String key, String value) {
        if (extendInfo == null) {
            extendInfo = new HashMap<>();
        }
        if (value != null) {
            extendInfo.put(key, value);
        } else {
            extendInfo.remove(key);
        }
        return this;
    }

    public String getEndpoint() {
        return endpoint;
    }

    public void setEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }

    public Integer getPort() {
        return port;
    }

    public void setPort(Integer port) {
        this.port = port;
    }

    public boolean isInGroup() {
        return isInGroup;
    }

    public void setInGroup(boolean hasInGroup) {
        isInGroup = hasInGroup;
    }

    public void setResourceGroupName(String name) {
        resourceGroupName = name;
    }

    public void setResourceGroupId(String id) {
        resourceGroupId = id;
    }
}
