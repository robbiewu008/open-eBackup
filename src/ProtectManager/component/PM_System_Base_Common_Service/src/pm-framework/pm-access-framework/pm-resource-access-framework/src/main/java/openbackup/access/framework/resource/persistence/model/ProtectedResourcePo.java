/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.model;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.Label;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedAgentExtend;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDesesitization;
import openbackup.system.base.common.utils.JSONObject;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;
import com.fasterxml.jackson.core.JsonProcessingException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;

import java.sql.Timestamp;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Protected Resource Po
 *
 * @author l00272247
 * @since 2021-10-15
 */
@TableName("RESOURCES")
@Slf4j
public class ProtectedResourcePo {
    /**
     * ENVIRONMENTS_DISCRIMINATOR
     */
    public static final String ENVIRONMENTS_DISCRIMINATOR = "environments";

    /**
     * RESOURCES_DISCRIMINATOR
     */
    public static final String RESOURCES_DISCRIMINATOR = "resources";

    /**
     * 资源UUID
     */
    @TableId
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型（主类）
     */
    private String type;

    /**
     * 资源子类
     */
    private String subType;

    /**
     * 资源路径
     */
    private String path;

    /**
     * 创建时间
     */
    private Timestamp createdTime;

    /**
     * 父资源名称
     */
    private String parentName;

    /**
     * 父资源uuid
     */
    private String parentUuid;

    /**
     * 受保护环境uuid
     */
    private String rootUuid;

    /**
     * 受保护状态
     */
    private Integer protectionStatus;

    /**
     * 资源的来源
     */
    private String sourceType;

    /**
     * 资源所属的用户
     */
    private String userId;

    /**
     * 资源版本
     */
    private String version;

    /**
     * 资源授权的用户名称
     */
    private String authorizedUser;

    private String discriminator;

    private String auth;

    /**
     * 资源的扩展属性列表
     */
    @TableField(exist = false)
    private List<ProtectedResourceExtendInfoPo> extendInfoList;

    /**
     * 新增扩展表信息
     */
    @TableField(exist = false)
    private ProtectedAgentExtendPo protectedAgentExtendPo;

    /**
     * 资源的标签列表
     */
    @TableField(exist = false)
    private List<Label> labelList;

    /**
     * 保护对象信息
     */
    @TableField(exist = false)
    private ProtectedObjectPo protectedObjectPo;

    /**
     * 资源脱敏信息
     */
    @TableField(exist = false)
    private ResourceDesesitizationPo desesitizationPo;

    /**
     * cast protected resource bo from protected resource
     *
     * @param resource protected resource
     * @return protected resource bo
     */
    public static ProtectedResourcePo fromProtectedResource(ProtectedResource resource) {
        ProtectedResourcePo target = new ProtectedResourcePo();
        BeanUtils.copyProperties(resource, target);

        Authentication authentication = resource.getAuth();
        if (authentication != null) {
            target.setAuth(JSONObject.stringify(authentication));
        }

        Map<String, String> extendInfo = Optional.ofNullable(resource.getExtendInfo()).orElse(Collections.emptyMap());
        List<ProtectedResourceExtendInfoPo> extendInfoPoList = extendInfo.entrySet()
            .stream()
            .map(ProtectedResourceExtendInfoPo::fromEntry)
            .collect(Collectors.toList());
        extendInfoPoList.forEach(extendInfoPo -> extendInfoPo.setResourceId(resource.getUuid()));
        target.setExtendInfoList(extendInfoPoList);
        target.setLabelList(resource.getLabelList());
        if (resource instanceof ProtectedEnvironment) {
            target.setDiscriminator(ENVIRONMENTS_DISCRIMINATOR);
        } else {
            target.setDiscriminator(RESOURCES_DISCRIMINATOR);
        }

        return target;
    }

    public List<Label> getLabelList() {
        return labelList;
    }

    public void setLabelList(List<Label> labelList) {
        this.labelList = labelList;
    }
    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getSubType() {
        return subType;
    }

    public void setSubType(String subType) {
        this.subType = subType;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public Timestamp getCreatedTime() {
        return createdTime;
    }

    public void setCreatedTime(Timestamp createdTime) {
        this.createdTime = createdTime;
    }

    public String getParentName() {
        return parentName;
    }

    public void setParentName(String parentName) {
        this.parentName = parentName;
    }

    public String getParentUuid() {
        return parentUuid;
    }

    public void setParentUuid(String parentUuid) {
        this.parentUuid = parentUuid;
    }

    public String getRootUuid() {
        return rootUuid;
    }

    public void setRootUuid(String rootUuid) {
        this.rootUuid = rootUuid;
    }

    public Integer getProtectionStatus() {
        return protectionStatus;
    }

    public void setProtectionStatus(Integer protectionStatus) {
        this.protectionStatus = protectionStatus;
    }

    public String getSourceType() {
        return sourceType;
    }

    public void setSourceType(String sourceType) {
        this.sourceType = sourceType;
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

    public String getDiscriminator() {
        return discriminator;
    }

    public void setDiscriminator(String discriminator) {
        this.discriminator = discriminator;
    }

    public String getAuth() {
        return auth;
    }

    public void setAuth(String auth) {
        this.auth = auth;
    }

    public List<ProtectedResourceExtendInfoPo> getExtendInfoList() {
        return extendInfoList;
    }

    public void setExtendInfoList(List<ProtectedResourceExtendInfoPo> extendInfoList) {
        this.extendInfoList = extendInfoList;
    }

    public ProtectedAgentExtendPo getProtectedAgentExtendPo() {
        return protectedAgentExtendPo;
    }

    public void setProtectedAgentExtendPo(ProtectedAgentExtendPo protectedAgentExtendPo) {
        this.protectedAgentExtendPo = protectedAgentExtendPo;
    }

    public ProtectedObjectPo getProtectedObjectPo() {
        return protectedObjectPo;
    }

    public void setProtectedObjectPo(ProtectedObjectPo protectedObjectPo) {
        this.protectedObjectPo = protectedObjectPo;
    }

    public ResourceDesesitizationPo getDesesitizationPo() {
        return desesitizationPo;
    }

    public void setDesesitizationPo(ResourceDesesitizationPo desesitizationPo) {
        this.desesitizationPo = desesitizationPo;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    /**
     * cast protected resource bo to protected resource
     *
     * @return protected resource
     */
    public ProtectedResource toProtectedResource() {
        ProtectedResource target = create();
        BeanUtils.copyProperties(this, target);
        target.setCreatedTime(createdTime.toString());

        Map<String, String> properties = new HashMap<>();
        Optional.ofNullable(extendInfoList)
            .orElse(Collections.emptyList())
            .forEach(extendInfo -> properties.put(extendInfo.getKey(), extendInfo.getValue()));
        target.setExtendInfo(properties);
        target.setLabelList(labelList);
        if (StringUtils.equals("", this.getUserId())) {
            target.setUserId(null);
        }
        if (protectedObjectPo != null) {
            ProtectedObject protectedObject = new ProtectedObject();
            BeanUtils.copyProperties(protectedObjectPo, protectedObject);
            protectedObject.setExtParameters(castExtParametersToMap(protectedObjectPo.getExtParameters()));
            protectedObject.setSlaCompliance(protectedObjectPo.getIsSlaCompliance());
            target.setProtectedObject(protectedObject);
        }
        if (protectedAgentExtendPo != null) {
            ProtectedAgentExtend protectedAgentExtend = new ProtectedAgentExtend();
            BeanUtils.copyProperties(protectedAgentExtendPo, protectedAgentExtend);
            target.setProtectedAgentExtend(protectedAgentExtend);
        }
        if (desesitizationPo != null) {
            ResourceDesesitization resourceDesesitization = new ResourceDesesitization();
            BeanUtils.copyProperties(desesitizationPo, resourceDesesitization);
            target.setResourceDesesitization(resourceDesesitization);
        }
        if (auth != null) {
            Authentication authentication = JSONObject.toBean(auth, Authentication.class, JSONObject.RAW_OBJ_MAPPER);
            target.setAuth(authentication);
        }
        return target;
    }

    private static Map<String, Object> castExtParametersToMap(String extParameters) {
        Object result = extParameters;
        while (result instanceof String) {
            try {
                result = JSONObject.RAW_OBJ_MAPPER.readValue(result.toString(), Object.class);
            } catch (JsonProcessingException e) {
                log.error("read value failed", e);
                return Collections.emptyMap();
            }
        }
        if (result instanceof Map) {
            return (Map<String, Object>) result;
        }
        return Collections.emptyMap();
    }

    /**
     * create a ProtectedResource
     *
     * @return ProtectedResource
     */
    protected ProtectedResource create() {
        return new ProtectedResource();
    }
}
